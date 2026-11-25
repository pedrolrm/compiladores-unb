#ifndef CODEGEN_VARS_H
#define CODEGEN_VARS_H

#include "ast.h"

/* O emissor de expressões retorna 0 em caso de sucesso e -1 em caso de erro. */
typedef int (*CodegenExpressionEmitter)(FILE *out, const ASTNode *expression);

/*
 * Emite uma declaração AST_VAR_DECL local ou global, com quatro espaços por
 * nível de indentação. Inicializadores escalares são delegados ao emissor de
 * expressões. Vetores devem ter tamanho fixo positivo e não ter inicializador.
 * Retorna 0 em caso de sucesso e -1 para declaração inválida ou erro de emissão.
 */
int codegen_emit_var_decl(FILE *out, const ASTNode *node, int indent_level,
                          CodegenExpressionEmitter emit_expression);

#endif /* CODEGEN_VARS_H */
