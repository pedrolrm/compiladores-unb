#include "codegen_func.h"

static void emit_indent(FILE *out, int indent_level) {
    for (int i = 0; i < indent_level; i++) {
        fputs("    ", out);
    }
}

/* Um bloco sem comandos não produz linha nenhuma e exige `pass` no corpo. */
static bool has_statements(const ASTNode *body) {
    return body && !(body->type == AST_BLOCK && !body->left);
}

/*
 * Emite os nomes dos parâmetros separados por vírgula, na ordem declarada.
 * Vetores são passados por nome, como qualquer outro parâmetro em Python.
 */
static int emit_parameters(FILE *out, const ASTNode *parameters) {
    const char *separator = "";
    for (const ASTNode *param = parameters; param; param = param->next) {
        if (param->type != AST_PARAM_DECL || !param->name) {
            return -1;
        }
        fprintf(out, "%s%s", separator, param->name);
        separator = ", ";
    }
    return 0;
}

int codegen_emit_function(FILE *out, const ASTNode *function, int indent_level,
                          CodegenFunctionStatementEmitter emit_statement) {
    if (!out || !function || function->type != AST_FUNC_DEF || !function->name ||
        !emit_statement || indent_level < 0) {
        return -1;
    }

    emit_indent(out, indent_level);
    fprintf(out, "def %s(", function->name);
    if (emit_parameters(out, function->left) != 0) {
        return -1;
    }
    fputs("):\n", out);

    if (has_statements(function->right)) {
        if (emit_statement(out, function->right, indent_level + 1) != 0) {
            return -1;
        }
    } else {
        emit_indent(out, indent_level + 1);
        fputs("pass\n", out);
    }

    return ferror(out) ? -1 : 0;
}
