%{
#include <stdio.h>
#include <stdlib.h>

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

/* ========================================================================= */
/* PRECEDÊNCIA E ASSOCIATIVIDADE (menor -> maior precedência)                 */
/* ========================================================================= */
%precedence OP_ASSIGN OP_PLUS_ASSIGN OP_MINUS_ASSIGN OP_MULT_ASSIGN OP_DIV_ASSIGN
%left OP_OR
%left OP_AND
%left OP_EQ OP_NEQ
%left OP_LT OP_LE OP_GT OP_GE
%left OP_PLUS OP_MINUS
%left OP_MULT OP_DIV OP_MOD
%precedence OP_NOT UMINUS      /* operadores unários */

/* Resolução do "dangling else": o KW_ELSE tem precedência maior que um
   comando if sem else, fazendo o parser preferir shift (else liga ao if
   mais próximo). */
%precedence LOWER_THAN_ELSE
%precedence KW_ELSE

%start program

%%
/* ========================================================================= */
/* ESTRUTURA BASE DO PROGRAMA                                                 */
/* ========================================================================= */
program
    : %empty
    | program statement
    ;

statement
    : declaration
    | expression_statement
    | compound_statement
    | selection_statement
    | iteration_statement
    | labeled_statement
    | jump_statement
    ;

/* ------------------------------------------------------------------------- */
/* Bloco de comandos: permite aninhamento arbitrário de escopos.              */
/* ------------------------------------------------------------------------- */
compound_statement
    : DELIM_LBRACE DELIM_RBRACE
    | DELIM_LBRACE statement_list DELIM_RBRACE
    ;

statement_list
    : statement
    | statement_list statement
    ;

/* ------------------------------------------------------------------------- */
/* Seleção: if / if-else (com dangling else) e switch.                        */
/* ------------------------------------------------------------------------- */
selection_statement
    : KW_IF DELIM_LPAREN expression DELIM_RPAREN statement %prec LOWER_THAN_ELSE
    | KW_IF DELIM_LPAREN expression DELIM_RPAREN statement KW_ELSE statement
    | KW_SWITCH DELIM_LPAREN expression DELIM_RPAREN statement
    ;

/* Rótulos do switch (case/default) tratados como labeled statements. */
labeled_statement
    : KW_CASE expression OP_COLON statement
    | KW_DEFAULT OP_COLON statement
    ;

/* ------------------------------------------------------------------------- */
/* Iteração: while e for (init pode ser declaração ou expressão).             */
/* ------------------------------------------------------------------------- */
iteration_statement
    : KW_WHILE DELIM_LPAREN expression DELIM_RPAREN statement
    | KW_FOR DELIM_LPAREN for_init expression_opt DELIM_SEMICOLON expression_opt DELIM_RPAREN statement
    ;

for_init
    : declaration
    | expression_statement
    ;

expression_opt
    : %empty
    | expression
    ;

/* ------------------------------------------------------------------------- */
/* Desvios de fluxo em laços e switch.                                        */
/* ------------------------------------------------------------------------- */
jump_statement
    : KW_BREAK DELIM_SEMICOLON
    | KW_CONTINUE DELIM_SEMICOLON
    ;

/* ------------------------------------------------------------------------- */
/* Declarações de variáveis: int/float/char/bool, com inicialização opcional  */
/* e múltiplos declaradores separados por vírgula.                            */
/* ------------------------------------------------------------------------- */
declaration
    : type_specifier init_declarator_list DELIM_SEMICOLON
    ;

type_specifier
    : KW_INT
    | KW_FLOAT
    | KW_CHAR
    | KW_BOOL
    ;

init_declarator_list
    : init_declarator
    | init_declarator_list DELIM_COMMA init_declarator
    ;

init_declarator
    : IDENTIFIER
    | IDENTIFIER OP_ASSIGN expression
    ;

/* ------------------------------------------------------------------------- */
/* Comando de expressão (inclui atribuições) e comando vazio.                 */
/* ------------------------------------------------------------------------- */
expression_statement
    : expression DELIM_SEMICOLON
    | DELIM_SEMICOLON
    ;

/* ------------------------------------------------------------------------- */
/* Expressões com precedência de operadores.                                  */
/* A ambiguidade é resolvida pelas declarações de precedência acima.          */
/* ------------------------------------------------------------------------- */
expression
    : IDENTIFIER OP_ASSIGN expression
    | IDENTIFIER OP_PLUS_ASSIGN expression
    | IDENTIFIER OP_MINUS_ASSIGN expression
    | IDENTIFIER OP_MULT_ASSIGN expression
    | IDENTIFIER OP_DIV_ASSIGN expression
    | expression OP_OR expression
    | expression OP_AND expression
    | expression OP_EQ expression
    | expression OP_NEQ expression
    | expression OP_LT expression
    | expression OP_LE expression
    | expression OP_GT expression
    | expression OP_GE expression
    | expression OP_PLUS expression
    | expression OP_MINUS expression
    | expression OP_MULT expression
    | expression OP_DIV expression
    | expression OP_MOD expression
    | OP_MINUS expression %prec UMINUS
    | OP_NOT expression
    | DELIM_LPAREN expression DELIM_RPAREN
    | primary_expression
    ;

primary_expression
    : IDENTIFIER
    | INT_LITERAL
    | FLOAT_LITERAL
    | CHAR_LITERAL
    | STRING_LITERAL
    | KW_TRUE
    | KW_FALSE
    ;
%%

void yyerror(const char *s) {
    /* Utiliza as variáveis yylloc (geradas pelo %locations) para reportar erros com precisão */
    fprintf(stderr, "Erro sintático na linha %d, coluna %d: %s\n", yylloc.first_line, yylloc.first_column, s);
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
