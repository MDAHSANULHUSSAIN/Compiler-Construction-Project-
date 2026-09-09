#ifndef SYMTAB_H
#define SYMTAB_H

#include "../ast/ast.h"

/* A single variable entry */
typedef struct SymEntry {
    char *name;
    DataType type;
    int scope_level;
    int line_declared;
    struct SymEntry *next;   /* chaining within a scope (linked list) */
} SymEntry;

/* One lexical scope. Scopes are stacked (nested blocks). */
typedef struct Scope {
    SymEntry *entries;
    struct Scope *parent;
    int level;
} Scope;

/* The symbol table = a stack of scopes + a flat log of every entry
 * ever inserted (so we can print a full symbol table at the end,
 * including variables from blocks that have since closed). */
typedef struct {
    Scope *current;
    int next_level;

    SymEntry **all_entries;   /* history log for "Show Symbol Table" */
    int all_count;
    int all_cap;
} SymbolTable;

SymbolTable *symtab_create(void);
void symtab_enter_scope(SymbolTable *st);
void symtab_exit_scope(SymbolTable *st);

/* Returns NULL on success, or a pointer to the pre-existing entry
 * if 'name' is already declared in the *current* scope (redeclaration). */
SymEntry *symtab_declare(SymbolTable *st, const char *name, DataType type, int line);

/* Looks up a name walking outward through enclosing scopes.
 * Returns NULL if not found (undeclared / out of scope). */
SymEntry *symtab_lookup(SymbolTable *st, const char *name);

void symtab_print_all(SymbolTable *st);
void symtab_free(SymbolTable *st);

#endif
