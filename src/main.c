/* ============================================================
 * main.c - Compiler driver.
 *
 * Usage:
 *   minicompiler tokenize < source.mc     -> prints raw token stream
 *   minicompiler full     < source.mc     -> runs the whole pipeline:
 *        parsing -> AST -> symbol table -> semantic analysis -> TAC
 *        Output is split into clearly marked sections so that any
 *        front-end (CLI or the bundled GUI) can parse it easily.
 * ============================================================ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast/ast.h"
#include "symbol_table/symtab.h"
#include "semantic/semantic.h"
#include "codegen/codegen.h"
#include "parser/parser.tab.h"

extern int yylex(void);
extern int yylex_destroy(void);
extern int yyparse(void);
extern int yylineno;
extern char *yytext;
extern FILE *yyin;
extern int syntax_error_count;
extern ASTNode *g_program;

/* --- lexical error reporting hook, called from lexer.l --- */
void lex_error(int line, const char *text) {
    printf("LEXERR|%d|Invalid token '%s'\n", line, text);
}

static const char *category_for_token(int tok) {
    switch (tok) {
        case KW_INT: case KW_FLOAT: case KW_BOOL:
        case KW_IF: case KW_ELSE: case KW_WHILE: case KW_PRINT:
            return "Keyword";
        case ID: return "Identifier";
        case INT_LIT: return "Integer Literal";
        case FLOAT_LIT: return "Float Literal";
        case BOOL_LIT: return "Boolean Literal";
        case PLUS: case MINUS: case STAR: case SLASH: case PERCENT:
        case LT: case GT: case LE: case GE: case EQ: case NE:
        case AND: case OR: case NOT: case ASSIGN:
            return "Operator";
        case LBRACE: case RBRACE: case LPAREN: case RPAREN: case SEMI:
            return "Special Character";
        default: return "Unknown";
    }
}

static void run_tokenize(void) {
    int tok;
    while ((tok = yylex()) != 0) {
        printf("TOKEN|%s|%s|%d\n", yytext, category_for_token(tok), yylineno);
    }
}

static void run_full(void) {
    int parse_result = yyparse();
    (void)parse_result;

    if (!g_program) {
        printf("FATAL|Parsing failed with no recoverable AST.\n");
        return;
    }

    printf("=== AST ===\n");
    ast_print_tree(g_program, 0);

    SymbolTable *st = symtab_create();
    int sem_errors = semantic_analyze(g_program, st);

    printf("=== SYMBOL TABLE ===\n");
    symtab_print_all(st);

    printf("=== SUMMARY ===\n");
    printf("Syntax errors: %d\n", syntax_error_count);
    printf("Semantic errors: %d\n", sem_errors);

    printf("=== TAC ===\n");
    if (syntax_error_count == 0 && sem_errors == 0) {
        codegen_generate(g_program);
    } else {
        printf("(Intermediate code generation skipped due to earlier errors.)\n");
    }

    symtab_free(st);
    ast_free(g_program);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s [tokenize|full] < source.mc\n", argv[0]);
        return 1;
    }

    yyin = stdin;

    if (strcmp(argv[1], "tokenize") == 0) {
        run_tokenize();
    } else if (strcmp(argv[1], "full") == 0) {
        run_full();
    } else {
        fprintf(stderr, "Unknown mode '%s'. Use 'tokenize' or 'full'.\n", argv[1]);
        return 1;
    }

    return 0;
}
