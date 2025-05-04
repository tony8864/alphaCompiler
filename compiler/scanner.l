%{

#include "icode/icode.h"
#include "symbol_table/symbol_table.h"
#include "scanner_util/scanner_util.h"
#include "comment_reader/comment_reader.h"
#include "parser.tab.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int stringStartLine = 0;

%}

%option noyywrap
%option yylineno

id          [a-zA-Z][a-zA-Z_0-9]*
integer     [0-9]+
real        [0-9]*\.[0-9](e|E)?[0-9]*
space       [\t\n \t\v]
linecomment "//".*

%x STRING_COND
%x BLOCK_COMMENT

%%

"if"            { return IF; }
"else"          { return ELSE; }
"while"         { return WHILE; }
"for"           { return FOR; }
"function"      { return FUNCTION; }
"return"        { return RETURN; }
"break"         { return BREAK; }
"continue"      { return CONTINUE; }
"and"           { return AND; }
"not"           { return NOT; }
"or"            { return OR; }
"local"         { return LOCAL; }
"true"          { return TRUE; }
"false"         { return FALSE; }
"nil"           { return NIL; }

"=="            { return EQUAL; }
"+"             { return PLUS; }
"-"             { return MINUS; }
"*"             { return MULTIPLY; }
"/"             { return DIVIDE; }
"%"             { return MODULO; }
"="             { return ASSIGN; }
"!="            { return NOT_EQUAL; }
"++"            { return PLUS_PLUS; }
"--"            { return MINUS_MINUS; }
">"             { return GREATER; }
">="            { return GREATER_EQUAL; }
"<"             { return LESS; }
"<="            { return LESS_EQUAL; }

"{"             { return LEFT_CURLY_BRACKET; }
"}"             { return RIGHT_CURLY_BRACKET; }
"["             { return LEFT_SQUARE_BRACKET; }
"]"             { return RIGHT_SQUARE_BRACKET; }
"("             { return LEFT_PARENTHESIS; }
")"             { return RIGHT_PARENTHESIS; }
";"             { return SEMICOLON; }
","             { return COMMA; }
":"             { return COLON; }
"::"            { return DOUBLE_COLON; }
"."             { return DOT; }
".."            { return DOT_DOT; }

{linecomment}   { }
{space}         { }

{id}            { yylval.strVal = strdup(yytext); return IDENTIFIER; }
{integer}       { yylval.intVal = atoi(yytext); return INTEGER; }
{real}          { yylval.realVal = atof(yytext); return REAL; }

\"                  {   
                        BEGIN(STRING_COND);
                        scannerUtil_initializeStringReader();
                        stringStartLine = yylineno;
                    }

<STRING_COND>\"     {   
                        char* string;
                        string = scannerUtil_getString();
                        yylval.strVal = strdup(string);
                        scannerUtil_freeStringReader();
                        BEGIN(INITIAL);
                        return STRING;
                    }

<STRING_COND>\n     {
                        scannerUtil_readChar('\n');
                    }


<STRING_COND>.           { scannerUtil_readChar(yytext[0]); }

<STRING_COND><<EOF>>    {   
                            printf("Error at line %d: String not closed.\n", stringStartLine); 
                            exit(1);
                        }


"/*"                {
                        BEGIN(BLOCK_COMMENT);
                        scannerUtil_initializeCommentReader();
                        scannerUtil_readComment(yylineno);
                    }

<BLOCK_COMMENT>"/*" {
                        scannerUtil_readComment(yylineno);
                    }

<BLOCK_COMMENT>"*/" {
                        BlockComment_T* comment;
                        unsigned int startLine;
                        char* content;

                        comment = scannerUtil_matchComment(yylineno);

                        startLine = commentReader_getCommentStart(comment);
                        content = commentReader_getCommentScope(comment);

                        free(comment);

                        if (scannerUtil_isFinalBlockComment()) {
                            scannerUtil_freeCommentReader();
                            BEGIN(INITIAL);
                        }
                    }

<BLOCK_COMMENT>{space}  {}

<BLOCK_COMMENT>.        {}

<BLOCK_COMMENT><<EOF>>  {
                            printf("Error: Block comment not closed.\n"); 
                            exit(1);
                        }

.       { printf("Did not find any match.\n");}

%%
