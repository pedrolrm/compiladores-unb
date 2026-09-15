%{
#include <stdio.h>
#include <stdlib.h>

extern int yylex();
extern FILE *yyin;
void yyerror(const char *s);
%}

/* Declaracao de Tokens */
%token IDENTIFIER

%%
/* Gramatica basica (placeholder) */
program: /* vazio */
       ;
%%

void yyerror(const char *s) {
    extern int yylineno;
    fprintf(stderr, "Erro sintatico na linha %d: %s\n", yylineno, s);
}

int main(int argc, char **argv) {
    /* Le do arquivo informado ou da entrada padrao */
    FILE *source = stdin;

    if (argc > 1) {
        source = fopen(argv[1], "r");
        if (!source) {
            perror(argv[1]);
            return 1;
        }
    }
    yyin = source;

    int status = yyparse();

    if (source != stdin) {
        fclose(source);
    }
    return status;
}
