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

    /* Parâmetro com mesmo nome da global não pode gerar `global` */
    ASTNode param = {0};
    param.type = AST_PARAM_DECL;
    param.name = "total";
    function.left = &param;
    out = tmpfile();
    assert(out != NULL);
    assert(codegen_emit_function_globals(out, &function, &global, 1) == 0);
    check(out, "");
    fclose(out);
    function.left = NULL;

    /* Variável local com mesmo nome da global não pode gerar `global` */
    ASTNode local_decl = {0};
    local_decl.type = AST_VAR_DECL;
    local_decl.name = "total";
    local_decl.next = &assignment;
    body.left = &local_decl;
    out = tmpfile();
    assert(out != NULL);
    assert(codegen_emit_function_globals(out, &function, &global, 1) == 0);
    check(out, "");
    fclose(out);
    body.left = &assignment;

    /* Atribuição em variável local não declarada como global */
    target.name = "local";
    out = tmpfile();
    assert(out != NULL);
    assert(codegen_emit_function_globals(out, &function, &global, 0) == 0);
    check(out, "");
    fclose(out);

    out = tmpfile();
    assert(out != NULL);
    assert(codegen_emit_python_entrypoint(out) == 0);
    check(out, "if __name__ == '__main__':\n    import sys\n    sys.exit(main())\n");
    fclose(out);

    assert(codegen_emit_function_globals(NULL, &function, &global, 0) == -1);
    assert(codegen_emit_function_globals(out, NULL, &global, 0) == -1);
    assert(codegen_emit_function_globals(out, &body, &global, 0) == -1);
    assert(codegen_emit_function_globals(out, &function, &global, -1) == -1);
    assert(codegen_emit_python_entrypoint(NULL) == -1);
    puts("Codegen #83: globais e ponto de entrada aprovados.");
    return 0;
}
