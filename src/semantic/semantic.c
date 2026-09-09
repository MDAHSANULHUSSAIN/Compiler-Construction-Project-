#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "semantic.h"

static int error_count = 0;

static void sem_error(int line, const char *fmt, ...) {
    error_count++;
    char buf[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    printf("SEMERR|%d|%s\n", line, buf);
}

static int was_ever_declared(SymbolTable *st, const char *name) {
    for (int i = 0; i < st->all_count; i++)
        if (strcmp(st->all_entries[i]->name, name) == 0) return 1;
    return 0;
}

static int is_numeric(DataType t) { return t == T_INT || t == T_FLOAT; }

static DataType check_expr(ASTNode *e, SymbolTable *st) {
    if (!e) return T_UNKNOWN;
    DataType result = T_UNKNOWN;
    switch (e->type) {
        case N_INT_LIT: result = T_INT; break;
        case N_FLOAT_LIT: result = T_FLOAT; break;
        case N_BOOL_LIT: result = T_BOOL; break;
        case N_IDENT: {
            SymEntry *entry = symtab_lookup(st, e->name);
            if (!entry) {
                if (was_ever_declared(st, e->name))
                    sem_error(e->line, "Scope violation: '%s' is used outside the block/scope in which it was declared", e->name);
                else
                    sem_error(e->line, "Undeclared variable '%s' used before declaration", e->name);
                result = T_UNKNOWN;
            } else {
                result = entry->type;
            }
            break;
        }
        case N_UNOP: {
            DataType t = check_expr(e->left, st);
            if (strcmp(e->op, "!") == 0) {
                if (t != T_BOOL && t != T_UNKNOWN)
                    sem_error(e->line, "Invalid expression: logical operator '!' applied to non-bool operand of type '%s'", type_to_str(t));
                result = T_BOOL;
            } else { /* unary minus */
                if (!is_numeric(t) && t != T_UNKNOWN)
                    sem_error(e->line, "Invalid expression: unary '-' applied to non-numeric operand of type '%s'", type_to_str(t));
                result = t;
            }
            break;
        }
        case N_BINOP: {
            DataType lt = check_expr(e->left, st);
            DataType rt = check_expr(e->right, st);
            const char *op = e->op;
            if (strcmp(op, "&&") == 0 || strcmp(op, "||") == 0) {
                if ((lt != T_BOOL && lt != T_UNKNOWN) || (rt != T_BOOL && rt != T_UNKNOWN))
                    sem_error(e->line, "Invalid expression: logical operator '%s' requires bool operands (got '%s' and '%s')", op, type_to_str(lt), type_to_str(rt));
                result = T_BOOL;
            } else if (strcmp(op, "<") == 0 || strcmp(op, ">") == 0 || strcmp(op, "<=") == 0 || strcmp(op, ">=") == 0) {
                if ((!is_numeric(lt) && lt != T_UNKNOWN) || (!is_numeric(rt) && rt != T_UNKNOWN))
                    sem_error(e->line, "Type mismatch: relational operator '%s' requires numeric operands (got '%s' and '%s')", op, type_to_str(lt), type_to_str(rt));
                result = T_BOOL;
            } else if (strcmp(op, "==") == 0 || strcmp(op, "!=") == 0) {
                if (lt != T_UNKNOWN && rt != T_UNKNOWN) {
                    int okBothNumeric = is_numeric(lt) && is_numeric(rt);
                    int okSame = (lt == rt);
                    if (!okBothNumeric && !okSame)
                        sem_error(e->line, "Type mismatch: cannot compare '%s' with '%s' using '%s'", type_to_str(lt), type_to_str(rt), op);
                }
                result = T_BOOL;
            } else { /* + - * / % */
                if ((!is_numeric(lt) && lt != T_UNKNOWN) || (!is_numeric(rt) && rt != T_UNKNOWN)) {
                    sem_error(e->line, "Invalid expression: arithmetic operator '%s' requires numeric operands (got '%s' and '%s')", op, type_to_str(lt), type_to_str(rt));
                    result = T_UNKNOWN;
                } else {
                    result = (lt == T_FLOAT || rt == T_FLOAT) ? T_FLOAT : T_INT;
                }
            }
            break;
        }
        default: break;
    }
    e->resolved_type = result;
    return result;
}

static void check_stmt(ASTNode *s, SymbolTable *st);

static void check_block_contents(ASTNode *block, SymbolTable *st) {
    for (int i = 0; i < block->child_count; i++) check_stmt(block->children[i], st);
}

static void check_stmt(ASTNode *s, SymbolTable *st) {
    if (!s) return;
    switch (s->type) {
        case N_VARDECL: {
            SymEntry *existing = symtab_declare(st, s->name, s->decl_type, s->line);
            if (existing)
                sem_error(s->line, "Redeclaration: '%s' is already declared in this scope (first declared at line %d)", s->name, existing->line_declared);
            break;
        }
        case N_ASSIGN: {
            SymEntry *entry = symtab_lookup(st, s->name);
            DataType rhs_t = check_expr(s->rhs, st);
            if (!entry) {
                if (was_ever_declared(st, s->name))
                    sem_error(s->line, "Scope violation: '%s' is used outside the block/scope in which it was declared", s->name);
                else
                    sem_error(s->line, "Undeclared variable '%s' used before declaration", s->name);
                break;
            }
            if (rhs_t == T_UNKNOWN) break; /* error already reported inside expr */
            int compatible = (entry->type == rhs_t) || (entry->type == T_FLOAT && rhs_t == T_INT);
            if (!compatible)
                sem_error(s->line, "Invalid assignment: cannot assign type '%s' to variable '%s' of type '%s'", type_to_str(rhs_t), s->name, type_to_str(entry->type));
            break;
        }
        case N_IF:
            check_expr(s->cond, st);
            if (s->cond->resolved_type != T_BOOL && s->cond->resolved_type != T_UNKNOWN)
                sem_error(s->line, "Type mismatch: 'if' condition must be bool (got '%s')", type_to_str(s->cond->resolved_type));
            symtab_enter_scope(st);
            check_block_contents(s->then_branch, st);
            symtab_exit_scope(st);
            if (s->else_branch) {
                symtab_enter_scope(st);
                check_block_contents(s->else_branch, st);
                symtab_exit_scope(st);
            }
            break;
        case N_WHILE:
            check_expr(s->cond, st);
            if (s->cond->resolved_type != T_BOOL && s->cond->resolved_type != T_UNKNOWN)
                sem_error(s->line, "Type mismatch: 'while' condition must be bool (got '%s')", type_to_str(s->cond->resolved_type));
            symtab_enter_scope(st);
            check_block_contents(s->then_branch, st);
            symtab_exit_scope(st);
            break;
        case N_PRINT:
            check_expr(s->expr, st);
            break;
        case N_BLOCK:
            symtab_enter_scope(st);
            check_block_contents(s, st);
            symtab_exit_scope(st);
            break;
        default:
            break;
    }
}

int semantic_analyze(ASTNode *program, SymbolTable *st) {
    error_count = 0;
    for (int i = 0; i < program->child_count; i++) check_stmt(program->children[i], st);
    return error_count;
}
