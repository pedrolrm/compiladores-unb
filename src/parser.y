%{
#include <stdio.h>
#include <stdlib.h>

extern int yylex();
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
    return yyparse();
}
