#ifndef AST_H
#define AST_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/*
 * Tipos de dados suportados pelo sistema de tipos do compilador.
 */
typedef enum {
    TYPE_UNKNOWN = 0,
    TYPE_VOID,
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_CHAR,
    TYPE_BOOL,
    TYPE_STRUCT
} BaseType;

/*
 * Descritor de tipo com suporte a structs e arrays.
 */
typedef struct DataType {
    BaseType base;
    char *struct_name;    /* Preenchido quando base == TYPE_STRUCT */
    bool is_array;
    int array_size;       /* Tamanho fixo do array (se > 0) ou 0 para indefinido */
} DataType;

/*
 * Enumeração de tipos de nós da Árvore Sintática Abstrata (AST).
 */
typedef enum {
    /* Raiz do programa */
    AST_PROGRAM,

    /* Declarações */
    AST_VAR_DECL,
    AST_FUNC_DEF,
    AST_PARAM_DECL,
    AST_STRUCT_DECL,

    /* Comandos (Statements) */
    AST_BLOCK,
    AST_EXPR_STMT,
    AST_IF,
    AST_WHILE,
    AST_FOR,
    AST_SWITCH,
    AST_CASE,
    AST_DEFAULT,
    AST_BREAK,
    AST_CONTINUE,
    AST_RETURN,

    /* Expressões */
    AST_BIN_OP,
    AST_UN_OP,
    AST_ASSIGN,
    AST_CALL,
    AST_ARRAY_ACCESS,
    AST_MEMBER_ACCESS,

    /* Literais e Identificadores */
    AST_IDENTIFIER,
    AST_LIT_INT,
    AST_LIT_FLOAT,
    AST_LIT_CHAR,
    AST_LIT_STRING,
    AST_LIT_BOOL
} ASTNodeType;

/*
 * Estrutura unificada do nó da Árvore Sintática Abstrata.
 */
typedef struct ASTNode {
    ASTNodeType type;
    int line;
    int column;
    DataType data_type;       /* Tipo de dado da declaração ou inferido na expressão */

    /* Dados para literais e nós folha */
    char *name;               /* Identificador, nome de função, struct ou membro */
    int int_val;              /* Para literais inteiros e booleanos */
    float float_val;          /* Para literais de ponto flutuante */
    char char_val;            /* Para literais de caractere */
    char *str_val;            /* Para literais de string */
    int op;                   /* Código do operador (ex.: OP_PLUS, OP_EQ, etc.) */

    /* Filhos estruturados */
    struct ASTNode *left;     /* Filho esquerdo (condição em if/while, lado esquerdo em binop/atribuição) */
    struct ASTNode *right;    /* Filho direito (then em if, corpo em while, lado direito em binop) */
    struct ASTNode *extra;    /* Terceiro filho (else em if, passo em for) */

    /* Encadeamento de nós em listas (comandos num bloco, parâmetros, argumentos, declaradores) */
    struct ASTNode *next;
} ASTNode;

/* Construtores e utilitários de Tipos */
DataType data_type_primitive(BaseType base);
DataType data_type_struct(const char *struct_name);
DataType data_type_array(DataType elem_type, int size);
const char *base_type_to_string(BaseType base);
const char *data_type_to_string(const DataType *dt);
bool data_type_equals(const DataType *a, const DataType *b);

/* Construtores de Nós da AST */
ASTNode *ast_create_node(ASTNodeType type, int line, int col);
ASTNode *ast_create_program(ASTNode *declarations, int line, int col);
ASTNode *ast_create_var_decl(DataType dt, const char *name, ASTNode *init_expr, int line, int col);
ASTNode *ast_create_func_def(DataType ret_type, const char *name, ASTNode *params, ASTNode *body, int line, int col);
ASTNode *ast_create_param(DataType dt, const char *name, int line, int col);
ASTNode *ast_create_struct_decl(const char *name, ASTNode *members, int line, int col);

ASTNode *ast_create_block(ASTNode *statements, int line, int col);
ASTNode *ast_create_expr_stmt(ASTNode *expr, int line, int col);
ASTNode *ast_create_if(ASTNode *cond, ASTNode *then_branch, ASTNode *else_branch, int line, int col);
ASTNode *ast_create_while(ASTNode *cond, ASTNode *body, int line, int col);
ASTNode *ast_create_for(ASTNode *init, ASTNode *cond, ASTNode *step, ASTNode *body, int line, int col);
ASTNode *ast_create_switch(ASTNode *expr, ASTNode *body, int line, int col);
ASTNode *ast_create_case(ASTNode *expr, ASTNode *stmt, int line, int col);
ASTNode *ast_create_default(ASTNode *stmt, int line, int col);
ASTNode *ast_create_break(int line, int col);
ASTNode *ast_create_continue(int line, int col);
ASTNode *ast_create_return(ASTNode *expr, int line, int col);

ASTNode *ast_create_bin_op(int op, ASTNode *left, ASTNode *right, int line, int col);
ASTNode *ast_create_un_op(int op, ASTNode *operand, int line, int col);
ASTNode *ast_create_assign(int op, ASTNode *target, ASTNode *expr, int line, int col);
ASTNode *ast_create_call(const char *callee_name, ASTNode *args, int line, int col);
ASTNode *ast_create_array_access(ASTNode *array, ASTNode *index, int line, int col);
ASTNode *ast_create_member_access(ASTNode *object, const char *member, int line, int col);

ASTNode *ast_create_identifier(const char *name, int line, int col);
ASTNode *ast_create_lit_int(int val, int line, int col);
ASTNode *ast_create_lit_float(float val, int line, int col);
ASTNode *ast_create_lit_char(char val, int line, int col);
ASTNode *ast_create_lit_string(const char *val, int line, int col);
ASTNode *ast_create_lit_bool(bool val, int line, int col);

/* Utilitários de Lista */
ASTNode *ast_append_sibling(ASTNode *list, ASTNode *item);

/* Variável global que armazena a raiz da AST gerada */
extern ASTNode *ast_root;

/* Visualização e Gerenciamento de Memória */
void ast_print(const ASTNode *root, int indent);
void ast_free(ASTNode *root);

#endif /* AST_H */
