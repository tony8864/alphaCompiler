%{

#include "parser_util/parser_util.h"

#include <stdio.h>

int yyerror(char* errorMessage);
int yylex(void);

extern FILE* yyin;
extern char* yytext;
extern int yylineno;

%}

%union {
    int     intVal;
    char*   strVal;
    double  realVal;
    unsigned unsignedVal;
    SymbolTableEntry* symbol;
    Expr* exprVal;
}


%start program

%token IF ELSE WHILE FOR FUNCTION RETURN BREAK CONTINUE AND NOT OR LOCAL TRUE FALSE NIL
%token EQUAL PLUS MINUS MULTIPLY DIVIDE MODULO ASSIGN NOT_EQUAL PLUS_PLUS MINUS_MINUS GREATER GREATER_EQUAL LESS LESS_EQUAL
%token LEFT_CURLY_BRACKET RIGHT_CURLY_BRACKET LEFT_SQUARE_BRACKET RIGHT_SQUARE_BRACKET LEFT_PARENTHESIS RIGHT_PARENTHESIS COLON SEMICOLON COMMA DOUBLE_COLON DOT DOT_DOT
%token COMMENT

%token<intVal>  INTEGER 
%token<realVal> REAL
%token<strVal>  STRING 
%token<strVal>  IDENTIFIER

%type<strVal> funcname
%type<unsignedVal> funcbody
%type<symbol> funcprefix funcdef
%type<exprVal> lvalue member expr primary const

%right ASSIGN
%left OR
%left AND
%nonassoc NOT_EQUAL EQUAL
%nonassoc GREATER GREATER_EQUAL LESS LESS_EQUAL
%left PLUS MINUS
%left MULTIPLY DIVIDE MODULO
%right NOT PLUS_PLUS MINUS_MINUS UMINUS
%left DOT DOT_DOT
%left LEFT_SQUARE_BRACKET RIGHT_SQUARE_BRACKET

%%

program:
        stmts               { printf("program\n"); }
        | /* empty */       { printf("empty program\n"); }
        ;


stmts: 
        stmt                { printf("stmt\n"); }
        | stmts stmt        { printf("stmts stmt\n"); }
        ;

stmt:
        expr SEMICOLON              { printf("expr semicolon\n"); }
        | ifstmt                    { printf("if stmt\n"); }
        | whilestmt                 { printf("while stmt\n"); }
        | forstmt                   { printf("for stmt\n"); }
        | returnstmt                { printf("return stmt\n"); }
        | BREAK SEMICOLON           { printf("break semicolon\n"); }
        | CONTINUE SEMICOLON        { printf("continue semicolon\n"); }
        | block                     { printf("block\n"); }
        | funcdef                   { printf("func def\n"); }
        | SEMICOLON
        ;

expr:
        lvalue ASSIGN expr
                {
                        $$ = parserUtil_handleAssignExpr($1, $3, yylineno);
                }
        | expr PLUS expr            { printf("expr + expr\n"); }
        | expr MINUS expr           { printf("expr - expr\n"); }
        | expr MULTIPLY expr        { printf("expr * expr\n"); }
        | expr DIVIDE expr          { printf("expr / expr\n"); }
        | expr MODULO expr          { printf("expr mod expr\n"); }
        | expr GREATER expr         { printf("expr > expr\n"); }
        | expr GREATER_EQUAL expr   { printf("expr >= expr\n"); }
        | expr LESS expr            { printf("expr < expr\n"); }
        | expr LESS_EQUAL expr      { printf("expr <= expr\n"); }
        | expr EQUAL expr           { printf("expr == expr\n"); }
        | expr NOT_EQUAL expr       { printf("expr != expr\n"); }
        | expr AND expr             { printf("expr and expr\n"); }
        | expr OR expr              { printf("expr or expr\n"); }
        | LEFT_PARENTHESIS expr RIGHT_PARENTHESIS   { printf("(expr )\n"); }
        | MINUS expr %prec UMINUS   { printf("minus expr\n"); }
        | NOT expr                  { printf("not expr\n"); }
        | PLUS_PLUS lvalue          { printf("l++lval\n"); }
        | lvalue PLUS_PLUS          { printf("lval++\n"); }
        | MINUS_MINUS lvalue        { printf("--lval\n"); }
        | lvalue MINUS_MINUS        { printf("lval--\n"); }
        | primary                   { $$ = $1; }
        ;

primary:
        lvalue
                {       
                        $$ = parserUtil_handlePrimary($1, yylineno);
                }
        | call
        | objectdef
        | LEFT_PARENTHESIS funcdef RIGHT_PARENTHESIS
        | const
                {
                        $$ = $1;
                }
        ;

lvalue:
        IDENTIFIER
                {
                        $$ = parserUtil_handleIdentifier($1, yylineno);
                }
        | LOCAL IDENTIFIER 
                {
                        $$ = parserUtil_handleLocalIdentifier($2, yylineno);
                }
        | DOUBLE_COLON IDENTIFIER
                {
                        $$ = parserUtil_habdleGlobalLookup($2, yylineno);
                }
        | member
        ;

