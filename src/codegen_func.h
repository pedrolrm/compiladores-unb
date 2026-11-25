#ifndef CODEGEN_FUNC_H
#define CODEGEN_FUNC_H

#include "ast.h"

/* O emissor de comandos retorna 0 em caso de sucesso e -1 em caso de erro. */
typedef int (*CodegenFunctionStatementEmitter)(FILE *out, const ASTNode *statement,
                                               int indent_level);

/*
 * Emite uma definição AST_FUNC_DEF como `def nome(p1, p2, ...):`, com o corpo
 * um nível mais indentado (quatro espaços por nível). Os parâmetros vêm da
 * lista de nós AST_PARAM_DECL encadeados em node->left e o corpo de node->right,
 * delegado ao emissor de comandos. Corpo vazio gera `pass`, evitando
 * IndentationError na execução do Python.
 * Retorna 0 em caso de sucesso e -1 para nó inválido ou erro de emissão.
 */
int codegen_emit_function(FILE *out, const ASTNode *function, int indent_level,
                          CodegenFunctionStatementEmitter emit_statement);

#endif /* CODEGEN_FUNC_H */
