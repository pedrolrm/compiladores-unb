#include "codegen_expr.h"
#include "parser.tab.h"
#include <assert.h>
#include <string.h>

static int cases = 0;

static void check_expression(const ASTNode *expression, const char *expected) {
    FILE *out = tmpfile();
    assert(out != NULL);
    assert(codegen_emit_expression(out, expression) == 0);
    rewind(out);

    char actual[256];
    size_t length = fread(actual, 1, sizeof(actual) - 1, out);
    assert(!ferror(out));
    actual[length] = '\0';
    fclose(out);

    if (strcmp(actual, expected) != 0) {
        fprintf(stderr, "Esperado: %s\nObtido:   %s\n", expected, actual);
        exit(1);
    }
    cases++;
}

/* Identificador com o tipo que a análise semântica atribuiria. */
static ASTNode *var(const char *name, BaseType base) {
    ASTNode *node = ast_create_identifier(name, 1, 1);
    node->data_type = data_type_primitive(base);
    return node;
}

static ASTNode *lit_int(int value) {
    return ast_create_lit_int(value, 1, 1);
}

static ASTNode *bin(int op, ASTNode *left, ASTNode *right) {
    return ast_create_bin_op(op, left, right, 1, 1);
}

static void test_literals(void) {
    ASTNode *nodes[] = {
        lit_int(42),
        ast_create_lit_float(2.5f, 1, 1),
        ast_create_lit_float(3.0f, 1, 1),
        ast_create_lit_float(0.1f, 1, 1),
        ast_create_lit_float(0.0f, 1, 1),
        ast_create_lit_char('a', 1, 1),
        ast_create_lit_char('\'', 1, 1),
        ast_create_lit_char('\n', 1, 1),
        ast_create_lit_char('\0', 1, 1),
        ast_create_lit_string("diz \"oi\"\n", 1, 1),
        ast_create_lit_string("C:\\temp", 1, 1),
        ast_create_lit_bool(true, 1, 1),
        ast_create_lit_bool(false, 1, 1),
    };
    const char *expected[] = {
        "42",
        "2.5",
        "3.0",
        "0.1",
        "0.0",
        "'a'",
        "'\\''",
        "'\\n'",
        "'\\x00'",
        "\"diz \\\"oi\\\"\\n\"",
        "\"C:\\\\temp\"",
        "True",
        "False",
    };

    for (size_t i = 0; i < sizeof(nodes) / sizeof(nodes[0]); i++) {
        check_expression(nodes[i], expected[i]);
        ast_free(nodes[i]);
    }
}

static void test_arithmetic(void) {
    /* a + b * c: a AST já agrupa b * c, e os parênteses tornam isso explícito */
    ASTNode *expr = bin(OP_PLUS, var("a", TYPE_INT),
                        bin(OP_MULT, var("b", TYPE_INT), var("c", TYPE_INT)));
    check_expression(expr, "(a + (b * c))");
    ast_free(expr);

    /* (a + b) * c: o agrupamento do código C é preservado */
    expr = bin(OP_MULT, bin(OP_PLUS, var("a", TYPE_INT), var("b", TYPE_INT)),
               var("c", TYPE_INT));
    check_expression(expr, "((a + b) * c)");
    ast_free(expr);

    expr = bin(OP_MINUS, var("a", TYPE_INT), var("b", TYPE_INT));
    check_expression(expr, "(a - b)");
    ast_free(expr);

    expr = bin(OP_MOD, var("a", TYPE_INT), lit_int(2));
    check_expression(expr, "(a % 2)");
    ast_free(expr);
}

static void test_division(void) {
    ASTNode *expr = bin(OP_DIV, var("a", TYPE_INT), var("b", TYPE_INT));
    check_expression(expr, "(a // b)");
    ast_free(expr);

    expr = bin(OP_DIV, lit_int(7), lit_int(2));
    check_expression(expr, "(7 // 2)");
    ast_free(expr);

    expr = bin(OP_DIV, var("x", TYPE_FLOAT), var("a", TYPE_INT));
    check_expression(expr, "(x / a)");
    ast_free(expr);

    expr = bin(OP_DIV, var("a", TYPE_INT), ast_create_lit_float(2.0f, 1, 1));
    check_expression(expr, "(a / 2.0)");
    ast_free(expr);

    /* O resultado de int / int também é int: (a / b) / c continua inteiro */
    ASTNode *inner = bin(OP_DIV, var("a", TYPE_INT), var("b", TYPE_INT));
    inner->data_type = data_type_primitive(TYPE_INT);
    expr = bin(OP_DIV, inner, var("c", TYPE_INT));
    check_expression(expr, "((a // b) // c)");
    ast_free(expr);
}

