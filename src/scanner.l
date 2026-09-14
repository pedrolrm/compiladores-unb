%{
#include <stdio.h>
#include "parser.tab.h" /* Gerado pelo Bison */
%}

%option noyywrap
%option yylineno

%%
[ \t\n]+    { /* Ignora espaços em branco */ }
.           { printf("Lexical Error: %s\n", yytext); }
%%
