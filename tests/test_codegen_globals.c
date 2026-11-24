#include "codegen_globals.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void check(FILE *out, const char *expected) {
    char actual[256];
    size_t size;
    rewind(out);
    size = fread(actual, 1, sizeof(actual) - 1, out);
    actual[size] = '\0';
    assert(strcmp(actual, expected) == 0);
}

int main(void) {
    ASTNode global = {0};
    global.type = AST_VAR_DECL;
    global.name = "total";
    global.data_type.base = TYPE_INT;

    ASTNode target = {0};
    target.type = AST_IDENTIFIER;
    target.name = "total";
    ASTNode value = {0};
    value.type = AST_LIT_INT;
    ASTNode assignment = {0};
    assignment.type = AST_ASSIGN;
    assignment.left = &target;
    assignment.right = &value;
    ASTNode body = {0};
    body.type = AST_BLOCK;
    body.left = &assignment;
    ASTNode function = {0};
    function.type = AST_FUNC_DEF;
    function.right = &body;

    FILE *out = tmpfile();
    assert(out != NULL);
    assert(codegen_emit_function_globals(out, &function, &global, 1) == 0);
    check(out, "    global total\n");
    fclose(out);

    target.name = "local";
    out = tmpfile();
    assert(codegen_emit_function_globals(out, &function, &global, 0) == 0);
    check(out, "");
    fclose(out);

    out = tmpfile();
    assert(codegen_emit_python_entrypoint(out) == 0);
    check(out, "if __name__ == '__main__':\n    import sys\n    sys.exit(main())\n");
    fclose(out);

    assert(codegen_emit_python_entrypoint(NULL) == -1);
    puts("Codegen #83: globais e ponto de entrada aprovados.");
    return 0;
}
