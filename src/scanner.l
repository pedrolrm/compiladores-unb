%{
#include <stdio.h>
#include "tokens.h" /* Inclui parser.tab.h (gerado pelo Bison) */
#include <string.h>
#include "parser.tab.h" /* Header gerado pelo Bison com a definição dos tokens */

/* Variavel global para controlar a coluna atual */
int current_column = 1;

/* Macro executada antes de cada ação para atualizar yylloc (linha/coluna) */
#define YY_USER_ACTION \
    yylloc.first_line = yylloc.last_line = yylineno; \
    yylloc.first_column = current_column; \
    yylloc.last_column = current_column + yyleng - 1; \
    current_column += yyleng;
%}

%option noyywrap
%option yylineno
%option nounput noinput

%%
\n                          { yycolumn = 1; /* Nova linha: reinicia a coluna */ }
[ \t\r]+                    { /* Ignora espacos em branco */ }
[a-zA-Z_][a-zA-Z0-9_]*      { return keyword_lookup(yytext); }
.                           { printf("Lexical Error (%d:%d): %s\n", yylineno, yycolumn - yyleng, yytext); }
%%
