#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

ASTNode *ast_new(NodeType type, int line) {
    ASTNode *n = calloc(1, sizeof(ASTNode));
    n->type = type;
    n->line = line;
    n->resolved_type = T_UNKNOWN;
    return n;
}

void ast_add_child(ASTNode *parent, ASTNode *child) {
    if (parent->child_count >= parent->child_cap) {
        parent->child_cap = parent->child_cap == 0 ? 4 : parent->child_cap * 2;
        parent->children = realloc(parent->children, sizeof(ASTNode *) * parent->child_cap);
    }
    parent->children[parent->child_count++] = child;
}

ASTNode *ast_program(void) { return ast_new(N_PROGRAM, 0); }
ASTNode *ast_block(int line) { return ast_new(N_BLOCK, line); }

ASTNode *ast_vardecl(DataType t, const char *name, int line) {
    ASTNode *n = ast_new(N_VARDECL, line);
    n->decl_type = t;
    n->name = strdup(name);
    return n;
}

ASTNode *ast_assign(const char *name, ASTNode *rhs, int line) {
    ASTNode *n = ast_new(N_ASSIGN, line);
    n->name = strdup(name);
    n->rhs = rhs;
    return n;
}

ASTNode *ast_if(ASTNode *cond, ASTNode *then_b, ASTNode *else_b, int line) {
    ASTNode *n = ast_new(N_IF, line);
    n->cond = cond;
    n->then_branch = then_b;
    n->else_branch = else_b;
    return n;
}

ASTNode *ast_while(ASTNode *cond, ASTNode *body, int line) {
    ASTNode *n = ast_new(N_WHILE, line);
    n->cond = cond;
    n->then_branch = body;
    return n;
}

ASTNode *ast_print(ASTNode *expr, int line) {
    ASTNode *n = ast_new(N_PRINT, line);
    n->expr = expr;
    return n;
}

ASTNode *ast_binop(const char *op, ASTNode *l, ASTNode *r, int line) {
    ASTNode *n = ast_new(N_BINOP, line);
    n->op = strdup(op);
    n->left = l;
    n->right = r;
    return n;
}

ASTNode *ast_unop(const char *op, ASTNode *operand, int line) {
    ASTNode *n = ast_new(N_UNOP, line);
    n->op = strdup(op);
    n->left = operand;
    return n;
}

ASTNode *ast_int(long v, int line) {
    ASTNode *n = ast_new(N_INT_LIT, line);
    n->ival = v;
    n->resolved_type = T_INT;
    return n;
}

ASTNode *ast_float(double v, int line) {
    ASTNode *n = ast_new(N_FLOAT_LIT, line);
    n->fval = v;
    n->resolved_type = T_FLOAT;
    return n;
}

ASTNode *ast_bool(int v, int line) {
    ASTNode *n = ast_new(N_BOOL_LIT, line);
    n->bval = v;
    n->resolved_type = T_BOOL;
    return n;
}

ASTNode *ast_ident(const char *name, int line) {
    ASTNode *n = ast_new(N_IDENT, line);
    n->name = strdup(name);
    return n;
}

const char *type_to_str(DataType t) {
    switch (t) {
        case T_INT: return "int";
        case T_FLOAT: return "float";
        case T_BOOL: return "bool";
        default: return "unknown";
    }
}

static void indent_print(int indent) {
    for (int i = 0; i < indent; i++) printf("  ");
}

void ast_print_tree(ASTNode *node, int indent) {
    if (!node) return;
    indent_print(indent);
    switch (node->type) {
        case N_PROGRAM:
            printf("Program\n");
            for (int i = 0; i < node->child_count; i++) ast_print_tree(node->children[i], indent + 1);
            break;
        case N_BLOCK:
            printf("Block (line %d)\n", node->line);
            for (int i = 0; i < node->child_count; i++) ast_print_tree(node->children[i], indent + 1);
            break;
        case N_VARDECL:
            printf("VarDecl <%s> %s (line %d)\n", type_to_str(node->decl_type), node->name, node->line);
            break;
        case N_ASSIGN:
            printf("Assign %s = (line %d)\n", node->name, node->line);
            ast_print_tree(node->rhs, indent + 1);
            break;
        case N_IF:
            printf("If (line %d)\n", node->line);
            indent_print(indent + 1); printf("Cond:\n");
            ast_print_tree(node->cond, indent + 2);
            indent_print(indent + 1); printf("Then:\n");
            ast_print_tree(node->then_branch, indent + 2);
            if (node->else_branch) {
                indent_print(indent + 1); printf("Else:\n");
                ast_print_tree(node->else_branch, indent + 2);
            }
            break;
        case N_WHILE:
            printf("While (line %d)\n", node->line);
            indent_print(indent + 1); printf("Cond:\n");
            ast_print_tree(node->cond, indent + 2);
            indent_print(indent + 1); printf("Body:\n");
            ast_print_tree(node->then_branch, indent + 2);
            break;
        case N_PRINT:
            printf("Print (line %d)\n", node->line);
            ast_print_tree(node->expr, indent + 1);
            break;
        case N_BINOP:
            printf("BinOp '%s' (line %d)\n", node->op, node->line);
            ast_print_tree(node->left, indent + 1);
            ast_print_tree(node->right, indent + 1);
            break;
        case N_UNOP:
            printf("UnOp '%s' (line %d)\n", node->op, node->line);
            ast_print_tree(node->left, indent + 1);
            break;
        case N_INT_LIT:
            printf("IntLit %ld (line %d)\n", node->ival, node->line);
            break;
        case N_FLOAT_LIT:
            printf("FloatLit %g (line %d)\n", node->fval, node->line);
            break;
        case N_BOOL_LIT:
            printf("BoolLit %s (line %d)\n", node->bval ? "true" : "false", node->line);
            break;
        case N_IDENT:
            printf("Ident %s (line %d)\n", node->name, node->line);
            break;
    }
}

void ast_free(ASTNode *node) {
    if (!node) return;
    for (int i = 0; i < node->child_count; i++) ast_free(node->children[i]);
    free(node->children);
    ast_free(node->rhs);
    ast_free(node->cond);
    ast_free(node->then_branch);
    ast_free(node->else_branch);
    ast_free(node->expr);
    ast_free(node->left);
    ast_free(node->right);
    free(node->name);
    free(node->op);
    free(node);
}
