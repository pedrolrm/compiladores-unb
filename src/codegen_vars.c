#include "codegen_vars.h"

int codegen_emit_var_decl(FILE *out, const ASTNode *node, int indent_level,
                          CodegenExpressionEmitter emit_expression) {
    if (!out || !node || node->type != AST_VAR_DECL || !node->name ||
        indent_level < 0) {
        return -1;
    }

    const char *default_value;
    switch (node->data_type.base) {
        case TYPE_INT:   default_value = "0"; break;
        case TYPE_FLOAT: default_value = "0.0"; break;
        case TYPE_CHAR:  default_value = "''"; break;
        case TYPE_BOOL:  default_value = "False"; break;
        default: return -1;
    }

    if (node->data_type.is_array) {
        bool is_char_string_init = (node->data_type.base == TYPE_CHAR &&
                                    node->left &&
                                    node->left->type == AST_LIT_STRING);
        if ((node->data_type.array_size <= 0 && !is_char_string_init) ||
            (node->left && !is_char_string_init)) {
            return -1;
        }
        if (is_char_string_init && !emit_expression) {
            return -1;
        }
    } else if (node->left && !emit_expression) {
        return -1;
    }

    for (int i = 0; i < indent_level; i++) {
        fputs("    ", out);
    }
    fprintf(out, "%s = ", node->name);

    if (node->data_type.is_array) {
        if (node->left) {
            fputs("list(", out);
            if (emit_expression(out, node->left) != 0) {
                return -1;
            }
            fputc(')', out);
        } else {
            fprintf(out, "[%s] * %d", default_value, node->data_type.array_size);
        }
    } else if (node->left) {
        if (emit_expression(out, node->left) != 0) {
            return -1;
        }
    } else {
        fputs(default_value, out);
    }
    fputc('\n', out);

    return ferror(out) ? -1 : 0;
}
