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

/*
 * Análise de Fluxo de Controle (Control Flow Analysis):
 * Verifica recursivamente se todos os caminhos de execução na AST de um comando/bloco
 * terminam garantidamente em um comando return com expressão válida.
 */
bool has_guaranteed_return(ASTNode *body);

#endif /* SEMANTIC_H */
