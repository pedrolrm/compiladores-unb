#ifndef SYMTAB_H
#define SYMTAB_H

#include "ast.h"
#include <stdbool.h>

#define SYMTAB_HASH_SIZE 64

/*
 * Categoria do símbolo armazenado na tabela.
 */
typedef enum {
    SYM_VAR,
    SYM_PARAM,
    SYM_FUNC
} SymbolKind;

struct Scope;

/*
 * Entrada de símbolo com informações de tipo e escopo.
 */
typedef struct Symbol {
    char *name;
    SymbolKind kind;
    DataType type;
    int line;
    int column;
    int scope_level;

    /* Metadados para funções */
    DataType return_type;
    int param_count;
    DataType *param_types;

    struct Symbol *next;    /* Encadeamento para colisões na tabela hash */
} Symbol;

/*
 * Estrutura de escopo contendo sua tabela local e ponteiro para o escopo pai.
 */
typedef struct Scope {
    int level;
    struct Scope *parent;
    Symbol *buckets[SYMTAB_HASH_SIZE];

    /* Lista de escopos filhos preservada para análises posteriores */
    struct Scope **children;
    int child_count;
    int child_capacity;
} Scope;

/* Criação e destruição */
Scope *symtab_create_global(void);
Scope *symtab_enter_scope(Scope *current);
Scope *symtab_exit_scope(Scope *current);
void symtab_free(Scope *scope);

/* Inserção e buscas */
Symbol *symtab_insert(Scope *scope, Symbol *sym);
Symbol *symtab_insert_var(Scope *scope, const char *name, DataType type, int line, int col);
Symbol *symtab_insert_param(Scope *scope, const char *name, DataType type, int line, int col);
Symbol *symtab_insert_func(Scope *scope, const char *name, DataType ret_type, int param_count, DataType *param_types, int line, int col);

Symbol *symtab_lookup(const Scope *scope, const char *name);
Symbol *symtab_lookup_local(const Scope *scope, const char *name);

/* Visualização */
void symtab_print(const Scope *scope, int indent);

#endif /* SYMTAB_H */
