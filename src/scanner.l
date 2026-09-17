%{
#include <stdio.h>
#include <stdlib.h>
#include "parser.tab.h" /* Gerado pelo Bison */
%}

%option noyywrap
%option yylineno

/* Declaração de um estado exclusivo para comentários de bloco */
%x BLOCK_COMMENT

%%
[ \t\n]+            { /* Ignora espaços em branco e quebras de linha */ }

"//".*              { /* Comentário de linha: ignora tudo até o final da linha atual */ }

"/*"                { BEGIN(BLOCK_COMMENT); /* Entra no estado de comentário de bloco */ }

<BLOCK_COMMENT>"*/" { BEGIN(INITIAL); /* Sai do estado e volta ao processamento normal */ }
<BLOCK_COMMENT>.|\n { /* Ignora qualquer caractere, incluindo quebras de linha, dentro do bloco */ }
<BLOCK_COMMENT><<EOF>> { 
                      /* Dispara erro e interrompe a execução caso o arquivo acabe no meio do comentário */
                      fprintf(stderr, "Erro Léxico na linha %d: Comentário de bloco não fechado (EOF inesperado).\n", yylineno); 
                      exit(1); 
                    }

.                   { printf("Lexical Error: %s\n", yytext); }
%%