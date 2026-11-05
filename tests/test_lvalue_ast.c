
#include "ast.h"
#include "parser.tab.h"
#include "semantic.h"

/* As expressões binárias e unárias não são aceitas pelo parser como alvo,
   mas a validação semântica deve se manter segura para qualquer AST. */
static int check_invalid_target(ASTNode *target) {
    ASTNode *assignment = ast_create_assign(
        OP_ASSIGN,
        target,
        ast_create_lit_int(2, 1, 1),
        1, 1
    );

    ASTNode *stmt = ast_create_expr_stmt(assignment, 1, 1);
    ASTNode *body = ast_create_block(stmt, 1, 1);

    ASTNode *function = ast_create_func_def(
        data_type_primitive(TYPE_VOID),
        "teste",
        NULL,
        body,
        1, 1
    );

    ASTNode *program = ast_create_program(function, 1, 1);
    int errors = semantic_analyze(program, NULL);

    ast_free(program);
    return errors == 1;
}

int main(void) {
    ASTNode *binary = ast_create_bin_op(
        OP_PLUS,
        ast_create_lit_int(1, 1, 1),
        ast_create_lit_int(2, 1, 1),
        1, 1
    );

    ASTNode *unary = ast_create_un_op(
        OP_MINUS,
        ast_create_lit_int(1, 1, 1),
        1, 1
    );

    return check_invalid_target(binary) &&
           check_invalid_target(unary) ? 0 : 1;
}
