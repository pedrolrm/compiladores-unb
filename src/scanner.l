%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.tab.h" /* Header gerado pelo Bison com a definição dos tokens */
/* Se os tokens estiverem apenas no token_reference.h, inclua-o também: #include "token_reference.h" */

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
    /* Operadores Relacionais e Lógicos Compostos (Maior prioridade de match) */
"=="                { return OP_EQ; }
"!="                { return OP_NEQ; }
"<="                { return OP_LE; }
">="                { return OP_GE; }
"&&"                { return OP_AND; }
"||"                { return OP_OR; }

    /* Incremento, Decremento e Atribuição Composta */
"++"                { return OP_INC; }
"--"                { return OP_DEC; }
"+="                { return OP_PLUS_ASSIGN; }
"-="                { return OP_MINUS_ASSIGN; }
"*="                { return OP_MULT_ASSIGN; }
"/="                { return OP_DIV_ASSIGN; }

    /* Operadores Aritméticos, Relacionais e Lógicos Simples */
"+"                 { return OP_PLUS; }
"-"                 { return OP_MINUS; }
"*"                 { return OP_MULT; }
"/"                 { return OP_DIV; }
"%"                 { return OP_MOD; }
"="                 { return OP_ASSIGN; }
"<"                 { return OP_LT; }
">"                 { return OP_GT; }
"!"                 { return OP_NOT; }

    /* Operador Ternário */
"?"                 { return OP_QUESTION; }
":"                 { return OP_COLON; }

    /* Delimitadores */
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