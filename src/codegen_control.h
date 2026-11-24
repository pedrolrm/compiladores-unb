#ifndef CODEGEN_CONTROL_H
#define CODEGEN_CONTROL_H

#include "ast.h"

typedef int (*CodegenControlExpressionEmitter)(FILE *out,
                                               const ASTNode *expression);

/* Emite if/else, break, continue e return com indentação Python. */
int codegen_emit_control(FILE *out, const ASTNode *node, int indent_level,
                         CodegenControlExpressionEmitter emit_expression);

#endif /* CODEGEN_CONTROL_H */
