#include "codegen_globals.h"
#include <string.h>

typedef struct {
    const char *name;
} GlobalName;

static bool contains_name(const GlobalName *names, int count, const char *name) {
    for (int i = 0; i < count; i++) {
        if (strcmp(names[i].name, name) == 0) return true;
    }
    return false;
}

static bool is_global_name(const GlobalName *globals, int global_count,
                          const char *name) {
    return name && contains_name(globals, global_count, name);
}

static void collect_global_names(const ASTNode *node, GlobalName *globals,
                                 int *count, int capacity) {
    for (const ASTNode *curr = node; curr; curr = curr->next) {
        if (curr->type == AST_VAR_DECL && curr->name &&
            !contains_name(globals, *count, curr->name) && *count < capacity) {
            globals[(*count)++].name = curr->name;
        }
    }
}

static void collect_assignment_target(const ASTNode *target,
                                      const GlobalName *globals,
                                      int global_count, GlobalName *used,
                                      int *used_count, int capacity) {
    if (!target) return;

    const ASTNode *base = target;
    if (target->type == AST_ARRAY_ACCESS) base = target->left;

    if (base && base->type == AST_IDENTIFIER &&
        is_global_name(globals, global_count, base->name) &&
        !contains_name(used, *used_count, base->name) && *used_count < capacity) {
        used[(*used_count)++].name = base->name;
    }
}

static void collect_assignments(const ASTNode *node, const GlobalName *globals,
                                int global_count, GlobalName *used,
                                int *used_count, int capacity) {
    for (const ASTNode *curr = node; curr; curr = curr->next) {
        if (curr->type == AST_ASSIGN) {
            collect_assignment_target(curr->left, globals, global_count,
                                       used, used_count, capacity);
        }
        if (curr->left) {
            collect_assignments(curr->left, globals, global_count, used,
                                used_count, capacity);
        }
        if (curr->right) {
            collect_assignments(curr->right, globals, global_count, used,
                                used_count, capacity);
        }
        if (curr->extra) {
            collect_assignments(curr->extra, globals, global_count, used,
                                used_count, capacity);
        }
    }
}

int codegen_emit_function_globals(FILE *out, const ASTNode *function,
                                  const ASTNode *global_declarations,
                                  int indent_level) {
    if (!out || !function || function->type != AST_FUNC_DEF || indent_level < 0) {
        return -1;
    }

    GlobalName globals[128] = {0};
    GlobalName used[128] = {0};
    int global_count = 0;
    int used_count = 0;
    collect_global_names(global_declarations, globals, &global_count, 128);
    collect_assignments(function->right, globals, global_count, used,
                        &used_count, 128);

    if (used_count == 0) return 0;

    for (int i = 0; i < indent_level; i++) fputs("    ", out);
    fputs("global ", out);
    for (int i = 0; i < used_count; i++) {
        if (i > 0) fputs(", ", out);
        fputs(used[i].name, out);
    }
    fputc('\n', out);
    return ferror(out) ? -1 : 0;
}

int codegen_emit_python_entrypoint(FILE *out) {
    if (!out) return -1;
    fputs("if __name__ == '__main__':\n", out);
    fputs("    import sys\n", out);
    fputs("    sys.exit(main())\n", out);
    return ferror(out) ? -1 : 0;
}
