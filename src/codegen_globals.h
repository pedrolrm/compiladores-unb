#ifndef CODEGEN_GLOBALS_H
#define CODEGEN_GLOBALS_H

#include "ast.h"

/* Emite `global` para os globais atribuídos no corpo da função. */
int codegen_emit_function_globals(FILE *out, const ASTNode *function,
                                  const ASTNode *global_declarations,
                                  int indent_level);

/* Emite o ponto de entrada Python que executa main(). */
int codegen_emit_python_entrypoint(FILE *out);

#endif /* CODEGEN_GLOBALS_H */
