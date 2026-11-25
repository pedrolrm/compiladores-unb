#include "codegen_loops.h"
#include "codegen_expr.h"
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
 * Emissor de comandos mínimo para os testes: cobre blocos, declarações,
 * atribuições, if, break e continue, e delega os laços ao módulo testado.
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

        case AST_IF:
            emit_indent(out, indent_level);
            fputs("if ", out);
            if (codegen_emit_expression(out, s->left) != 0) return -1;
            fputs(":\n", out);
            return emit_statement(out, s->right, indent_level + 1);

        case AST_BREAK:
            emit_indent(out, indent_level);
            fputs("break\n", out);
            return 0;

        case AST_CONTINUE:
            return codegen_emit_continue(out, indent_level);

        case AST_WHILE:
            return codegen_emit_while(out, s, indent_level, emit_statement);

        case AST_FOR:
            return codegen_emit_for(out, s, indent_level, emit_statement);

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

static void check_loop(ASTNode *loop, int indent_level, const char *expected) {
    FILE *out = tmpfile();
    assert(out != NULL);
    assert(emit_statement(out, loop, indent_level) == 0);
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
    ast_free(loop);
}

/* Construtores auxiliares com os tipos que a análise semântica atribuiria */
static ASTNode *var(const char *name) {
    ASTNode *node = ast_create_identifier(name, 1, 1);
    node->data_type = data_type_primitive(TYPE_INT);
    return node;
}

static ASTNode *lit(int value) {
    return ast_create_lit_int(value, 1, 1);
}

static ASTNode *bin(int op, ASTNode *left, ASTNode *right) {
    return ast_create_bin_op(op, left, right, 1, 1);
}

static ASTNode *assign(int op, const char *name, ASTNode *value) {
    return ast_create_assign(op, var(name), value, 1, 1);
}

static ASTNode *stmt(ASTNode *expression) {
    return ast_create_expr_stmt(expression, 1, 1);
}

static ASTNode *decl(const char *name, ASTNode *init) {
    return ast_create_var_decl(data_type_primitive(TYPE_INT), name, init, 1, 1);
}

static ASTNode *block(ASTNode *statements) {
    return ast_create_block(statements, 1, 1);
}

static ASTNode *list2(ASTNode *first, ASTNode *second) {
    return ast_append_sibling(first, second);
}

static void test_while(void) {
    /* while (n > 0) { soma += n; n -= 1; } */
    check_loop(ast_create_while(bin(OP_GT, var("n"), lit(0)),
                                block(list2(stmt(assign(OP_PLUS_ASSIGN, "soma", var("n"))),
                                            stmt(assign(OP_MINUS_ASSIGN, "n", lit(1))))),
                                1, 1),
               0,
               "while (n > 0):\n"
               "    soma += n\n"
               "    n -= 1\n");

    /* while (x) {} */
    check_loop(ast_create_while(var("x"), block(NULL), 1, 1), 0,
               "while x:\n"
               "    pass\n");

    /* while (true) { break; } dentro de uma função */
    check_loop(ast_create_while(ast_create_lit_bool(true, 1, 1),
                                block(ast_create_break(1, 1)), 1, 1),
               1,
               "    while True:\n"
               "        break\n");
}

static void test_for(void) {
    /* for (int i = 1; i <= 10; i += 1) { total += i; } dentro de uma função */
    check_loop(ast_create_for(decl("i", lit(1)),
                              bin(OP_LE, var("i"), lit(10)),
                              assign(OP_PLUS_ASSIGN, "i", lit(1)),
                              block(stmt(assign(OP_PLUS_ASSIGN, "total", var("i")))),
                              1, 1),
               1,
               "    i = 1\n"
               "    while (i <= 10):\n"
               "        total += i\n"
               "        i += 1\n");

    /* for (;;) { break; } */
    check_loop(ast_create_for(NULL, NULL, NULL, block(ast_create_break(1, 1)), 1, 1), 0,
               "while True:\n"
               "    break\n");

    /* for (;;); */
    check_loop(ast_create_for(NULL, NULL, NULL, NULL, 1, 1), 0,
               "while True:\n"
               "    pass\n");

    /* for (; i < 3;) { i += 1; } */
    check_loop(ast_create_for(NULL, bin(OP_LT, var("i"), lit(3)), NULL,
                              block(stmt(assign(OP_PLUS_ASSIGN, "i", lit(1)))), 1, 1),
               0,
               "while (i < 3):\n"
               "    i += 1\n");

    /* for (i = 0; i < 3; i += 1) {} */
    check_loop(ast_create_for(stmt(assign(OP_ASSIGN, "i", lit(0))),
                              bin(OP_LT, var("i"), lit(3)),
                              assign(OP_PLUS_ASSIGN, "i", lit(1)),
                              block(NULL), 1, 1),
               0,
               "i = 0\n"
               "while (i < 3):\n"
               "    i += 1\n");

    /* for (int i = 0, j = 10; i < j; i += 1) {} */
    check_loop(ast_create_for(list2(decl("i", lit(0)), decl("j", lit(10))),
                              bin(OP_LT, var("i"), var("j")),
                              assign(OP_PLUS_ASSIGN, "i", lit(1)),
                              block(NULL), 1, 1),
               0,
               "i = 0\n"
               "j = 10\n"
               "while (i < j):\n"
               "    i += 1\n");

    /* for (; n > 1; n /= 2) {} : divisão inteira no passo */
    check_loop(ast_create_for(NULL, bin(OP_GT, var("n"), lit(1)),
                              assign(OP_DIV_ASSIGN, "n", lit(2)), block(NULL), 1, 1),
               0,
               "while (n > 1):\n"
               "    n //= 2\n");

    /* for (; i < 3; avanca(i)) {} : passo que não é atribuição */
    check_loop(ast_create_for(NULL, bin(OP_LT, var("i"), lit(3)),
                              ast_create_call("avanca", var("i"), 1, 1), block(NULL), 1, 1),
               0,
               "while (i < 3):\n"
               "    avanca(i)\n");
}

