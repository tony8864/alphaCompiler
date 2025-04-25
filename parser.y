%{

#include "parser_util/parser_util.h"

#include <stdio.h>
#include <stdlib.h>

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
    Call* callVal;
    Indexed* indexedVal;
    ForPrefix* forPrefixVal;
    Statement* statementVal;
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
%type<unsignedVal> funcbody ifprefix elseprefix whilestart whilecond N M
%type<symbol> funcprefix funcdef
%type<exprVal> lvalue member expr primary const elist call objectdef
%type<callVal> callsuffix normcall methodcall
%type<indexedVal> indexedelem indexed
%type<forPrefixVal> forprefix
%type<statementVal> stmts stmt block ifstmt loopstmt

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
        stmt                { $$ = $1; }
        | stmts stmt        { $$ = parserUtil_handleStatement($1, $2); }
        ;

stmt:
        expr SEMICOLON              { $$ = parserUtil_handleGeneralStatement(); }
        | ifstmt                    { $$ = $1; }
        | whilestmt                 { $$ = parserUtil_handleGeneralStatement(); }
        | forstmt                   { $$ = parserUtil_handleGeneralStatement(); }
        | returnstmt                { $$ = parserUtil_handleGeneralStatement(); }
        | BREAK SEMICOLON           { $$ = parserUtil_handleBreak(yylineno); }
        | CONTINUE SEMICOLON        { $$ = parserUtil_handleContinue(yylineno);}
        | block                     { $$ = $1; }
        | funcdef                   { $$ = parserUtil_handleGeneralStatement(); }
        | SEMICOLON                 { $$ = parserUtil_handleGeneralStatement(); }
        ;

expr:
        lvalue ASSIGN expr                              { $$ = parserUtil_handleAssignExpr($1, $3, yylineno); }
        | expr PLUS expr                                { $$ = parserUtil_handleArithmeticExpr($1, $3, add_op, yylineno); }
        | expr MINUS expr                               { $$ = parserUtil_handleArithmeticExpr($1, $3, sub_op, yylineno); }
        | expr MULTIPLY expr                            { $$ = parserUtil_handleArithmeticExpr($1, $3, mul_op, yylineno); }
        | expr DIVIDE expr                              { $$ = parserUtil_handleArithmeticExpr($1, $3, div_op, yylineno); }
        | expr MODULO expr                              { $$ = parserUtil_handleArithmeticExpr($1, $3, mod_op, yylineno); }
        | expr GREATER expr                             { $$ = parserUtil_handleRelationalExpr($1, $3, if_greater_op, yylineno); }
        | expr GREATER_EQUAL expr                       { $$ = parserUtil_handleRelationalExpr($1, $3, if_greatereq_op, yylineno); }
        | expr LESS expr                                { $$ = parserUtil_handleRelationalExpr($1, $3, if_less_op, yylineno); }
        | expr LESS_EQUAL expr                          { $$ = parserUtil_handleRelationalExpr($1, $3, if_lesseq_op, yylineno); }
        | expr EQUAL expr                               { $$ = parserUtil_handleRelationalExpr($1, $3, if_eq_op, yylineno);}
        | expr NOT_EQUAL expr                           { $$ = parserUtil_handleRelationalExpr($1, $3, if_noteq_op, yylineno); }
        | expr AND expr                                 { $$ = parserUtil_handleBooleanExpr($1, $3, and_op, yylineno); }
        | expr OR expr                                  { $$ = parserUtil_handleBooleanExpr($1, $3, or_op, yylineno); }
        | LEFT_PARENTHESIS expr RIGHT_PARENTHESIS       { $$ = $2; }
        | MINUS expr %prec UMINUS                       { $$ = parserUtil_handleUminusExpr($2, yylineno); }
        | NOT expr                                      { $$ = parserUtil_handleNotExpr($2, yylineno); }
        | PLUS_PLUS lvalue                              { $$ = parserUtil_handleIncrementLvalue($2, yylineno); }
        | lvalue PLUS_PLUS                              { $$ = parserUtil_handleLvalueIncrement($1, yylineno); }
        | MINUS_MINUS lvalue                            { $$ = parserUtil_handleDecrementLvalue($2, yylineno); }
        | lvalue MINUS_MINUS                            { $$ = parserUtil_handleLvalueDecrement($1, yylineno); }
        | primary                                       { $$ = $1; }
        ;

primary:
        lvalue                                          { $$ = parserUtil_handlePrimary($1, yylineno); }
        | call                                          { $$ = $1; }
        | objectdef                                     { $$ = $1; }
        | LEFT_PARENTHESIS funcdef RIGHT_PARENTHESIS    { $$ = parserUtil_handlePrimaryFuncdef($2); }
        | const                                         { $$ = $1; }
        ;

lvalue:
        IDENTIFIER                      { $$ = parserUtil_handleIdentifier($1, yylineno); }
        | LOCAL IDENTIFIER              { $$ = parserUtil_handleLocalIdentifier($2, yylineno); }
        | DOUBLE_COLON IDENTIFIER       { $$ = parserUtil_habdleGlobalLookup($2, yylineno); }
        | member                        { $$ = $1; }
        ;

member:
        lvalue DOT IDENTIFIER                                   { $$ = parserUtil_handleLvalueIdentifierTableItem($1, $3, yylineno); }
        | lvalue LEFT_SQUARE_BRACKET expr RIGHT_SQUARE_BRACKET  { $$ = parserUtil_handleLvalueExprTableItem($1, $3, yylineno); }
        | call DOT IDENTIFIER                                   { $$ = parserUtil_handleLvalueIdentifierTableItem($1, $3, yylineno); }
        | call LEFT_SQUARE_BRACKET expr RIGHT_SQUARE_BRACKET    { $$ = parserUtil_handleLvalueExprTableItem($1, $3, yylineno); }
        ;

call:
        call LEFT_PARENTHESIS elist RIGHT_PARENTHESIS                                           { $$ = parserUtil_handleCall($1, $3, yylineno); }
        | call LEFT_PARENTHESIS RIGHT_PARENTHESIS                                               { $$ = parserUtil_handleCall($1, NULL, yylineno); }
        | lvalue callsuffix                                                                     { $$ = parserUtil_handleCallSuffix($1, $2, yylineno); }
        | LEFT_PARENTHESIS funcdef RIGHT_PARENTHESIS LEFT_PARENTHESIS elist RIGHT_PARENTHESIS   { $$ = parserUtil_handleCallFuncdef($2, $5, yylineno); }
        ;

callsuffix:      normcall { $$ = $1; } | methodcall { $$ = $1; };
normcall:        LEFT_PARENTHESIS elist RIGHT_PARENTHESIS { $$ = parserUtil_handleNormCall($2); } | LEFT_PARENTHESIS RIGHT_PARENTHESIS  { $$ = parserUtil_handleNormCall(NULL); };
methodcall:      DOT_DOT IDENTIFIER LEFT_PARENTHESIS elist RIGHT_PARENTHESIS                                                            { $$ = parserUtil_handleMethodCall($2, $4); };

elist:
        expr                    { $$ = $1; }
        | elist COMMA expr      { $$ = parserUtil_handleElist($1, $3); }
        ;

objectdef:
            LEFT_SQUARE_BRACKET elist RIGHT_SQUARE_BRACKET      { $$ = parserUtil_handleMakeElistTable($2, yylineno); }
            | LEFT_SQUARE_BRACKET indexed RIGHT_SQUARE_BRACKET  { $$ = parserUtil_handleMakeIndexedTable($2, yylineno); }
            | LEFT_SQUARE_BRACKET RIGHT_SQUARE_BRACKET          { $$ = parserUtil_handleMakeIndexedTable(NULL, yylineno); }
            ;

indexed:        indexedelem                                             { $$ = $1; } | indexed COMMA indexedelem { $$ = parserUtil_handleIndexed($1, $3); };
indexedelem:    LEFT_CURLY_BRACKET expr COLON expr RIGHT_CURLY_BRACKET  { $$ = parserUtil_newIndexed($2, $4); };

block:
        LEFT_CURLY_BRACKET      { parserUtil_handleBlockEntrance(); } 
        stmts 
        RIGHT_CURLY_BRACKET     { parserUtil_handleBlockExit(); $$ = $3; }
        | LEFT_CURLY_BRACKET    { parserUtil_handleBlockEntrance(); }
        RIGHT_CURLY_BRACKET     { parserUtil_handleBlockExit(); $$ = parserUtil_newStatement(); }
        ;

funcblockstart: { parserUtil_handleFuncBlockStart(); };
funcblockend:   { parserUtil_handleFuncBlockEnd(); };

funcname:       IDENTIFIER                                      { $$ = $1; } | /* empty */   { $$ = parserUtil_generateUnnamedFunctionName(); } ;
funcprefix:     FUNCTION funcname                               { $$ = parserUtil_handleFuncPrefix($2, yylineno); };
funcargs:       LEFT_PARENTHESIS idlist RIGHT_PARENTHESIS       { parserUtil_handleFuncArgs(); };
funcbody:       funcblockstart funcblock funcblockend           { $$ = parserUtil_handleFuncbody(); };
funcdef:        funcprefix funcargs funcbody                    { $$ = parserUtil_handleFuncdef($1, $3, yylineno); };
funcblock:      LEFT_CURLY_BRACKET stmts RIGHT_CURLY_BRACKET | LEFT_CURLY_BRACKET RIGHT_CURLY_BRACKET ;
           
const:
          INTEGER         { $$ = parserUtil_newConstnumExpr($1 * 1.0); }
        | REAL            { $$ = parserUtil_newConstnumExpr($1); }
        | STRING          { $$ = parserUtil_newConstString($1); }
        | NIL             { $$ = parserUtil_newConstNil(); }
        | TRUE            { $$ = parserUtil_newBoolExpr(1); }
        | FALSE           { $$ = parserUtil_newBoolExpr(0); }
        ;

idlist:
          IDENTIFIER                      { parserUtil_handleFormalArgument(yylval.strVal, yylineno); }
        | idlist COMMA IDENTIFIER         { parserUtil_handleFormalArgument(yylval.strVal, yylineno); }
        | /* empty */
        ;

ifstmt:
          ifprefix stmt                           { parserUtil_handleIfPrefixStatement($1); $$ = $2; }
        | ifprefix stmt elseprefix stmt           { $$ = parserUtil_handleIfElsePrefixStatement($1, $2, $3, $4); }
        ;

ifprefix:       IF LEFT_PARENTHESIS expr RIGHT_PARENTHESIS      { $$ = parserUtil_handleIfPrefix($3, yylineno);};
elseprefix:     ELSE                                            { $$ = parserUtil_handleElse(yylineno); }

loopstart:                              { parserUtil_handleLoopStart(); };
loopend:                                { parserUtil_handleLoopEnd(); };
loopstmt:       loopstart stmt loopend  {  $$ = $2; }

whilestmt:      whilestart whilecond loopstmt           { parserUtil_handleWhileStatement($1, $2, $3, yylineno);};
whilestart:     WHILE                                   { $$ = parserUtil_handleWhileStart();};
whilecond:      LEFT_PARENTHESIS expr RIGHT_PARENTHESIS { $$ = parserUtil_handleWhileCond($2, yylineno);};


forstmt: forprefix N elist RIGHT_PARENTHESIS N stmt N                   { parserUtil_handleForStatement($1, $2, $5, $6, $7); }
forprefix: FOR LEFT_PARENTHESIS elist SEMICOLON M expr SEMICOLON        { $$ = parserUtil_handleForPrefix($5, $6, yylineno);}
N: { $$ = parserUtil_handleNrule(yylineno); }
M: { $$ = parserUtil_handleMrule(yylineno); }

returnstmt: RETURN expr SEMICOLON { parserUtil_handleReturn($2, yylineno); }
            | RETURN SEMICOLON { parserUtil_handleReturn(NULL, yylineno); }
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