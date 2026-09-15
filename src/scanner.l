%{
#include <stdio.h>
#include "tokens.h" /* Inclui parser.tab.h (gerado pelo Bison) */

/* Coluna inicial do token atual (linha vem de yylineno) */
int yycolumn = 1;

/* Avanca a coluna antes de cada acao */
#define YY_USER_ACTION yycolumn += yyleng;
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
