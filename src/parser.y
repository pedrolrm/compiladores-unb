%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scanner.h"

extern int yylex();
extern FILE *yyin;
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

%%
/* Gramática básica (placeholder) */
program: /* vazio */
       ;
%%

void yyerror(const char *s) {
    /* Utiliza as variáveis yylloc (geradas pelo %locations) para reportar erros com precisão */
    fprintf(stderr, "Erro sintático na linha %d, coluna %d: %s\n", yylloc.first_line, yylloc.first_column, s);
}

int main(int argc, char **argv) {
    int parse_mode = 0;
    char *filepath = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--parse") == 0 || strcmp(argv[i], "-p") == 0) {
            parse_mode = 1;
        } else if (strcmp(argv[i], "--scan") == 0 || strcmp(argv[i], "-s") == 0) {
            parse_mode = 0;
        } else if (argv[i][0] != '-') {
            filepath = argv[i];
        }
    }

    FILE *source = stdin;

    if (filepath) {
        source = fopen(filepath, "r");
        if (!source) {
            perror(filepath);
            return 1;
        }
    }

    int status = 0;

    /*
     * Modo Scanner (padrão via CLI ao passar arquivo .c):
     * Consome a entrada e imprime a listagem formatada dos tokens gerados.
     * Caso o usuário passe a flag --parse ou -p, executa a análise sintática do Bison.
     */
    if (parse_mode) {
        yyin = source;
        status = yyparse();
        if (status == 0 && lexical_errors_count > 0) {
            status = 1;
        }
    } else {
        status = run_scanner(source);
    }

    if (source != stdin) {
        fclose(source);
    }

    return status;
}
