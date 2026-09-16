%{
#include <stdio.h>
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
\n          { current_column = 1; /* Reseta a coluna ao pular linha */ }
[ \t\r]+    { /* Ignora espaços em branco, yy_user_action já atualiza a coluna */ }
.           { 
                printf("Lexical Error (%d:%d): %s\n", yylineno, yylloc.first_column, yytext); 
                return TOKEN_ERROR; 
            }
%%
