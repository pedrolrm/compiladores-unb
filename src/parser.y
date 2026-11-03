%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scanner.h"
#include "semantic.h"

extern int yylex();
extern FILE *yyin;
void yyerror(const char *s);
%}

%code requires {
#include "ast.h"
}

/* Habilita o rastreamento automático de linhas e colunas (YYLTYPE) */
%locations

/* Estrutura para armazenar o valor semântico de um token ou nó (yylval) */
%union {
    int intval;
    float floatval;
    char charval;
    char *strval;
    ASTNode *node;
    DataType dt;
}

/* ========================================================================= */
/* DEFINIÇÃO DE TOKENS                                                       */
/* ========================================================================= */

/* Palavras-chave básicas */
%token KW_INT KW_FLOAT KW_CHAR KW_VOID
%token KW_IF KW_ELSE KW_WHILE KW_FOR KW_RETURN

/* Controle de fluxo e seleção (tokens preservados no scanner) */
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
%token DELIM_DOT

/* Token genérico de erro (caso haja caractere inválido) */
%token TOKEN_ERROR

/* ========================================================================= */
/* TIPOS DOS NÃO-TERMINAIS                                                   */
/* ========================================================================= */
%type <node> program statement function_definition
%type <node> parameter_list_opt parameter_list parameter
%type <node> compound_statement statement_list
%type <node> selection_statement
%type <node> iteration_statement for_init expression_opt
%type <node> jump_statement declaration
%type <node> init_declarator_list init_declarator declarator
%type <node> expression_statement expression postfix_expression primary_expression
%type <node> argument_list_opt argument_list
%type <dt> type_specifier

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

/* Resolução do "dangling else" */
%precedence LOWER_THAN_ELSE
%precedence KW_ELSE

%start program

%%
/* ========================================================================= */
/* ESTRUTURA BASE DO PROGRAMA                                                 */
/* ========================================================================= */
program
    : %empty {
        $$ = NULL;
        ast_root = NULL;
    }
    | program statement {
        if ($2) {
            $$ = ast_append_sibling($1, $2);
        } else {
            $$ = $1;
        }
        ast_root = $$;
    }
    ;

statement
    : declaration           { $$ = $1; }
    | function_definition  { $$ = $1; }
    | expression_statement { $$ = $1; }
    | compound_statement   { $$ = $1; }
    | selection_statement  { $$ = $1; }
    | iteration_statement  { $$ = $1; }
    | jump_statement       { $$ = $1; }
    ;

/* ------------------------------------------------------------------------- */
/* Definição de função: tipo de retorno (incl. void), parâmetros e corpo.     */
/* ------------------------------------------------------------------------- */
function_definition
    : type_specifier IDENTIFIER DELIM_LPAREN parameter_list_opt DELIM_RPAREN compound_statement {
        $$ = ast_create_func_def($1, $2, $4, $6, @2.first_line, @2.first_column);
        free($2);
    }
    | KW_VOID IDENTIFIER DELIM_LPAREN parameter_list_opt DELIM_RPAREN compound_statement {
        $$ = ast_create_func_def(data_type_primitive(TYPE_VOID), $2, $4, $6, @2.first_line, @2.first_column);
        free($2);
    }
    ;

parameter_list_opt
    : %empty        { $$ = NULL; }
    | KW_VOID       { $$ = NULL; }
    | parameter_list { $$ = $1; }
    ;

parameter_list
    : parameter { $$ = $1; }
    | parameter_list DELIM_COMMA parameter {
        $$ = ast_append_sibling($1, $3);
    }
    ;

parameter
    : type_specifier IDENTIFIER {
        $$ = ast_create_param($1, $2, @2.first_line, @2.first_column);
        free($2);
    }
    ;

/* ------------------------------------------------------------------------- */
/* Bloco de comandos: permite aninhamento arbitrário de escopos.              */
/* ------------------------------------------------------------------------- */
compound_statement
    : DELIM_LBRACE DELIM_RBRACE {
        $$ = ast_create_block(NULL, @1.first_line, @1.first_column);
    }
    | DELIM_LBRACE statement_list DELIM_RBRACE {
        $$ = ast_create_block($2, @1.first_line, @1.first_column);
    }
    ;