member:
        lvalue DOT IDENTIFIER
                {
                        $$ = parserUtil_handleLvalueIdentifierTableItem($1, $3, yylineno);
                }
        | lvalue LEFT_SQUARE_BRACKET expr RIGHT_SQUARE_BRACKET
                {
                        $$ = parserUtil_handleLvalueExprTableItem($1, $3, yylineno);
                }
        | call DOT IDENTIFIER
        | call LEFT_SQUARE_BRACKET expr RIGHT_SQUARE_BRACKET
        ;

call:
        call LEFT_PARENTHESIS elist RIGHT_PARENTHESIS
        | lvalue callsuffix
        | LEFT_PARENTHESIS funcdef RIGHT_PARENTHESIS LEFT_PARENTHESIS elist RIGHT_PARENTHESIS
        ;

callsuffix:
            normcall
            | methodcall
            ;

normcall:
            LEFT_PARENTHESIS elist RIGHT_PARENTHESIS
            ;

methodcall:
            DOT_DOT IDENTIFIER LEFT_PARENTHESIS elist RIGHT_PARENTHESIS
            ;

elist:
        expr
        | elist COMMA expr
        ;

objectdef:
            LEFT_SQUARE_BRACKET elist RIGHT_SQUARE_BRACKET
            | LEFT_SQUARE_BRACKET indexed RIGHT_SQUARE_BRACKET
            ;

indexed:
        indexedelem
        | indexed COMMA indexedelem
        ;

indexedelem:
            LEFT_CURLY_BRACKET expr COLON expr RIGHT_CURLY_BRACKET
            ;

block:
        LEFT_CURLY_BRACKET 
                {
                        parserUtil_handleBlockEntrance();
                } 
        stmts 
        RIGHT_CURLY_BRACKET 
                {
                        parserUtil_handleBlockExit();
                }
        | LEFT_CURLY_BRACKET
                {
                        parserUtil_handleBlockEntrance();
                }
        RIGHT_CURLY_BRACKET
                {
                        parserUtil_handleBlockExit();
                }
        ;

funcname: IDENTIFIER
                {
                        $$ = $1;
                }
        | /* empty */
                {
                        $$ = parserUtil_generateUnnamedFunctionName();
                }
        ;

funcprefix: FUNCTION funcname
                {
                        $$ = parserUtil_handleFuncPrefix($2, yylineno);
                }
        ;

funcargs: LEFT_PARENTHESIS idlist RIGHT_PARENTHESIS
                {
                        parserUtil_handleFuncArgs();
                }
        ;

funcbody: funcblock
                {
                        $$ = parserUtil_handleFuncbody();
                }
        ;

funcdef: funcprefix funcargs funcbody
                {
                        $$ = parserUtil_handleFuncdef($1, $3, yylineno);
                }
        ;

funcblock: 
        LEFT_CURLY_BRACKET
                {

                }
        stmts
        RIGHT_CURLY_BRACKET
                {

                }
        | LEFT_CURLY_BRACKET
                {

                }
        RIGHT_CURLY_BRACKET
                {

                }
        ;
           

const:
        INTEGER
                {
                        $$ = parserUtil_newConstnumExpr($1 * 1.0);
                }
        | REAL      { printf("real: %f\n", $1); }
        | STRING    { printf("string: %s\n", $1); }
        | NIL       { printf("nil\n"); }
        | TRUE      { printf("true\n"); }
        | FALSE     { printf("false\n"); }
        ;

idlist:
        IDENTIFIER
                {
                        parserUtil_handleFormalArgument(yylval.strVal, yylineno);
                }
        | idlist COMMA IDENTIFIER
                {
                        parserUtil_handleFormalArgument(yylval.strVal, yylineno);
                }
        | /* empty */
        ;

ifstmt:
        IF LEFT_PARENTHESIS expr RIGHT_PARENTHESIS stmt
        | IF LEFT_PARENTHESIS expr RIGHT_PARENTHESIS stmt ELSE stmt
        ;

whilestmt:
            WHILE LEFT_PARENTHESIS expr RIGHT_PARENTHESIS stmt
            ;

forstmt:
        FOR LEFT_PARENTHESIS elist SEMICOLON expr SEMICOLON elist RIGHT_PARENTHESIS stmt
        ;

returnstmt:
            RETURN expr SEMICOLON
            ;


%%

int main(int argc, char **argv) {
    if (argc > 1) {
        if (!(yyin = fopen(argv[1], "r"))) {
            printf("Cannot read file: %s\n", argv[1]);
            return 1;
        }
    }
    else {
        yyin = stdin;
    }

    parserUtil_initialize();
    yyparse();
    parserUtil_cleanup();
    return 0;
}

int yyerror(char* errorMessage) {

    fprintf(stderr, "Syntax error at line %d: %s\n", yylineno, errorMessage);
    fprintf(stderr, "  Offending token: '%s'\n", yytext);

    return 1;
}