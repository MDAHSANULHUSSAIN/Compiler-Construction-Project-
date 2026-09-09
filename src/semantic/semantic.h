#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "../ast/ast.h"
#include "../symbol_table/symtab.h"

/* Runs full semantic analysis over the AST, populating the symbol
 * table as it goes (declarations create entries; blocks push/pop
 * scopes). Returns the number of semantic errors found (0 = clean).
 * Error messages are printed to stdout prefixed with "SEMERR|". */
int semantic_analyze(ASTNode *program, SymbolTable *st);

#endif
