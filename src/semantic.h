#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"
#include "symtab.h"

extern int semantic_errors_count;

/*
 * Executa a análise semântica sobre a AST gerada.
 * Popula a tabela de símbolos global e valida escopos e tipos.
 * Retorna o número total de erros semânticos encontrados.
 */
int semantic_analyze(ASTNode *root, Scope **out_global_scope);

/*
 * Emite mensagem padronizada de erro semântico com linha e coluna.
 */
void report_semantic_error(int line, int col, const char *msg);

#endif /* SEMANTIC_H */
