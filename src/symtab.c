#include "symtab.h"
#include <string.h>

/*
 * Função hash simples (djb2) para strings
 */
static unsigned int symtab_hash(const char *str) {
    unsigned int hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % SYMTAB_HASH_SIZE;
}

Scope *symtab_create_global(void) {
    Scope *scope = (Scope *)calloc(1, sizeof(Scope));
    if (!scope) {
        perror("calloc");
        exit(EXIT_FAILURE);
    }
    scope->level = 0;
    scope->parent = NULL;
    scope->child_capacity = 4;
    scope->children = (Scope **)calloc(scope->child_capacity, sizeof(Scope *));
    return scope;
}

Scope *symtab_enter_scope(Scope *current) {
    if (!current) return symtab_create_global();

    Scope *new_scope = (Scope *)calloc(1, sizeof(Scope));
    if (!new_scope) {
        perror("calloc");
        exit(EXIT_FAILURE);
    }
    new_scope->level = current->level + 1;
    new_scope->parent = current;
    new_scope->child_capacity = 4;
    new_scope->children = (Scope **)calloc(new_scope->child_capacity, sizeof(Scope *));

    /* Registra nos filhos do escopo atual para preservar a árvore */
    if (current->child_count >= current->child_capacity) {
        current->child_capacity *= 2;
        current->children = (Scope **)realloc(current->children, current->child_capacity * sizeof(Scope *));
    }
    current->children[current->child_count++] = new_scope;

    return new_scope;
}

Scope *symtab_exit_scope(Scope *current) {
    if (!current) return NULL;
    return current->parent;
}

Symbol *symtab_insert(Scope *scope, Symbol *sym) {
    if (!scope || !sym || !sym->name) return NULL;

    /* Verifica declaração duplicada no mesmo escopo */
    if (symtab_lookup_local(scope, sym->name) != NULL) {
        return NULL;
    }

    sym->scope_level = scope->level;
    unsigned int idx = symtab_hash(sym->name);
    sym->next = scope->buckets[idx];
    scope->buckets[idx] = sym;
    return sym;
}

Symbol *symtab_insert_var(Scope *scope, const char *name, DataType type, int line, int col) {
    Symbol *sym = (Symbol *)calloc(1, sizeof(Symbol));
    sym->name = strdup(name);
    sym->kind = SYM_VAR;
    sym->type = type;
    sym->line = line;
    sym->column = col;
    return symtab_insert(scope, sym);
}

Symbol *symtab_insert_param(Scope *scope, const char *name, DataType type, int line, int col) {
    Symbol *sym = (Symbol *)calloc(1, sizeof(Symbol));
    sym->name = strdup(name);
    sym->kind = SYM_PARAM;
    sym->type = type;
    sym->line = line;
    sym->column = col;
    return symtab_insert(scope, sym);
}

Symbol *symtab_insert_func(Scope *scope, const char *name, DataType ret_type, int param_count, DataType *param_types, int line, int col) {
    Symbol *sym = (Symbol *)calloc(1, sizeof(Symbol));
    sym->name = strdup(name);
    sym->kind = SYM_FUNC;
    sym->type = ret_type;
    sym->return_type = ret_type;
    sym->param_count = param_count;
    sym->line = line;
    sym->column = col;

    if (param_count > 0 && param_types) {
        sym->param_types = (DataType *)calloc(param_count, sizeof(DataType));
        for (int i = 0; i < param_count; i++) {
            sym->param_types[i] = param_types[i];
            if (param_types[i].base == TYPE_STRUCT && param_types[i].struct_name) {
                sym->param_types[i].struct_name = strdup(param_types[i].struct_name);
            }
        }
    }
    return symtab_insert(scope, sym);
}

Symbol *symtab_insert_struct(Scope *scope, const char *name, Scope *members, int line, int col) {
    Symbol *sym = (Symbol *)calloc(1, sizeof(Symbol));
    sym->name = strdup(name);
    sym->kind = SYM_STRUCT;
    sym->type = data_type_struct(name);
    sym->struct_scope = members;
    sym->line = line;
    sym->column = col;
    return symtab_insert(scope, sym);
}

