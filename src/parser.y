%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int yylex();
extern FILE *yyin;
extern int yylineno;
extern int yycolumn;
extern char *yytext;
void yyerror(const char *s);
%}

/* Habilita o rastreamento automático de linhas e colunas (YYLTYPE) */
%locations

/* Estrutura para armazenar o valor semântico de um token (yylval) */
%union {
    int intval;
    float floatval;
    char charval;
    char *strval;
}

/* ========================================================================= */
/* DEFINIÇÃO DE TOKENS (Equivalente ao antigo enum TokenType)                 */
/* ========================================================================= */

/* Palavras-chave básicas */
%token KW_INT KW_FLOAT KW_CHAR KW_VOID
%token KW_IF KW_ELSE KW_WHILE KW_FOR KW_RETURN

/* Controle de fluxo e seleção */
%token KW_BREAK KW_CONTINUE KW_SWITCH KW_CASE KW_DEFAULT

/* Tipos e modificadores extras */
%token KW_DOUBLE KW_BOOL KW_LONG KW_SHORT KW_UNSIGNED KW_CONST

/* Estruturas de dados e outros laços */
%token KW_STRUCT KW_TYPEDEF KW_DO

/* Identificadores e Literais com valores semânticos */
%token <strval> IDENTIFIER
%token <intval> INT_LITERAL
%token <floatval> FLOAT_LITERAL
%token <strval> STRING_LITERAL
%token <charval> CHAR_LITERAL
%token KW_TRUE KW_FALSE

/* Operadores Aritméticos e Lógicos */
%token OP_PLUS OP_MINUS OP_MULT OP_DIV OP_MOD
%token OP_ASSIGN OP_EQ OP_NEQ OP_LT OP_LE OP_GT OP_GE
%token OP_AND OP_OR OP_NOT

/* Operadores de Incremento/Decremento e Atribuição Composta */
%token OP_INC OP_DEC
%token OP_PLUS_ASSIGN OP_MINUS_ASSIGN OP_MULT_ASSIGN OP_DIV_ASSIGN

/* Operador Ternário */
%token OP_QUESTION OP_COLON

/* Delimitadores e Pontuação */
%token DELIM_LPAREN DELIM_RPAREN
%token DELIM_LBRACE DELIM_RBRACE
%token DELIM_LBRACKET DELIM_RBRACKET
%token DELIM_SEMICOLON DELIM_COMMA

/* Token genérico de erro (caso haja caractere inválido) */
%token TOKEN_ERROR

/* Palavras reservadas: tipos */
%token KW_INT KW_FLOAT KW_CHAR KW_VOID KW_DOUBLE KW_BOOL
%token KW_LONG KW_SHORT KW_UNSIGNED KW_CONST

/* Palavras reservadas: controle de fluxo */
%token KW_IF KW_ELSE KW_WHILE KW_FOR KW_DO
%token KW_RETURN KW_BREAK KW_CONTINUE
%token KW_SWITCH KW_CASE KW_DEFAULT

/* Palavras reservadas: estruturas de dados */
%token KW_STRUCT KW_TYPEDEF

/* Literais booleanos */
%token KW_TRUE KW_FALSE

%%
/* Gramática básica (placeholder) */
program: /* vazio */
       ;
%%

#include "tokens.h"

/* Modo scanner: consome a entrada e imprime um token por linha. */
static void dump_tokens(void) {
    int token;

    while ((token = yylex()) != 0) {
        printf("%d:%d\t%s\t%s\n",
               yylineno, yycolumn - (int)strlen(yytext),
               token_name(token), yytext);
    }
}

void yyerror(const char *s) {
    /* Utiliza as variáveis yylloc (geradas pelo %locations) para reportar erros com precisão */
    fprintf(stderr, "Erro sintático na linha %d, coluna %d: %s\n", yylloc.first_line, yylloc.first_column, s);
}

int main(int argc, char **argv) {
    /* Le do arquivo informado ou da entrada padrao */
    FILE *source = stdin;
    int only_tokens = 0;
    int i;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--tokens") == 0) {
            only_tokens = 1;
        } else {
            source = fopen(argv[i], "r");
            if (!source) {
                perror(argv[i]);
                return 1;
            }
        }
    }
    yyin = source;

    int status = 0;

    if (only_tokens) {
        dump_tokens();
    } else {
        status = yyparse();
    }

    if (source != stdin) {
        fclose(source);
    }
    return status;
}
