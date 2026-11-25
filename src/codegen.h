#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include "symtab.h"
#include <stdbool.h>
#include <stdio.h>

/* Quantidade de espaços por nível de indentação (PEP 8). */
#define CODEGEN_INDENT_SPACES 4

/*
 * Contexto de geração de código Python 3.
 * Mantém o fluxo de saída, o nível de indentação atual e o escopo global
 * com as variáveis e funções declaradas no programa.
 */
typedef struct CodegenContext {
    FILE *out;              /* Ponteiro para o arquivo de saída */
    int indent_level;       /* Nível atual de indentação (4 espaços por nível) */
    Scope *global_scope;    /* Escopo global / tabela de variáveis globais */
    bool in_function;       /* Indica se a emissão ocorre dentro de uma função */
    int errors_count;       /* Quantidade de erros ocorridos na geração */
} CodegenContext;

/*
 * Inicializa uma estrutura CodegenContext com o arquivo de saída e o escopo global.
 */
void codegen_context_init(CodegenContext *ctx, FILE *out, Scope *global_scope);

/*
 * Emite os espaços correspondentes ao nível de indentação atual
 * (CODEGEN_INDENT_SPACES * ctx->indent_level) no arquivo de saída.
 */
void codegen_indent(CodegenContext *ctx);

/*
 * Emite texto formatado no arquivo de saída sem aplicar indentação
 * nem adicionar quebra de linha ao final.
 */
void codegen_emit(CodegenContext *ctx, const char *fmt, ...);

/*
 * Emite uma linha indentada e formatada seguida de quebra de linha ('\n').
 * Quando fmt é vazio (""), emite apenas a quebra de linha sem espaços extras.
 */
void codegen_emit_line(CodegenContext *ctx, const char *fmt, ...);

/*
 * Ponto de entrada da geração de código Python 3 a partir da AST.
 * Retorna 0 em caso de sucesso ou valor diferente de 0 em caso de erro.
 */
int codegen_generate(ASTNode *root, FILE *output);

#endif /* CODEGEN_H */
