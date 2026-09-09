#ifndef AST_H
#define AST_H

/* ===================== AST Node Definitions =====================
 * Every construct in the language is represented as a node in the
 * Abstract Syntax Tree. Parentheses and semicolons are dropped;
 * only meaningful structure survives.
 * =================================================================
 */

typedef enum {
    /* Program / statements */
    N_PROGRAM,
    N_VARDECL,
    N_ASSIGN,
    N_IF,
    N_WHILE,
    N_PRINT,
    N_BLOCK,

    /* Expressions */
    N_BINOP,
    N_UNOP,
    N_INT_LIT,
    N_FLOAT_LIT,
    N_BOOL_LIT,
    N_IDENT
} NodeType;

/* Data types recognized by the language */
typedef enum {
    T_INT,
    T_FLOAT,
    T_BOOL,
    T_UNKNOWN   /* used before/if type inference fails */
} DataType;

typedef struct ASTNode {
    NodeType type;
    int line;

    /* --- generic children list (used for N_PROGRAM / N_BLOCK) --- */
    struct ASTNode **children;
    int child_count;
    int child_cap;

    /* --- VarDecl --- */
    DataType decl_type;
    char *name;              /* also used by N_IDENT, N_ASSIGN (lhs name) */

    /* --- Assign --- */
    struct ASTNode *rhs;

    /* --- If / While --- */
    struct ASTNode *cond;
    struct ASTNode *then_branch;
    struct ASTNode *else_branch;  /* NULL if no else */

    /* --- Print --- */
    struct ASTNode *expr;

    /* --- BinOp / UnOp --- */
    char *op;                /* operator text, e.g. "+", "&&", "!" */
    struct ASTNode *left;
    struct ASTNode *right;    /* NULL for unary */

    /* --- Literals --- */
    long ival;
    double fval;
    int bval;

    /* resolved type after semantic analysis (for expr nodes) */
    DataType resolved_type;
} ASTNode;

ASTNode *ast_new(NodeType type, int line);
void ast_add_child(ASTNode *parent, ASTNode *child);

ASTNode *ast_program(void);
ASTNode *ast_block(int line);
ASTNode *ast_vardecl(DataType t, const char *name, int line);
ASTNode *ast_assign(const char *name, ASTNode *rhs, int line);
ASTNode *ast_if(ASTNode *cond, ASTNode *then_b, ASTNode *else_b, int line);
ASTNode *ast_while(ASTNode *cond, ASTNode *body, int line);
ASTNode *ast_print(ASTNode *expr, int line);
ASTNode *ast_binop(const char *op, ASTNode *l, ASTNode *r, int line);
ASTNode *ast_unop(const char *op, ASTNode *operand, int line);
ASTNode *ast_int(long v, int line);
ASTNode *ast_float(double v, int line);
ASTNode *ast_bool(int v, int line);
ASTNode *ast_ident(const char *name, int line);

const char *type_to_str(DataType t);
void ast_print_tree(ASTNode *node, int indent);
void ast_free(ASTNode *node);

#endif