statement_list
    : statement { $$ = $1; }
    | statement_list statement {
        if ($2) {
            $$ = ast_append_sibling($1, $2);
        } else {
            $$ = $1;
        }
    }
    ;

/* ------------------------------------------------------------------------- */
/* Seleção: if / if-else (com dangling else).                                 */
/* ------------------------------------------------------------------------- */
selection_statement
    : KW_IF DELIM_LPAREN expression DELIM_RPAREN statement %prec LOWER_THAN_ELSE {
        $$ = ast_create_if($3, $5, NULL, @1.first_line, @1.first_column);
    }
    | KW_IF DELIM_LPAREN expression DELIM_RPAREN statement KW_ELSE statement {
        $$ = ast_create_if($3, $5, $7, @1.first_line, @1.first_column);
    }
    ;

/* ------------------------------------------------------------------------- */
/* Iteração: while e for (init pode ser declaração ou expressão).             */
/* ------------------------------------------------------------------------- */
iteration_statement
    : KW_WHILE DELIM_LPAREN expression DELIM_RPAREN statement {
        $$ = ast_create_while($3, $5, @1.first_line, @1.first_column);
    }
    | KW_FOR DELIM_LPAREN for_init expression_opt DELIM_SEMICOLON expression_opt DELIM_RPAREN statement {
        $$ = ast_create_for($3, $4, $6, $8, @1.first_line, @1.first_column);
    }
    ;

for_init
    : declaration          { $$ = $1; }
    | expression_statement { $$ = $1; }
    ;

expression_opt
    : %empty     { $$ = NULL; }
    | expression { $$ = $1; }
    ;

/* ------------------------------------------------------------------------- */
/* Desvios de fluxo em laços.                                                 */
/* ------------------------------------------------------------------------- */
jump_statement
    : KW_BREAK DELIM_SEMICOLON {
        $$ = ast_create_break(@1.first_line, @1.first_column);
    }
    | KW_CONTINUE DELIM_SEMICOLON {
        $$ = ast_create_continue(@1.first_line, @1.first_column);
    }
    | KW_RETURN expression_opt DELIM_SEMICOLON {
        $$ = ast_create_return($2, @1.first_line, @1.first_column);
    }
    ;

/* ------------------------------------------------------------------------- */
/* Declarações de variáveis                                                  */
/* ------------------------------------------------------------------------- */
declaration
    : type_specifier init_declarator_list DELIM_SEMICOLON {
        ASTNode *curr = $2;
        while (curr) {
            curr->data_type.base = $1.base;
            curr = curr->next;
        }
        $$ = $2;
    }
    ;

type_specifier
    : KW_INT {
        $$ = data_type_primitive(TYPE_INT);
    }
    | KW_FLOAT {
        $$ = data_type_primitive(TYPE_FLOAT);
    }
    | KW_CHAR {
        $$ = data_type_primitive(TYPE_CHAR);
    }
    | KW_BOOL {
        $$ = data_type_primitive(TYPE_BOOL);
    }
    ;

init_declarator_list
    : init_declarator { $$ = $1; }
    | init_declarator_list DELIM_COMMA init_declarator {
        $$ = ast_append_sibling($1, $3);
    }
    ;

init_declarator
    : declarator { $$ = $1; }
    | declarator OP_ASSIGN expression {
        $1->left = $3;
        $$ = $1;
    }
    ;

