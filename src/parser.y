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

/* Declaracao de Tokens */
%token IDENTIFIER

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
/* Gramatica basica (placeholder) */
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
    fprintf(stderr, "Erro sintatico na linha %d: %s\n", yylineno, s);
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
