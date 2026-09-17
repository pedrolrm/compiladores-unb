%{
#include <stdio.h>
#include <stdlib.h>
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

/* Declaração de um estado exclusivo para comentários de bloco */
%x BLOCK_COMMENT

%%
\n                  { current_column = 1; /* Reseta a coluna ao pular linha */ }
[ \t\r]+            { /* Ignora espaços em branco, yy_user_action já atualiza a coluna */ }

"//".*              { /* Comentário de linha: ignora tudo até o final da linha atual */ }

"/*"                { BEGIN(BLOCK_COMMENT); /* Entra no estado de comentário de bloco */ }

<BLOCK_COMMENT>"*/" { BEGIN(INITIAL); /* Sai do estado e volta ao processamento normal */ }
<BLOCK_COMMENT>\n   { current_column = 1; /* Reseta a coluna nas quebras de linha dentro do bloco */ }
<BLOCK_COMMENT>.    { /* Ignora qualquer outro caractere dentro do bloco */ }
<BLOCK_COMMENT><<EOF>> { 
                      /* Dispara erro e interrompe a execução caso o arquivo acabe no meio do comentário */
                      fprintf(stderr, "Erro Léxico na linha %d: Comentário de bloco não fechado (EOF inesperado).\n", yylineno); 
                      exit(1); 
                    }

.                   { 
                        printf("Lexical Error (%d:%d): %s\n", yylineno, yylloc.first_column, yytext); 
                        return TOKEN_ERROR; 
                    }
%%