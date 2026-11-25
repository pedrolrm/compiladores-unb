#ifndef CODEGEN_CONTROL_H
#define CODEGEN_CONTROL_H

#include "ast.h"

typedef int (*CodegenControlExpressionEmitter)(FILE *out,
                                               const ASTNode *expression);

typedef int (*CodegenControlStatementEmitter)(FILE *out,
                                              const ASTNode *statement,
                                              int indent_level);

/*
 * Emite if/else, break, continue e return com indentação Python.
 * Comandos não-controle dentro de blocos ou ramos if/else (e continue quando
 * emit_statement é fornecido) são delegados a emit_statement.
 */
int codegen_emit_control(FILE *out, const ASTNode *node, int indent_level,
                         CodegenControlExpressionEmitter emit_expression,
                         CodegenControlStatementEmitter emit_statement);

#endif /* CODEGEN_CONTROL_H */
