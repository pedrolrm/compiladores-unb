#include "codegen_loops.h"
#include "codegen_expr.h"
#include "parser.tab.h"

#define CODEGEN_MAX_LOOP_DEPTH 64

/*
 * Passo de cada laço aberto, do mais externo ao mais interno.
 * while e for sem passo empilham NULL, para que um continue dentro deles
 * não repita o passo de um for mais externo.
 */
static const ASTNode *loop_steps[CODEGEN_MAX_LOOP_DEPTH];
static int loop_depth = 0;

static void emit_indent(FILE *out, int indent_level) {
    for (int i = 0; i < indent_level; i++) {
        fputs("    ", out);
    }
}

static bool is_int_operand(const ASTNode *node) {
    return node->data_type.base == TYPE_INT && !node->data_type.is_array;
}

/*
 * No AST_FOR, node->extra guarda o passo (com o corpo em passo->next) ou,
 * quando não há passo, o próprio corpo. O passo é sempre uma expressão e o
 * corpo é sempre um comando, então o tipo do nó separa os dois casos.
 */
static bool is_statement(const ASTNode *node) {
    switch (node->type) {
        case AST_VAR_DECL:
        case AST_FUNC_DEF:
        case AST_BLOCK:
        case AST_EXPR_STMT:
        case AST_IF:
        case AST_WHILE:
        case AST_FOR:
        case AST_BREAK:
        case AST_CONTINUE:
        case AST_RETURN:
            return true;
        default:
            return false;
    }
}

static bool has_statements(const ASTNode *body) {
    return body && !(body->type == AST_BLOCK && !body->left);
}

/* Passo do for: atribuição (i += 1) ou expressão qualquer (f(i)). */
static int emit_step(FILE *out, const ASTNode *step, int indent_level) {
    if (step->type == AST_ASSIGN) {
        return codegen_emit_assignment(out, step, indent_level);
    }

    emit_indent(out, indent_level);
    if (codegen_emit_expression(out, step) != 0) {
        return -1;
    }
    fputc('\n', out);
    return 0;
}

/*
 * Emite `while cond:`, o corpo e, quando houver, o passo ao final do corpo.
 */
static int emit_loop(FILE *out, const ASTNode *cond, const ASTNode *body,
                     const ASTNode *step, int indent_level,
                     CodegenStatementEmitter emit_statement) {
    if (loop_depth >= CODEGEN_MAX_LOOP_DEPTH) {
        return -1;
    }

    emit_indent(out, indent_level);
    fputs("while ", out);
    if (cond) {
        if (codegen_emit_expression(out, cond) != 0) {
            return -1;
        }
    } else {
        fputs("True", out);
    }
    fputs(":\n", out);

    int status = 0;
    loop_steps[loop_depth++] = step;
    if (has_statements(body)) {
        status = emit_statement(out, body, indent_level + 1);
    }
    loop_depth--;
    if (status != 0) {
        return -1;
    }

    if (step) {
        if (emit_step(out, step, indent_level + 1) != 0) {
            return -1;
        }
    } else if (!has_statements(body)) {
        emit_indent(out, indent_level + 1);
        fputs("pass\n", out);
    }

    return ferror(out) ? -1 : 0;
}

int codegen_emit_while(FILE *out, const ASTNode *node, int indent_level,
                       CodegenStatementEmitter emit_statement) {
    if (!out || !node || node->type != AST_WHILE || !node->left ||
        !emit_statement || indent_level < 0) {
        return -1;
    }

    return emit_loop(out, node->left, node->right, NULL, indent_level, emit_statement);
}

int codegen_emit_for(FILE *out, const ASTNode *node, int indent_level,
                     CodegenStatementEmitter emit_statement) {
    if (!out || !node || node->type != AST_FOR || !emit_statement || indent_level < 0) {
        return -1;
    }

    /* Inicialização: uma declaração (int i = 0, j = 1;) ou uma expressão (i = 0;) */
    for (const ASTNode *init = node->left; init; init = init->next) {
        if (emit_statement(out, init, indent_level) != 0) {
            return -1;
        }
    }

    const ASTNode *step = NULL;
    const ASTNode *body = node->extra;
    if (body && !is_statement(body)) {
        step = body;
        body = step->next;
    }

    return emit_loop(out, node->right, body, step, indent_level, emit_statement);
}

int codegen_emit_continue(FILE *out, int indent_level) {
    if (!out || loop_depth == 0 || indent_level < 0) {
        return -1;
    }

    const ASTNode *step = loop_steps[loop_depth - 1];
    if (step && emit_step(out, step, indent_level) != 0) {
        return -1;
    }

    emit_indent(out, indent_level);
    fputs("continue\n", out);
    return ferror(out) ? -1 : 0;
}

int codegen_emit_assignment(FILE *out, const ASTNode *assignment, int indent_level) {
    if (!out || !assignment || assignment->type != AST_ASSIGN ||
        !assignment->left || !assignment->right || indent_level < 0) {
        return -1;
    }

    const char *op;
    switch (assignment->op) {
        case OP_ASSIGN:       op = "="; break;
        case OP_PLUS_ASSIGN:  op = "+="; break;
        case OP_MINUS_ASSIGN: op = "-="; break;
        case OP_MULT_ASSIGN:  op = "*="; break;
        case OP_DIV_ASSIGN:
            op = is_int_operand(assignment->left) && is_int_operand(assignment->right) ? "//=" : "/=";
            break;
        default:
            return -1;
    }

    emit_indent(out, indent_level);
    if (codegen_emit_expression(out, assignment->left) != 0) {
        return -1;
    }
    fprintf(out, " %s ", op);
    if (codegen_emit_expression(out, assignment->right) != 0) {
        return -1;
    }
    fputc('\n', out);
    return ferror(out) ? -1 : 0;
}