/* Declarador escalar ou vetor 1D */
declarator
    : IDENTIFIER {
        $$ = ast_create_var_decl(data_type_primitive(TYPE_UNKNOWN), $1, NULL, @1.first_line, @1.first_column);
        free($1);
    }
    | IDENTIFIER DELIM_LBRACKET expression DELIM_RBRACKET {
        ASTNode *decl = ast_create_var_decl(data_type_primitive(TYPE_UNKNOWN), $1, NULL, @1.first_line, @1.first_column);
        decl->data_type.is_array = true;
        if ($3 && $3->type == AST_LIT_INT) {
            decl->data_type.array_size = $3->int_val;
        }
        ast_free($3);
        free($1);
        $$ = decl;
    }
    | IDENTIFIER DELIM_LBRACKET DELIM_RBRACKET {
        ASTNode *decl = ast_create_var_decl(data_type_primitive(TYPE_UNKNOWN), $1, NULL, @1.first_line, @1.first_column);
        decl->data_type.is_array = true;
        decl->data_type.array_size = 0;
        free($1);
        $$ = decl;
    }
    ;

/* ------------------------------------------------------------------------- */
/* Comando de expressão e expressões                                          */
/* ------------------------------------------------------------------------- */
expression_statement
    : expression DELIM_SEMICOLON {
        $$ = ast_create_expr_stmt($1, @1.first_line, @1.first_column);
    }
    | DELIM_SEMICOLON {
        $$ = NULL;
    }
    ;

expression
    : postfix_expression OP_ASSIGN expression {
        $$ = ast_create_assign(OP_ASSIGN, $1, $3, @2.first_line, @2.first_column);
    }
    | postfix_expression OP_PLUS_ASSIGN expression {
        $$ = ast_create_assign(OP_PLUS_ASSIGN, $1, $3, @2.first_line, @2.first_column);
    }
    | postfix_expression OP_MINUS_ASSIGN expression {
        $$ = ast_create_assign(OP_MINUS_ASSIGN, $1, $3, @2.first_line, @2.first_column);
    }
    | postfix_expression OP_MULT_ASSIGN expression {
        $$ = ast_create_assign(OP_MULT_ASSIGN, $1, $3, @2.first_line, @2.first_column);
    }
    | postfix_expression OP_DIV_ASSIGN expression {
        $$ = ast_create_assign(OP_DIV_ASSIGN, $1, $3, @2.first_line, @2.first_column);
    }
    | expression OP_OR expression {
        $$ = ast_create_bin_op(OP_OR, $1, $3, @2.first_line, @2.first_column);
    }
    | expression OP_AND expression {
        $$ = ast_create_bin_op(OP_AND, $1, $3, @2.first_line, @2.first_column);
    }
    | expression OP_EQ expression {
        $$ = ast_create_bin_op(OP_EQ, $1, $3, @2.first_line, @2.first_column);
    }
    | expression OP_NEQ expression {
        $$ = ast_create_bin_op(OP_NEQ, $1, $3, @2.first_line, @2.first_column);
    }
    | expression OP_LT expression {
        $$ = ast_create_bin_op(OP_LT, $1, $3, @2.first_line, @2.first_column);
    }
    | expression OP_LE expression {
        $$ = ast_create_bin_op(OP_LE, $1, $3, @2.first_line, @2.first_column);
    }
    | expression OP_GT expression {
        $$ = ast_create_bin_op(OP_GT, $1, $3, @2.first_line, @2.first_column);
    }
    | expression OP_GE expression {
        $$ = ast_create_bin_op(OP_GE, $1, $3, @2.first_line, @2.first_column);
    }
    | expression OP_PLUS expression {
        $$ = ast_create_bin_op(OP_PLUS, $1, $3, @2.first_line, @2.first_column);
    }
    | expression OP_MINUS expression {
        $$ = ast_create_bin_op(OP_MINUS, $1, $3, @2.first_line, @2.first_column);
    }
    | expression OP_MULT expression {
        $$ = ast_create_bin_op(OP_MULT, $1, $3, @2.first_line, @2.first_column);
    }
    | expression OP_DIV expression {
        $$ = ast_create_bin_op(OP_DIV, $1, $3, @2.first_line, @2.first_column);
    }
    | expression OP_MOD expression {
        $$ = ast_create_bin_op(OP_MOD, $1, $3, @2.first_line, @2.first_column);
    }
    | OP_MINUS expression %prec UMINUS {
        $$ = ast_create_un_op(OP_MINUS, $2, @1.first_line, @1.first_column);
    }
    | OP_NOT expression {
        $$ = ast_create_un_op(OP_NOT, $2, @1.first_line, @1.first_column);
    }
    | DELIM_LPAREN expression DELIM_RPAREN {
        $$ = $2;
    }
    | postfix_expression {
        $$ = $1;
    }
    ;

