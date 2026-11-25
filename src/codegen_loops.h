#ifndef CODEGEN_LOOPS_H
#define CODEGEN_LOOPS_H

#include "ast.h"

/*
 * Emite um único comando no nível de indentação informado, sem seguir
 * statement->next. Usado para o corpo dos laços e para a inicialização do for.
 * Retorna 0 em caso de sucesso e -1 em caso de erro.
 */
typedef int (*CodegenStatementEmitter)(FILE *out, const ASTNode *statement, int indent_level);

/*
 * Emite AST_WHILE como `while cond:` com o corpo um nível mais indentado.
 * Corpo vazio gera `pass`.
 * Retorna 0 em caso de sucesso e -1 para nó inválido ou erro de emissão.
 */
int codegen_emit_while(FILE *out, const ASTNode *node, int indent_level,
                       CodegenStatementEmitter emit_statement);

/*
 * Emite AST_FOR na forma estruturada equivalente:
 *
 *     inicializacao
 *     while cond:        (while True: quando a condição é vazia)
 *         corpo
 *         passo
 *
 * Retorna 0 em caso de sucesso e -1 para nó inválido ou erro de emissão.
 */
int codegen_emit_for(FILE *out, const ASTNode *node, int indent_level,
                     CodegenStatementEmitter emit_statement);

/*
 * Emite `continue`. Dentro de um for com passo, emite antes o passo do laço
 * mais interno, já que em C o continue de um for ainda executa o passo.
 * Retorna -1 quando chamado fora de um laço.
 */
int codegen_emit_continue(FILE *out, int indent_level);

/*
 * Emite uma atribuição (=, +=, -=, *=, /=) como comando Python em uma linha.
 * /= entre dois operandos int é emitido como //=.
 * Retorna 0 em caso de sucesso e -1 para nó inválido ou erro de emissão.
 */
int codegen_emit_assignment(FILE *out, const ASTNode *assignment, int indent_level);

#endif /* CODEGEN_LOOPS_H */
