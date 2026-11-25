#ifndef CODEGEN_EXPR_H
#define CODEGEN_EXPR_H

#include "ast.h"

/*
 * Emite uma expressão da AST em sintaxe Python 3, sem indentação e sem
 * quebra de linha.
 *
 * - Operações binárias e unárias são emitidas entre parênteses, preservando
 *   a precedência e o agrupamento da expressão original em C.
 * - A divisão entre dois operandos int é emitida como divisão inteira (//).
 * - &&, || e ! são mapeados para and, or e not; true e false para True e False.
 *
 * Requer que a análise semântica já tenha preenchido o data_type dos nós.
 * Retorna 0 em caso de sucesso e -1 para nó não suportado ou erro de emissão.
 */
int codegen_emit_expression(FILE *out, const ASTNode *expression);

#endif /* CODEGEN_EXPR_H */