/* Pós-fixados: indexação de array */
postfix_expression
    : primary_expression { $$ = $1; }
    | postfix_expression DELIM_LBRACKET expression DELIM_RBRACKET {
        $$ = ast_create_array_access($1, $3, @2.first_line, @2.first_column);
    }
    ;

primary_expression
    : IDENTIFIER {
        $$ = ast_create_identifier($1, @1.first_line, @1.first_column);
        free($1);
    }
    | IDENTIFIER DELIM_LPAREN argument_list_opt DELIM_RPAREN {
        $$ = ast_create_call($1, $3, @1.first_line, @1.first_column);
        free($1);
    }
    | INT_LITERAL {
        $$ = ast_create_lit_int($1, @1.first_line, @1.first_column);
    }
    | FLOAT_LITERAL {
        $$ = ast_create_lit_float($1, @1.first_line, @1.first_column);
    }
    | CHAR_LITERAL {
        $$ = ast_create_lit_char($1, @1.first_line, @1.first_column);
    }
    | STRING_LITERAL {
        $$ = ast_create_lit_string($1, @1.first_line, @1.first_column);
        free($1);
    }
    | KW_TRUE {
        $$ = ast_create_lit_bool(true, @1.first_line, @1.first_column);
    }
    | KW_FALSE {
        $$ = ast_create_lit_bool(false, @1.first_line, @1.first_column);
    }
    ;

argument_list_opt
    : %empty        { $$ = NULL; }
    | argument_list { $$ = $1; }
    ;

argument_list
    : expression { $$ = $1; }
    | argument_list DELIM_COMMA expression {
        $$ = ast_append_sibling($1, $3);
    }
    ;
%%

void yyerror(const char *s) {
    /* Utiliza as variáveis yylloc (geradas pelo %locations) para reportar erros com precisão */
    fprintf(stderr, "Erro sintático na linha %d, coluna %d: %s\n", yylloc.first_line, yylloc.first_column, s);
}

int main(int argc, char **argv) {
    int parse_mode = 0;
    int print_ast = 0;
    int print_symbols = 0;
    int check_semantic = 0;
    char *filepath = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--parse") == 0 || strcmp(argv[i], "-p") == 0) {
            parse_mode = 1;
        } else if (strcmp(argv[i], "--ast") == 0 || strcmp(argv[i], "-a") == 0) {
            parse_mode = 1;
            print_ast = 1;
        } else if (strcmp(argv[i], "--symbols") == 0 || strcmp(argv[i], "-t") == 0) {
            parse_mode = 1;
            print_symbols = 1;
            check_semantic = 1;
        } else if (strcmp(argv[i], "--check") == 0 || strcmp(argv[i], "-c") == 0) {
            parse_mode = 1;
            check_semantic = 1;
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

    if (parse_mode) {
        yyin = source;
        status = yyparse();
        if (status == 0 && lexical_errors_count > 0) {
            status = 1;
        }

        if (status == 0 && print_ast && ast_root) {
            ast_print(ast_root, 0);
        }

        if (status == 0 && (check_semantic || print_symbols) && ast_root) {
            Scope *global_scope = NULL;
            int sem_errors = semantic_analyze(ast_root, &global_scope);
            if (sem_errors > 0) {
                status = 1;
            }

            if (print_symbols && global_scope) {
                symtab_print(global_scope, 0);
            }

            if (global_scope) {
                symtab_free(global_scope);
            }
        }

        if (ast_root) {
            ast_free(ast_root);
            ast_root = NULL;
        }
    } else {
        status = run_scanner(source);
    }

    if (source != stdin) {
        fclose(source);
    }

    return status;
}
