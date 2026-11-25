#include "codegen_globals.h"
#include <string.h>

#define MAX_GLOBAL_NAMES 128

typedef struct {
    const char *name;
} GlobalName;

static bool contains_name(const GlobalName *names, int count, const char *name) {
    for (int i = 0; i < count; i++) {
        if (strcmp(names[i].name, name) == 0) return true;
    }
    return false;
}

static int add_unique_name(GlobalName *names, int *count, int capacity,
                           const char *name) {
    if (!name || contains_name(names, *count, name)) return 0;
    if (*count >= capacity) return -1;
    names[(*count)++].name = name;
    return 0;
}

static int collect_global_names(const ASTNode *node, GlobalName *globals,
                                int *count, int capacity) {
    for (const ASTNode *curr = node; curr; curr = curr->next) {
        if (curr->type == AST_VAR_DECL && curr->name) {
            if (add_unique_name(globals, count, capacity, curr->name) != 0) {
                return -1;
            }
        }
    }
    return 0;
}

static int collect_shadowed_names(const ASTNode *node, GlobalName *shadowed,
                                  int *count, int capacity) {
    for (const ASTNode *curr = node; curr; curr = curr->next) {
        if ((curr->type == AST_PARAM_DECL || curr->type == AST_VAR_DECL) &&
            curr->name) {
            if (add_unique_name(shadowed, count, capacity, curr->name) != 0) {
                return -1;
            }
        }
        if (curr->left &&
            collect_shadowed_names(curr->left, shadowed, count, capacity) != 0) {
            return -1;
        }
        if (curr->right &&
            collect_shadowed_names(curr->right, shadowed, count, capacity) != 0) {
            return -1;
        }
        if (curr->extra &&
            collect_shadowed_names(curr->extra, shadowed, count, capacity) != 0) {
            return -1;
        }
    }
    return 0;
}

static int collect_assignment_target(const ASTNode *target,
                                     const GlobalName *globals, int global_count,
                                     const GlobalName *shadowed, int shadowed_count,
                                     GlobalName *used, int *used_count,
                                     int capacity) {
    if (!target) return 0;

    const ASTNode *base = target;
    if (target->type == AST_ARRAY_ACCESS) base = target->left;

    if (base && base->type == AST_IDENTIFIER && base->name &&
        contains_name(globals, global_count, base->name) &&
        !contains_name(shadowed, shadowed_count, base->name)) {
        return add_unique_name(used, used_count, capacity, base->name);
    }
    return 0;
}

static int collect_assignments(const ASTNode *node, const GlobalName *globals,
                               int global_count, const GlobalName *shadowed,
                               int shadowed_count, GlobalName *used,
                               int *used_count, int capacity) {
    for (const ASTNode *curr = node; curr; curr = curr->next) {
        if (curr->type == AST_ASSIGN &&
            collect_assignment_target(curr->left, globals, global_count,
                                      shadowed, shadowed_count, used,
                                      used_count, capacity) != 0) {
            return -1;
        }
        if (curr->left &&
            collect_assignments(curr->left, globals, global_count, shadowed,
                                shadowed_count, used, used_count, capacity) != 0) {
            return -1;
        }
        if (curr->right &&
            collect_assignments(curr->right, globals, global_count, shadowed,
                                shadowed_count, used, used_count, capacity) != 0) {
            return -1;
        }
        if (curr->extra &&
            collect_assignments(curr->extra, globals, global_count, shadowed,
                                shadowed_count, used, used_count, capacity) != 0) {
            return -1;
        }
    }
    return 0;
}

int codegen_emit_function_globals(FILE *out, const ASTNode *function,
                                  const ASTNode *global_declarations,
                                  int indent_level) {
    if (!out || !function || function->type != AST_FUNC_DEF || indent_level < 0) {
        return -1;
    }

    GlobalName globals[MAX_GLOBAL_NAMES] = {0};
    GlobalName shadowed[MAX_GLOBAL_NAMES] = {0};
    GlobalName used[MAX_GLOBAL_NAMES] = {0};
    int global_count = 0;
    int shadowed_count = 0;
    int used_count = 0;

    if (collect_global_names(global_declarations, globals, &global_count,
                             MAX_GLOBAL_NAMES) != 0) {
        return -1;
    }
    if (collect_shadowed_names(function->left, shadowed, &shadowed_count,
                               MAX_GLOBAL_NAMES) != 0 ||
        collect_shadowed_names(function->right, shadowed, &shadowed_count,
                               MAX_GLOBAL_NAMES) != 0) {
        return -1;
    }
    if (collect_assignments(function->right, globals, global_count, shadowed,
                            shadowed_count, used, &used_count,
                            MAX_GLOBAL_NAMES) != 0) {
        return -1;
    }

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
