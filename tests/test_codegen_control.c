#include "codegen_control.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static const char *expr_text;
static int emit_expr(FILE *out, const ASTNode *expr) {
    if (expr->type == AST_LIT_INT) {
        return fprintf(out, "%d", expr->int_val) < 0 ? -1 : 0;
    }
    return fputs(expr_text, out) == EOF ? -1 : 0;
}

static void check(FILE *out, const char *expected) {
    char actual[512];
    size_t size;
    rewind(out);
    size = fread(actual, 1, sizeof(actual) - 1, out);
    actual[size] = '\0';
    assert(strcmp(actual, expected) == 0);
}

int main(void) {
    ASTNode condition = {0};
    condition.type = AST_LIT_BOOL;
    ASTNode ret_value = {0};
    ret_value.type = AST_LIT_INT;
    ret_value.int_val = 5;
    ASTNode ret = {0};
    ret.type = AST_RETURN;
    ret.left = &ret_value;
    ASTNode then_block = {0};
    then_block.type = AST_BLOCK;
    then_block.left = &ret;
    ASTNode otherwise = {0};
    otherwise.type = AST_CONTINUE;
    ASTNode branch = {0};
    branch.type = AST_IF;
    branch.left = &condition;
    branch.right = &then_block;
    branch.extra = &otherwise;

    FILE *out = tmpfile();
    assert(out != NULL);
    expr_text = "x > 0";
    assert(codegen_emit_control(out, &branch, 1, emit_expr) == 0);
    check(out, "    if x > 0:\n        return 5\n    else:\n        continue\n");
    fclose(out);

    ASTNode empty = {0};
    empty.type = AST_BLOCK;
    out = tmpfile();
    assert(codegen_emit_control(out, &empty, 0, emit_expr) == 0);
    check(out, "pass\n");
    fclose(out);

    ASTNode no_value = {0};
    no_value.type = AST_RETURN;
    out = tmpfile();
    assert(codegen_emit_control(out, &no_value, 0, NULL) == 0);
    check(out, "return\n");
    fclose(out);

    ASTNode bad = {0};
    bad.type = AST_IF;
    out = tmpfile();
    assert(codegen_emit_control(out, &bad, 0, emit_expr) == -1);
    assert(codegen_emit_control(NULL, &branch, 0, emit_expr) == -1);
    fclose(out);
    puts("Codegen #84: selecao e desvios aprovados.");
    return 0;
}
