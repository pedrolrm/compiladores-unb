%{
#include <stdio.h>
#include "parser.tab.h" /* Gerado pelo Bison */

/* Coluna inicial do token atual (linha vem de yylineno) */
int yycolumn = 1;

/* Avanca a coluna antes de cada acao */
#define YY_USER_ACTION yycolumn += yyleng;
%}

%option noyywrap
%option yylineno
%option nounput noinput

%%
\n          { yycolumn = 1; /* Nova linha: reinicia a coluna */ }
[ \t\r]+    { /* Ignora espacos em branco */ }
.           { printf("Lexical Error (%d:%d): %s\n", yylineno, yycolumn - yyleng, yytext); }
%%
