#include "codegen_func.h"
#include "codegen_expr.h"
#include "codegen_loops.h"
#include "parser.tab.h"
#include <assert.h>
#include <string.h>

static int cases = 0;

static void emit_indent(FILE *out, int indent_level) {
    for (int i = 0; i < indent_level; i++) {
        fputs("    ", out);
    }
}

/*
 * Emissor de comandos mínimo para os testes: cobre blocos, atribuições,
 * return, while e as definições de função aninhadas pelo módulo testado.
 */
static int emit_statement(FILE *out, const ASTNode *s, int indent_level) {
    switch (s->type) {
        case AST_BLOCK:
            for (const ASTNode *child = s->left; child; child = child->next) {
                if (emit_statement(out, child, indent_level) != 0) {
                    return -1;
                }
            }
            return 0;

        case AST_VAR_DECL:
            emit_indent(out, indent_level);
            fprintf(out, "%s = ", s->name);
            if (s->left) {
                if (codegen_emit_expression(out, s->left) != 0) return -1;
            } else {
                fputc('0', out);
            }
            fputc('\n', out);
            return 0;

        case AST_EXPR_STMT:
            if (s->left->type == AST_ASSIGN) {
                return codegen_emit_assignment(out, s->left, indent_level);
            }
            emit_indent(out, indent_level);
            if (codegen_emit_expression(out, s->left) != 0) return -1;
            fputc('\n', out);
            return 0;

        case AST_RETURN:
            emit_indent(out, indent_level);
            fputs("return", out);
            if (s->left) {
                fputc(' ', out);
                if (codegen_emit_expression(out, s->left) != 0) return -1;
            }
            fputc('\n', out);
            return 0;

        case AST_WHILE:
            return codegen_emit_while(out, s, indent_level, emit_statement);

        case AST_FUNC_DEF:
            return codegen_emit_function(out, s, indent_level, emit_statement);

        default:
            return -1;
    }
}

static int fail_statement(FILE *out, const ASTNode *s, int indent_level) {
    (void)out;
    (void)s;
    (void)indent_level;
    return -1;
}

static void check_function(ASTNode *function, int indent_level, const char *expected) {
    FILE *out = tmpfile();
    assert(out != NULL);
    assert(codegen_emit_function(out, function, indent_level, emit_statement) == 0);
    rewind(out);

    char actual[512];
    size_t length = fread(actual, 1, sizeof(actual) - 1, out);
    assert(!ferror(out));
    actual[length] = '\0';
    fclose(out);

    if (strcmp(actual, expected) != 0) {
        fprintf(stderr, "Esperado:\n%s\nObtido:\n%s\n", expected, actual);
        exit(1);
    }
    cases++;
    ast_free(function);
}

/* Construtores auxiliares com os tipos que a análise semântica atribuiria */
static DataType int_type(void) {
    return data_type_primitive(TYPE_INT);
}

static ASTNode *var(const char *name) {
    ASTNode *node = ast_create_identifier(name, 1, 1);
    node->data_type = int_type();
    return node;
}

static ASTNode *lit(int value) {
    return ast_create_lit_int(value, 1, 1);
}

static ASTNode *bin(int op, ASTNode *left, ASTNode *right) {
    return ast_create_bin_op(op, left, right, 1, 1);
}

static ASTNode *block(ASTNode *statements) {
    return ast_create_block(statements, 1, 1);
}

static ASTNode *param(const char *name) {
    return ast_create_param(int_type(), name, 1, 1);
}

static ASTNode *func(const char *name, ASTNode *params, ASTNode *body) {
    return ast_create_func_def(int_type(), name, params, body, 1, 1);
}

static void test_sem_parametros(void) {
    /* int zero(void) { return 0; } */
    check_function(func("zero", NULL, block(ast_create_return(lit(0), 1, 1))), 0,
                   "def zero():\n"
                   "    return 0\n");
}

static void test_multiplos_parametros(void) {
    /* int soma(int a, int b, int c) { return a + b + c; } */
    ASTNode *params = ast_append_sibling(ast_append_sibling(param("a"), param("b")), param("c"));
    check_function(func("soma", params,
                        block(ast_create_return(bin(OP_PLUS, bin(OP_PLUS, var("a"), var("b")),
                                                    var("c")), 1, 1))), 0,
                   "def soma(a, b, c):\n"
                   "    return ((a + b) + c)\n");
}

static void test_corpo_vazio(void) {
    /* void nada(int a) { } */
    check_function(func("nada", param("a"), block(NULL)), 0,
                   "def nada(a):\n"
                   "    pass\n");

    /* Corpo ausente na AST também gera pass */
    check_function(func("nada", NULL, NULL), 0,
                   "def nada():\n"
                   "    pass\n");
}

static void test_indentacao_do_corpo(void) {
    /* Comandos internos sobem um nível a partir do nível da definição */
    check_function(func("acumula", param("n"),
                        block(ast_create_while(bin(OP_GT, var("n"), lit(0)),
                                               block(ast_create_expr_stmt(
                                                   ast_create_assign(OP_MINUS_ASSIGN, var("n"),
                                                                     lit(1), 1, 1), 1, 1)),
                                               1, 1))), 1,
                   "    def acumula(n):\n"
                   "        while (n > 0):\n"
                   "            n -= 1\n");
}

static void test_errors(void) {
    FILE *out = tmpfile();
    assert(out != NULL);

    ASTNode *function = func("f", param("a"), block(ast_create_return(lit(1), 1, 1)));
    assert(codegen_emit_function(NULL, function, 0, emit_statement) == -1);
    assert(codegen_emit_function(out, NULL, 0, emit_statement) == -1);
    assert(codegen_emit_function(out, function, 0, NULL) == -1);
    assert(codegen_emit_function(out, function, -1, emit_statement) == -1);

    /* Falha no corpo é propagada */
    assert(codegen_emit_function(out, function, 0, fail_statement) == -1);
    ast_free(function);

    /* Nó que não é definição de função */
    ASTNode *not_function = block(NULL);
    assert(codegen_emit_function(out, not_function, 0, emit_statement) == -1);
    ast_free(not_function);

    /* Função sem nome e parâmetro sem nome são rejeitados */
    ASTNode *unnamed = func(NULL, NULL, block(NULL));
    assert(codegen_emit_function(out, unnamed, 0, emit_statement) == -1);
    ast_free(unnamed);

    ASTNode *bad_param = func("f", ast_create_param(int_type(), NULL, 1, 1), block(NULL));
    assert(codegen_emit_function(out, bad_param, 0, emit_statement) == -1);
    ast_free(bad_param);

    fclose(out);
}

int main(void) {
    test_sem_parametros();
    test_multiplos_parametros();
    test_corpo_vazio();
    test_indentacao_do_corpo();
    test_errors();

    printf("Codegen #81: %d casos de definicoes de funcao aprovados e erros verificados.\n", cases);
    return 0;
}
