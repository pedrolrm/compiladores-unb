#include "codegen.h"
#include "codegen_control.h"
#include "codegen_expr.h"
#include "codegen_func.h"
#include "codegen_globals.h"
#include "codegen_loops.h"
#include "codegen_vars.h"
#include <stdarg.h>

static const ASTNode *g_program_declarations = NULL;
static const ASTNode *g_pending_globals_func = NULL;

static int emit_statement(FILE *out, const ASTNode *stmt, int indent_level);

/*
 * Emite um comando AST_EXPR_STMT (atribuição, chamada ou expressão avulsa).
 */
static int emit_expr_statement(FILE *out, const ASTNode *stmt, int indent_level) {
    if (!stmt->left) {
        for (int i = 0; i < indent_level; i++) {
            fputs("    ", out);
        }
        fputs("pass\n", out);
        return ferror(out) ? -1 : 0;
    }

    if (stmt->left->type == AST_ASSIGN) {
        return codegen_emit_assignment(out, stmt->left, indent_level);
    }

    for (int i = 0; i < indent_level; i++) {
        fputs("    ", out);
    }
    if (codegen_emit_expression(out, stmt->left) != 0) {
        return -1;
    }
    fputc('\n', out);
    return ferror(out) ? -1 : 0;
}

/*
 * Despachante unificado de comandos para os emissores de geração de código.
 */
static int emit_statement(FILE *out, const ASTNode *stmt, int indent_level) {
    if (!out || !stmt || indent_level < 0) {
        return -1;
    }

    if (g_pending_globals_func != NULL) {
        const ASTNode *func = g_pending_globals_func;
        g_pending_globals_func = NULL;
        if (codegen_emit_function_globals(out, func, g_program_declarations,
                                          indent_level) != 0) {
            return -1;
        }
    }

    switch (stmt->type) {
        case AST_VAR_DECL:
            return codegen_emit_var_decl(out, stmt, indent_level,
                                         codegen_emit_expression);
        case AST_BLOCK:
        case AST_IF:
        case AST_BREAK:
        case AST_RETURN:
            return codegen_emit_control(out, stmt, indent_level,
                                        codegen_emit_expression, emit_statement);
        case AST_CONTINUE:
            return codegen_emit_continue(out, indent_level);
        case AST_WHILE:
            return codegen_emit_while(out, stmt, indent_level, emit_statement);
        case AST_FOR:
            return codegen_emit_for(out, stmt, indent_level, emit_statement);
        case AST_ASSIGN:
            return codegen_emit_assignment(out, stmt, indent_level);
        case AST_EXPR_STMT:
            return emit_expr_statement(out, stmt, indent_level);
        default:
            return -1;
    }
}

/*
 * Inicializa o contexto de geração de código com valores padrão seguros.
 */
void codegen_context_init(CodegenContext *ctx, FILE *out, Scope *global_scope) {
    if (!ctx) return;
    ctx->out = out;
    ctx->indent_level = 0;
    ctx->global_scope = global_scope;
    ctx->in_function = false;
    ctx->errors_count = 0;
}

/*
 * Emite 4 espaços multiplicados por ctx->indent_level (padrão PEP 8).
 */
void codegen_indent(CodegenContext *ctx) {
    if (!ctx || !ctx->out || ctx->indent_level <= 0) return;

    for (int i = 0; i < ctx->indent_level; i++) {
        for (int s = 0; s < CODEGEN_INDENT_SPACES; s++) {
            fputc(' ', ctx->out);
        }
    }
}

/*
 * Emite texto formatado diretamente no arquivo de saída sem quebra de linha.
 */
void codegen_emit(CodegenContext *ctx, const char *fmt, ...) {
    if (!ctx || !ctx->out || !fmt) return;

    va_list args;
    va_start(args, fmt);
    vfprintf(ctx->out, fmt, args);
    va_end(args);
}

/*
 * Emite texto indentado seguido de quebra de linha.
 * Linhas vazias não recebem espaços de indentação (PEP 8).
 */
void codegen_emit_line(CodegenContext *ctx, const char *fmt, ...) {
    if (!ctx || !ctx->out) return;

    if (!fmt || fmt[0] == '\0') {
        fputc('\n', ctx->out);
        return;
    }

    codegen_indent(ctx);

    va_list args;
    va_start(args, fmt);
    vfprintf(ctx->out, fmt, args);
    va_end(args);

    fputc('\n', ctx->out);
}

/*
 * Percorre as declarações de topo da AST e registra variáveis globais
 * no escopo global do contexto de geração de código.
 */
static void collect_global_symbols(CodegenContext *ctx, ASTNode *declarations) {
    ASTNode *curr = declarations;
    while (curr) {
        if (curr->type == AST_VAR_DECL && curr->name && ctx->global_scope) {
            if (!symtab_lookup_local(ctx->global_scope, curr->name)) {
                symtab_insert_var(ctx->global_scope, curr->name, curr->data_type,
                                  curr->line, curr->column);
            }
        }
        curr = curr->next;
    }
}

/*
 * Função principal de geração de código Python 3 a partir da AST.
 */
int codegen_generate(ASTNode *root, FILE *output) {
    if (!root || !output) {
        return -1;
    }

    Scope *global_scope = symtab_create_global();
    if (!global_scope) {
        return -1;
    }

    CodegenContext ctx;
    codegen_context_init(&ctx, output, global_scope);

    ASTNode *declarations = (root->type == AST_PROGRAM) ? root->left : root;
    collect_global_symbols(&ctx, declarations);

    g_program_declarations = declarations;
    bool emitted_any = false;

    for (ASTNode *curr = declarations; curr; curr = curr->next) {
        if (curr->type == AST_VAR_DECL) {
            if (codegen_emit_var_decl(output, curr, 0, codegen_emit_expression) != 0) {
                ctx.errors_count++;
                break;
            }
            emitted_any = true;
        } else if (curr->type == AST_FUNC_DEF) {
            if (emitted_any) {
                fputc('\n', output);
            }
            g_pending_globals_func = curr;
            int func_rc = codegen_emit_function(output, curr, 0, emit_statement);
            g_pending_globals_func = NULL;
            if (func_rc != 0) {
                ctx.errors_count++;
                break;
            }
            emitted_any = true;
        } else {
            ctx.errors_count++;
            break;
        }
    }

    if (ctx.errors_count == 0) {
        if (emitted_any) {
            fputc('\n', output);
        }
        if (codegen_emit_python_entrypoint(output) != 0) {
            ctx.errors_count++;
        }
    }

    g_program_declarations = NULL;
    symtab_free(global_scope);
    ctx.global_scope = NULL;

    return ctx.errors_count;
}