static void test_continue(void) {
    /* for (int i = 0; i < 10; i += 1) { if (i % 2 == 0) continue; soma += i; } */
    check_loop(ast_create_for(decl("i", lit(0)),
                              bin(OP_LT, var("i"), lit(10)),
                              assign(OP_PLUS_ASSIGN, "i", lit(1)),
                              block(list2(ast_create_if(bin(OP_EQ, bin(OP_MOD, var("i"), lit(2)), lit(0)),
                                                        ast_create_continue(1, 1), NULL, 1, 1),
                                          stmt(assign(OP_PLUS_ASSIGN, "soma", var("i"))))),
                              1, 1),
               0,
               "i = 0\n"
               "while (i < 10):\n"
               "    if ((i % 2) == 0):\n"
               "        i += 1\n"
               "        continue\n"
               "    soma += i\n"
               "    i += 1\n");

    /* continue de um while interno não repete o passo do for externo */
    check_loop(ast_create_for(decl("i", lit(0)),
                              bin(OP_LT, var("i"), lit(3)),
                              assign(OP_PLUS_ASSIGN, "i", lit(1)),
                              block(ast_create_while(bin(OP_LT, var("j"), var("i")),
                                                     block(list2(stmt(assign(OP_PLUS_ASSIGN, "j", lit(1))),
                                                                 ast_create_continue(1, 1))),
                                                     1, 1)),
                              1, 1),
               0,
               "i = 0\n"
               "while (i < 3):\n"
               "    while (j < i):\n"
               "        j += 1\n"
               "        continue\n"
               "    i += 1\n");

    /* for aninhados: cada continue repete o passo do seu próprio for */
    check_loop(ast_create_for(decl("i", lit(0)),
                              bin(OP_LT, var("i"), lit(2)),
                              assign(OP_PLUS_ASSIGN, "i", lit(1)),
                              block(ast_create_for(decl("j", lit(0)),
                                                   bin(OP_LT, var("j"), lit(2)),
                                                   assign(OP_PLUS_ASSIGN, "j", lit(1)),
                                                   block(ast_create_continue(1, 1)), 1, 1)),
                              1, 1),
               1,
               "    i = 0\n"
               "    while (i < 2):\n"
               "        j = 0\n"
               "        while (j < 2):\n"
               "            j += 1\n"
               "            continue\n"
               "            j += 1\n"
               "        i += 1\n");
}

static void test_errors(void) {
    FILE *out = tmpfile();
    assert(out != NULL);

    ASTNode *loop = ast_create_while(var("x"), block(ast_create_continue(1, 1)), 1, 1);
    assert(codegen_emit_while(NULL, loop, 0, emit_statement) == -1);
    assert(codegen_emit_while(out, NULL, 0, emit_statement) == -1);
    assert(codegen_emit_while(out, loop, 0, NULL) == -1);
    assert(codegen_emit_while(out, loop, -1, emit_statement) == -1);
    assert(codegen_emit_for(out, loop, 0, emit_statement) == -1);

    /* Falha no corpo é propagada e o laço não fica aberto */
    assert(codegen_emit_while(out, loop, 0, fail_statement) == -1);
    assert(codegen_emit_continue(out, 0) == -1);
    ast_free(loop);

    ASTNode *not_assign = bin(OP_PLUS, var("a"), lit(1));
    assert(codegen_emit_assignment(out, not_assign, 0) == -1);
    ast_free(not_assign);

    fclose(out);
}

int main(void) {
    test_while();
    test_for();
    test_continue();
    test_errors();

    printf("Codegen #85: %d casos de lacos aprovados e erros verificados.\n", cases);
    return 0;
}