Symbol *symtab_lookup_local(const Scope *scope, const char *name) {
    if (!scope || !name) return NULL;
    unsigned int idx = symtab_hash(name);
    Symbol *curr = scope->buckets[idx];
    while (curr) {
        if (strcmp(curr->name, name) == 0) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

Symbol *symtab_lookup(const Scope *scope, const char *name) {
    const Scope *curr_scope = scope;
    while (curr_scope) {
        Symbol *sym = symtab_lookup_local(curr_scope, name);
        if (sym) {
            return sym;
        }
        curr_scope = curr_scope->parent;
    }
    return NULL;
}

Symbol *symtab_lookup_member(const Scope *struct_scope, const char *member_name) {
    return symtab_lookup_local(struct_scope, member_name);
}

/*
 * Visualização formatada da tabela de símbolos
 */
static void print_indent(int indent) {
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
}

static const char *symbol_kind_to_string(SymbolKind kind) {
    switch (kind) {
        case SYM_VAR:    return "VAR";
        case SYM_PARAM:  return "PARAM";
        case SYM_FUNC:   return "FUNC";
        case SYM_STRUCT: return "STRUCT";
        default:         return "UNKNOWN";
    }
}

void symtab_print(const Scope *scope, int indent) {
    if (!scope) return;

    print_indent(indent);
    printf("--- Escopo Nível %d ---\n", scope->level);

    for (int i = 0; i < SYMTAB_HASH_SIZE; i++) {
        Symbol *sym = scope->buckets[i];
        while (sym) {
            print_indent(indent + 1);
            if (sym->kind == SYM_FUNC) {
                printf("[%s] %s -> Retorno: %s, Parâmetros: %d (Linha: %d)\n",
                       symbol_kind_to_string(sym->kind),
                       sym->name,
                       data_type_to_string(&sym->return_type),
                       sym->param_count,
                       sym->line);
            } else if (sym->kind == SYM_STRUCT) {
                printf("[%s] %s (Linha: %d)\n",
                       symbol_kind_to_string(sym->kind),
                       sym->name,
                       sym->line);
                if (sym->struct_scope) {
                    print_indent(indent + 2);
                    printf("Membros:\n");
                    symtab_print(sym->struct_scope, indent + 3);
                }
            } else {
                printf("[%s] %s : %s (Linha: %d)\n",
                       symbol_kind_to_string(sym->kind),
                       sym->name,
                       data_type_to_string(&sym->type),
                       sym->line);
            }
            sym = sym->next;
        }
    }

    for (int j = 0; j < scope->child_count; j++) {
        symtab_print(scope->children[j], indent + 1);
    }
}

/*
 * Liberação de memória dos escopos e símbolos
 */
void symtab_free(Scope *scope) {
    if (!scope) return;

    /* Libera todos os símbolos do escopo */
    for (int i = 0; i < SYMTAB_HASH_SIZE; i++) {
        Symbol *sym = scope->buckets[i];
        while (sym) {
            Symbol *next = sym->next;

            if (sym->name) free(sym->name);
            if (sym->type.struct_name) free(sym->type.struct_name);
            if (sym->return_type.struct_name) free(sym->return_type.struct_name);

            if (sym->param_types) {
                for (int p = 0; p < sym->param_count; p++) {
                    if (sym->param_types[p].struct_name) {
                        free(sym->param_types[p].struct_name);
                    }
                }
                free(sym->param_types);
            }

            if (sym->struct_scope) {
                symtab_free(sym->struct_scope);
            }

            free(sym);
            sym = next;
        }
    }

    /* Libera escopos filhos */
    for (int j = 0; j < scope->child_count; j++) {
        symtab_free(scope->children[j]);
    }
    if (scope->children) {
        free(scope->children);
    }

    free(scope);
}
