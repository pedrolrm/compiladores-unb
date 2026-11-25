#include "codegen_expr.h"
#include "parser.tab.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

/*
 * Retorna o operador Python equivalente ao operador binário de C,
 * ou NULL quando o operador não é suportado.
 */
static const char *python_binary_operator(int op) {
    switch (op) {
        case OP_PLUS:  return "+";
        case OP_MINUS: return "-";
        case OP_MULT:  return "*";
        case OP_DIV:   return "/";
        case OP_MOD:   return "%";
        case OP_EQ:    return "==";
        case OP_NEQ:   return "!=";
        case OP_LT:    return "<";
        case OP_LE:    return "<=";
        case OP_GT:    return ">";
        case OP_GE:    return ">=";
        case OP_AND:   return "and";
        case OP_OR:    return "or";
        default:       return NULL;
    }
}

static bool is_int_operand(const ASTNode *node) {
    return node->data_type.base == TYPE_INT && !node->data_type.is_array;
}

/*
 * Emite um caractere dentro de um literal Python delimitado por `quote`,
 * escapando barra invertida, o próprio delimitador e caracteres de controle.
 */
static void emit_escaped_char(FILE *out, char c, char quote) {
    unsigned char uc = (unsigned char)c;

    if (c == '\\' || c == quote) {
        fputc('\\', out);
        fputc(c, out);
    } else if (c == '\n') {
        fputs("\\n", out);
    } else if (c == '\t') {
        fputs("\\t", out);
    } else if (uc < 32 || uc == 127) {
        fprintf(out, "\\x%02x", uc);
    } else {
        fputc(c, out);
    }
}

/*
 * Emite o menor texto que representa o float sem perda e garante a presença
 * do ponto decimal, para que o Python não interprete o literal como int.
 */
static int emit_float_literal(FILE *out, float value) {
    if (isinf(value) || isnan(value)) {
        return -1;
    }

    char buffer[32];
    for (int precision = 1; precision <= 9; precision++) {
        snprintf(buffer, sizeof(buffer), "%.*g", precision, value);
        if (strtof(buffer, NULL) == value) {
            break;
        }
    }

    fputs(buffer, out);
    if (!strpbrk(buffer, ".e")) {
        fputs(".0", out);
    }
    return 0;
}

static int emit_call(FILE *out, const ASTNode *call) {
    if (!call->name) {
        return -1;
    }

    fprintf(out, "%s(", call->name);
    for (const ASTNode *arg = call->left; arg; arg = arg->next) {
        if (arg != call->left) {
            fputs(", ", out);
        }
        if (codegen_emit_expression(out, arg) != 0) {
            return -1;
        }
    }
    fputc(')', out);
    return 0;
}

static int emit_unary(FILE *out, const ASTNode *node) {
    const char *op;
    switch (node->op) {
        case OP_MINUS: op = "-"; break;
        case OP_NOT:   op = "not "; break;
        default: return -1;
    }

    fprintf(out, "(%s", op);
    if (codegen_emit_expression(out, node->left) != 0) {
        return -1;
    }
    fputc(')', out);
    return 0;
}

static int emit_binary(FILE *out, const ASTNode *node) {
    const char *op = python_binary_operator(node->op);
    if (!op || !node->left || !node->right) {
        return -1;
    }

    /* int / int em C trunca o resultado; em Python o equivalente é // */
    if (node->op == OP_DIV && is_int_operand(node->left) && is_int_operand(node->right)) {
        op = "//";
    }

    fputc('(', out);
    if (codegen_emit_expression(out, node->left) != 0) {
        return -1;
    }
    fprintf(out, " %s ", op);
    if (codegen_emit_expression(out, node->right) != 0) {
        return -1;
    }
    fputc(')', out);
    return 0;
}

int codegen_emit_expression(FILE *out, const ASTNode *expression) {
    if (!out || !expression) {
        return -1;
    }

    int status = 0;
    switch (expression->type) {
        case AST_LIT_INT:
            fprintf(out, "%d", expression->int_val);
            break;

        case AST_LIT_FLOAT:
            status = emit_float_literal(out, expression->float_val);
            break;

        case AST_LIT_CHAR:
            fputc('\'', out);
            emit_escaped_char(out, expression->char_val, '\'');
            fputc('\'', out);
            break;

        case AST_LIT_STRING:
            fputc('"', out);
            if (expression->str_val) {
                for (const char *c = expression->str_val; *c; c++) {
                    emit_escaped_char(out, *c, '"');
                }
            }
            fputc('"', out);
            break;

        case AST_LIT_BOOL:
            fputs(expression->int_val ? "True" : "False", out);
            break;

        case AST_IDENTIFIER:
            if (!expression->name) {
                return -1;
            }
            fputs(expression->name, out);
            break;

        case AST_ARRAY_ACCESS:
            if (!expression->right || codegen_emit_expression(out, expression->left) != 0) {
                return -1;
            }
            fputc('[', out);
            status = codegen_emit_expression(out, expression->right);
            fputc(']', out);
            break;

        case AST_CALL:
            status = emit_call(out, expression);
            break;

        case AST_UN_OP:
            status = emit_unary(out, expression);
            break;

        case AST_BIN_OP:
            status = emit_binary(out, expression);
            break;

        default:
            return -1;
    }

    if (status != 0) {
        return -1;
    }
    return ferror(out) ? -1 : 0;
}