static void test_relational(void) {
    const int ops[] = { OP_EQ, OP_NEQ, OP_LT, OP_LE, OP_GT, OP_GE };
    const char *expected[] = {
        "(a == b)", "(a != b)", "(a < b)", "(a <= b)", "(a > b)", "(a >= b)",
    };

    for (size_t i = 0; i < sizeof(ops) / sizeof(ops[0]); i++) {
        ASTNode *expr = bin(ops[i], var("a", TYPE_INT), var("b", TYPE_INT));
        check_expression(expr, expected[i]);
        ast_free(expr);
    }
}

static void test_logical_and_unary(void) {
    ASTNode *expr = bin(OP_AND, var("p", TYPE_BOOL), var("q", TYPE_BOOL));
    check_expression(expr, "(p and q)");
    ast_free(expr);

    expr = bin(OP_OR, var("p", TYPE_BOOL), var("q", TYPE_BOOL));
    check_expression(expr, "(p or q)");
    ast_free(expr);

    expr = ast_create_un_op(OP_NOT, var("p", TYPE_BOOL), 1, 1);
    check_expression(expr, "(not p)");
    ast_free(expr);

    expr = ast_create_un_op(OP_MINUS, var("x", TYPE_INT), 1, 1);
    check_expression(expr, "(-x)");
    ast_free(expr);

    /* (x < y && y != 0) || !(x == 5) */
    expr = bin(OP_OR,
               bin(OP_AND,
                   bin(OP_LT, var("x", TYPE_INT), var("y", TYPE_INT)),
                   bin(OP_NEQ, var("y", TYPE_INT), lit_int(0))),
               ast_create_un_op(OP_NOT, bin(OP_EQ, var("x", TYPE_INT), lit_int(5)), 1, 1));
    check_expression(expr, "(((x < y) and (y != 0)) or (not (x == 5)))");
    ast_free(expr);
}

static void test_array_and_calls(void) {
    ASTNode *expr = ast_create_array_access(
        var("v", TYPE_INT), bin(OP_PLUS, var("i", TYPE_INT), lit_int(1)), 1, 1);
    check_expression(expr, "v[(i + 1)]");
    ast_free(expr);

    expr = ast_create_call("f", NULL, 1, 1);
    check_expression(expr, "f()");
    ast_free(expr);

    ASTNode *args = var("a", TYPE_INT);
    ast_append_sibling(args, lit_int(2));
    ast_append_sibling(args, ast_create_call("g", var("b", TYPE_INT), 1, 1));
    expr = ast_create_call("soma", args, 1, 1);
    check_expression(expr, "soma(a, 2, g(b))");
    ast_free(expr);

    /* n * fatorial(n - 1) */
    expr = bin(OP_MULT, var("n", TYPE_INT),
               ast_create_call("fatorial", bin(OP_MINUS, var("n", TYPE_INT), lit_int(1)), 1, 1));
    check_expression(expr, "(n * fatorial((n - 1)))");
    ast_free(expr);
}

static void test_errors(void) {
    FILE *out = tmpfile();
    assert(out != NULL);

    ASTNode *expr = lit_int(1);
    assert(codegen_emit_expression(NULL, expr) == -1);
    assert(codegen_emit_expression(out, NULL) == -1);
    ast_free(expr);

    /* Atribuição não é expressão em Python e fica fora deste emissor */
    expr = ast_create_assign(OP_ASSIGN, var("x", TYPE_INT), lit_int(1), 1, 1);
    assert(codegen_emit_expression(out, expr) == -1);
    ast_free(expr);

    expr = bin(OP_PLUS, var("a", TYPE_INT), ast_create_return(NULL, 1, 1));
    assert(codegen_emit_expression(out, expr) == -1);
    ast_free(expr);

    fclose(out);
}

int main(void) {
    test_literals();
    test_arithmetic();
    test_division();
    test_relational();
    test_logical_and_unary();
    test_array_and_calls();
    test_errors();

    printf("Codegen #86: %d casos de expressoes aprovados e erros verificados.\n", cases);
    return 0;
}
