#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codegen.h"

static int temp_count = 0;
static int label_count = 0;

static char *new_temp(void) {
    char *buf = malloc(16);
    snprintf(buf, 16, "t%d", ++temp_count);
    return buf;
}

static char *new_label(void) {
    char *buf = malloc(16);
    snprintf(buf, 16, "L%d", ++label_count);
    return buf;
}

/* Returns a heap-allocated string naming the place (temp, identifier,
 * or literal) holding the value of expression 'e'. Emits TAC lines
 * for any sub-computation needed along the way. */
static char *gen_expr(ASTNode *e) {
    char *buf;
    switch (e->type) {
        case N_INT_LIT:
            buf = malloc(32);
            snprintf(buf, 32, "%ld", e->ival);
            return buf;
        case N_FLOAT_LIT:
            buf = malloc(32);
            snprintf(buf, 32, "%g", e->fval);
            return buf;
        case N_BOOL_LIT:
            return strdup(e->bval ? "true" : "false");
        case N_IDENT:
            return strdup(e->name);
        case N_UNOP: {
            char *operand = gen_expr(e->left);
            char *t = new_temp();
            printf("%s = %s%s\n", t, e->op, operand);
            free(operand);
            return t;
        }
        case N_BINOP: {
            char *l = gen_expr(e->left);
            char *r = gen_expr(e->right);
            char *t = new_temp();
            printf("%s = %s %s %s\n", t, l, e->op, r);
            free(l);
            free(r);
            return t;
        }
        default:
            return strdup("?");
    }
}

static void gen_stmt(ASTNode *s) {
    if (!s) return;
    switch (s->type) {
        case N_VARDECL:
            /* declarations produce no TAC by themselves */
            break;
        case N_ASSIGN: {
            char *r = gen_expr(s->rhs);
            printf("%s = %s\n", s->name, r);
            free(r);
            break;
        }
        case N_IF: {
            char *cond = gen_expr(s->cond);
            char *Ltrue = new_label();
            if (s->else_branch) {
                char *Lelse = new_label();
                char *Lend = new_label();
                printf("if %s == false goto %s\n", cond, Lelse);
                for (int i = 0; i < s->then_branch->child_count; i++) gen_stmt(s->then_branch->children[i]);
                printf("goto %s\n", Lend);
                printf("%s:\n", Lelse);
                for (int i = 0; i < s->else_branch->child_count; i++) gen_stmt(s->else_branch->children[i]);
                printf("%s:\n", Lend);
                free(Lelse); free(Lend);
            } else {
                printf("if %s == false goto %s\n", cond, Ltrue);
                for (int i = 0; i < s->then_branch->child_count; i++) gen_stmt(s->then_branch->children[i]);
                printf("%s:\n", Ltrue);
            }
            free(cond); free(Ltrue);
            break;
        }
        case N_WHILE: {
            char *Lstart = new_label();
            char *Lend = new_label();
            printf("%s:\n", Lstart);
            char *cond = gen_expr(s->cond);
            printf("if %s == false goto %s\n", cond, Lend);
            for (int i = 0; i < s->then_branch->child_count; i++) gen_stmt(s->then_branch->children[i]);
            printf("goto %s\n", Lstart);
            printf("%s:\n", Lend);
            free(cond); free(Lstart); free(Lend);
            break;
        }
        case N_PRINT: {
            char *v = gen_expr(s->expr);
            printf("print %s\n", v);
            free(v);
            break;
        }
        case N_BLOCK:
            for (int i = 0; i < s->child_count; i++) gen_stmt(s->children[i]);
            break;
        default:
            break;
    }
}

void codegen_generate(ASTNode *program) {
    temp_count = 0;
    label_count = 0;
    for (int i = 0; i < program->child_count; i++) gen_stmt(program->children[i]);
}
