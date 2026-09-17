%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.tab.h"

int current_column = 1;

#define YY_USER_ACTION \
    yylloc.first_line = yylloc.last_line = yylineno; \
    yylloc.first_column = current_column; \
    yylloc.last_column = current_column + yyleng - 1; \
    current_column += yyleng;
%}

%option noyywrap
%option yylineno
%option nounput noinput

%x BLOCK_COMMENT

%%
\n                  { current_column = 1; }
[ \t\r]+            { /* Ignora espaços */ }

"//".*              { /* Comentário de linha */ }
"/*"                { BEGIN(BLOCK_COMMENT); }

<BLOCK_COMMENT>"*/" { BEGIN(INITIAL); }
<BLOCK_COMMENT>\n   { current_column = 1; }
<BLOCK_COMMENT>.    { /* Ignora dentro do bloco */ }
<BLOCK_COMMENT><<EOF>> { 
                      fprintf(stderr, "Erro Léxico na linha %d: Comentário de bloco não fechado (EOF inesperado).\n", yylineno); 
                      exit(1); 
                    }

"=="                { return OP_EQ; }
"!="                { return OP_NEQ; }
"<="                { return OP_LE; }
">="                { return OP_GE; }
"&&"                { return OP_AND; }
"||"                { return OP_OR; }

"++"                { return OP_INC; }
"--"                { return OP_DEC; }
"+="                { return OP_PLUS_ASSIGN; }
"-="                { return OP_MINUS_ASSIGN; }
"*="                { return OP_MULT_ASSIGN; }
"/="                { return OP_DIV_ASSIGN; }

"+"                 { return OP_PLUS; }
"-"                 { return OP_MINUS; }
"*"                 { return OP_MULT; }
"/"                 { return OP_DIV; }
"%"                 { return OP_MOD; }
"="                 { return OP_ASSIGN; }
"<"                 { return OP_LT; }
">"                 { return OP_GT; }
"!"                 { return OP_NOT; }

"?"                 { return OP_QUESTION; }
":"                 { return OP_COLON; }

"("                 { return DELIM_LPAREN; }
")"                 { return DELIM_RPAREN; }
"{"                 { return DELIM_LBRACE; }
"}"                 { return DELIM_RBRACE; }
"["                 { return DELIM_LBRACKET; }
"]"                 { return DELIM_RBRACKET; }
";"                 { return DELIM_SEMICOLON; }
","                 { return DELIM_COMMA; }

.                   { 
                        printf("Lexical Error (%d:%d): %s\n", yylineno, yylloc.first_column, yytext); 
                        return TOKEN_ERROR; 
                    }
%%