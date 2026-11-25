#include "codegen_control.h"

static void emit_indent(FILE *out, int indent_level) {
    for (int i = 0; i < indent_level; i++) fputs("    ", out);
}

static int emit_branch(FILE *out, const ASTNode *node, int indent_level,
                       CodegenControlExpressionEmitter emit_expression,
                       CodegenControlStatementEmitter emit_statement);

static int emit_statement_list(FILE *out, const ASTNode *node, int indent_level,
                               CodegenControlExpressionEmitter emit_expression,
                               CodegenControlStatementEmitter emit_statement) {
    if (!node) {
        emit_indent(out, indent_level);
        fputs("pass\n", out);
        return ferror(out) ? -1 : 0;
    }
    for (const ASTNode *curr = node; curr; curr = curr->next) {
        if (emit_branch(out, curr, indent_level, emit_expression,
                        emit_statement) != 0) {
            return -1;
        }
    }
    return 0;
}

static int emit_one(FILE *out, const ASTNode *node, int indent_level,
                    CodegenControlExpressionEmitter emit_expression,
                    CodegenControlStatementEmitter emit_statement) {
    if (!node || indent_level < 0) return -1;

    switch (node->type) {
        case AST_BLOCK:
            return emit_statement_list(out, node->left, indent_level,
                                       emit_expression, emit_statement);

        case AST_IF:
            if (!node->left || !node->right || !emit_expression) return -1;
            emit_indent(out, indent_level);
            fputs("if ", out);
            if (emit_expression(out, node->left) != 0) return -1;
            fputs(":\n", out);
            if (emit_branch(out, node->right, indent_level + 1,
                            emit_expression, emit_statement) != 0) return -1;
            if (node->extra) {
                emit_indent(out, indent_level);
                fputs("else:\n", out);
                if (emit_branch(out, node->extra, indent_level + 1,
                                emit_expression, emit_statement) != 0) return -1;
            }
            return ferror(out) ? -1 : 0;

        case AST_BREAK:
            emit_indent(out, indent_level);
            fputs("break\n", out);
            return ferror(out) ? -1 : 0;

        case AST_CONTINUE:
            if (emit_statement) {
                return emit_statement(out, node, indent_level);
            }
            emit_indent(out, indent_level);
            fputs("continue\n", out);
            return ferror(out) ? -1 : 0;

        case AST_RETURN:
            emit_indent(out, indent_level);
            fputs("return", out);
            if (node->left) {
                if (!emit_expression) return -1;
                fputc(' ', out);
                if (emit_expression(out, node->left) != 0) return -1;
            }
            fputc('\n', out);
            return ferror(out) ? -1 : 0;

        default:
            return -1;
    }
}

static int emit_branch(FILE *out, const ASTNode *node, int indent_level,
                       CodegenControlExpressionEmitter emit_expression,
                       CodegenControlStatementEmitter emit_statement) {
    if (!node || indent_level < 0) return -1;
    switch (node->type) {
        case AST_BLOCK:
        case AST_IF:
        case AST_BREAK:
        case AST_CONTINUE:
        case AST_RETURN:
            return emit_one(out, node, indent_level, emit_expression,
                            emit_statement);
        default:
            return emit_statement ? emit_statement(out, node, indent_level) : -1;
    }
}

int codegen_emit_control(FILE *out, const ASTNode *node, int indent_level,
                         CodegenControlExpressionEmitter emit_expression,
                         CodegenControlStatementEmitter emit_statement) {
    if (!out || !node || indent_level < 0) return -1;
    return emit_one(out, node, indent_level, emit_expression, emit_statement);
}
