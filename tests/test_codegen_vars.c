#include "codegen_vars.h"
#include <assert.h>
#include <string.h>

static const ASTNode *expected_expression;
static const char *expression_text;

static int emit_expression(FILE *out, const ASTNode *expression) {
    assert(expression == expected_expression);
    return fputs(expression_text, out) == EOF ? -1 : 0;
}

static int fail_expression(FILE *out, const ASTNode *expression) {
    (void)out;
    (void)expression;
    return -1;
}

static void check_declaration(ASTNode *node, int indent_level,
                              CodegenExpressionEmitter emitter,
                              const char *expected) {
    FILE *out = tmpfile();
    assert(out != NULL);
    assert(codegen_emit_var_decl(out, node, indent_level, emitter) == 0);
    rewind(out);

    char actual[256];
    size_t length = fread(actual, 1, sizeof(actual) - 1, out);
    assert(!ferror(out));
    actual[length] = '\0';
    assert(strcmp(actual, expected) == 0);
    fclose(out);
}

int main(void) {
    const BaseType types[] = { TYPE_INT, TYPE_FLOAT, TYPE_CHAR, TYPE_BOOL };
    const char *defaults[] = { "0", "0.0", "''", "False" };
    const char *initializers[] = { "(base + 2)", "1.5", "'a'", "True" };
    ASTNode expression = {0};
    expected_expression = &expression;

    ASTNode declaration = {0};
    declaration.type = AST_VAR_DECL;
    declaration.name = "value";

    /* A mesma declaração serve para o escopo global e o corpo de funções. */
    for (int indent_level = 0; indent_level <= 2; indent_level++) {
        char indent[9] = "";
        for (int i = 0; i < indent_level; i++) {
            strcat(indent, "    ");
        }
        for (size_t i = 0; i < sizeof(types) / sizeof(types[0]); i++) {
            char expected[128];
            declaration.data_type.base = types[i];
            declaration.data_type.is_array = false;
            declaration.left = NULL;
            snprintf(expected, sizeof(expected), "%svalue = %s\n", indent, defaults[i]);
            check_declaration(&declaration, indent_level, NULL, expected);

            declaration.left = &expression;
            expression_text = initializers[i];
            snprintf(expected, sizeof(expected), "%svalue = %s\n", indent, initializers[i]);
            check_declaration(&declaration, indent_level, emit_expression, expected);

            declaration.left = NULL;
            declaration.data_type.is_array = true;
            declaration.data_type.array_size = 5;
            snprintf(expected, sizeof(expected), "%svalue = [%s] * 5\n", indent, defaults[i]);
            check_declaration(&declaration, indent_level, NULL, expected);
        }
    }

    FILE *out = tmpfile();
    assert(out != NULL);
    declaration.data_type.array_size = 0;
    assert(codegen_emit_var_decl(out, &declaration, 0, NULL) == -1);
    declaration.data_type.array_size = -1;
    assert(codegen_emit_var_decl(out, &declaration, 0, NULL) == -1);
    declaration.data_type.array_size = 5;
    declaration.left = &expression;
    assert(codegen_emit_var_decl(out, &declaration, 0, emit_expression) == -1);
    declaration.data_type.is_array = false;
    assert(codegen_emit_var_decl(out, &declaration, 0, NULL) == -1);
    assert(codegen_emit_var_decl(out, &declaration, 0, fail_expression) == -1);
    declaration.left = NULL;
    declaration.data_type.base = TYPE_VOID;
    assert(codegen_emit_var_decl(out, &declaration, 0, NULL) == -1);
    declaration.data_type.base = TYPE_INT;
    assert(codegen_emit_var_decl(out, &declaration, -1, NULL) == -1);
    assert(codegen_emit_var_decl(NULL, &declaration, 0, NULL) == -1);
    assert(codegen_emit_var_decl(out, NULL, 0, NULL) == -1);
    declaration.type = AST_IDENTIFIER;
    assert(codegen_emit_var_decl(out, &declaration, 0, NULL) == -1);
    fclose(out);

    /* Vetor char[] com inicializador string literal (char s[] = "oi") */
    ASTNode str_init = {0};
    str_init.type = AST_LIT_STRING;
    str_init.str_val = "oi";
    expected_expression = &str_init;
    expression_text = "\"oi\"";
    declaration.type = AST_VAR_DECL;
    declaration.name = "msg";
    declaration.data_type.base = TYPE_CHAR;
    declaration.data_type.is_array = true;
    declaration.data_type.array_size = 3;
    declaration.left = &str_init;
    check_declaration(&declaration, 0, emit_expression, "msg = list(\"oi\")\n");

    puts("Codegen #82: 37 casos de declaracoes aprovados e erros verificados.");
    return 0;
}
