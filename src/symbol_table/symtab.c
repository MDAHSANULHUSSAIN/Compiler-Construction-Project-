#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

SymbolTable *symtab_create(void) {
    SymbolTable *st = calloc(1, sizeof(SymbolTable));
    st->next_level = 0;
    symtab_enter_scope(st); /* global scope, level 0 */
    return st;
}

void symtab_enter_scope(SymbolTable *st) {
    Scope *s = calloc(1, sizeof(Scope));
    s->parent = st->current;
    s->level = st->next_level++;
    st->current = s;
}

void symtab_exit_scope(SymbolTable *st) {
    Scope *s = st->current;
    if (!s) return;
    st->current = s->parent;
    /* Note: entries themselves are NOT freed here; they remain owned
     * by the 'all_entries' history log for later printing/debugging. */
    free(s);
}

static void log_entry(SymbolTable *st, SymEntry *e) {
    if (st->all_count >= st->all_cap) {
        st->all_cap = st->all_cap == 0 ? 8 : st->all_cap * 2;
        st->all_entries = realloc(st->all_entries, sizeof(SymEntry *) * st->all_cap);
    }
    st->all_entries[st->all_count++] = e;
}

SymEntry *symtab_declare(SymbolTable *st, const char *name, DataType type, int line) {
    /* check redeclaration within the current scope only */
    for (SymEntry *e = st->current->entries; e; e = e->next) {
        if (strcmp(e->name, name) == 0) {
            return e; /* already declared here -> caller reports error */
        }
    }
    SymEntry *e = calloc(1, sizeof(SymEntry));
    e->name = strdup(name);
    e->type = type;
    e->scope_level = st->current->level;
    e->line_declared = line;
    e->next = st->current->entries;
    st->current->entries = e;
    log_entry(st, e);
    return NULL;
}

SymEntry *symtab_lookup(SymbolTable *st, const char *name) {
    for (Scope *s = st->current; s; s = s->parent) {
        for (SymEntry *e = s->entries; e; e = e->next) {
            if (strcmp(e->name, name) == 0) return e;
        }
    }
    return NULL;
}

void symtab_print_all(SymbolTable *st) {
    printf("%-15s %-8s %-8s %-14s\n", "Name", "Type", "Scope", "Line Declared");
    printf("--------------------------------------------------\n");
    for (int i = 0; i < st->all_count; i++) {
        SymEntry *e = st->all_entries[i];
        printf("%-15s %-8s %-8d %-14d\n", e->name, type_to_str(e->type), e->scope_level, e->line_declared);
    }
    if (st->all_count == 0) printf("(no declared symbols)\n");
}

void symtab_free(SymbolTable *st) {
    while (st->current) symtab_exit_scope(st);
    for (int i = 0; i < st->all_count; i++) {
        free(st->all_entries[i]->name);
        free(st->all_entries[i]);
    }
    free(st->all_entries);
    free(st);
}
