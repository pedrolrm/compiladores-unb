#include "codegen.h"
#include <stdarg.h>

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

    symtab_free(global_scope);
    ctx.global_scope = NULL;

    return ctx.errors_count;
}
