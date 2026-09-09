#ifndef CODEGEN_H
#define CODEGEN_H

#include "../ast/ast.h"

/* Generates Three Address Code (TAC) for the given (semantically
 * validated) AST and prints it to stdout, one instruction per line. */
void codegen_generate(ASTNode *program);

#endif
