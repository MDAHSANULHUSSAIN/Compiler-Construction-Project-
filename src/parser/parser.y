/* ============================================================
 * parser.y - Bison grammar for the custom mini language.
 * Builds an Abstract Syntax Tree (AST) as it parses.
 * ============================================================ */
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../ast/ast.h"

extern int yylineno;
extern int yylex(void);
extern char *yytext;
extern FILE *yyin;

int syntax_error_count = 0;
ASTNode *g_program = NULL;

void yyerror(const char *msg);
%}

%union {
    long ival;
    double fval;
    int bval;
    char *sval;
    struct ASTNode *node;
}

%token <sval> ID
%token <ival> INT_LIT
%token <fval> FLOAT_LIT
%token <bval> BOOL_LIT

%token KW_INT KW_FLOAT KW_BOOL KW_IF KW_ELSE KW_WHILE KW_PRINT
%token LBRACE RBRACE LPAREN RPAREN SEMI ASSIGN
%token PLUS MINUS STAR SLASH PERCENT
%token LT GT LE GE EQ NE
%token AND OR NOT

%type <node> program stmt_list stmt block
%type <node> decl_stmt assign_stmt if_stmt while_stmt print_stmt
%type <node> expr

/* Precedence, lowest to highest */
%right ASSIGN
%left OR
%left AND
%right NOT
%nonassoc LT GT LE GE EQ NE
%left PLUS MINUS
%left STAR SLASH PERCENT
%right UMINUS

%start program

%%

program:
      stmt_list {
          g_program = ast_program();
          for (int i = 0; i < $1->child_count; i++) ast_add_child(g_program, $1->children[i]);
          free($1->children);
          free($1);
          $$ = g_program;
      }
    ;

stmt_list:
      /* empty */ { $$ = ast_block(yylineno); }
    | stmt_list stmt { ast_add_child($1, $2); $$ = $1; }
    | stmt_list error SEMI {
          /* basic error recovery: skip to next semicolon and continue */
          $$ = $1;
      }
    ;

stmt:
      decl_stmt
    | assign_stmt
    | if_stmt
    | while_stmt
    | print_stmt
    | block
    ;

block:
      LBRACE stmt_list RBRACE { $$ = $2; }
    ;

decl_stmt:
      KW_INT ID SEMI   { $$ = ast_vardecl(T_INT, $2, yylineno); free($2); }
    | KW_FLOAT ID SEMI { $$ = ast_vardecl(T_FLOAT, $2, yylineno); free($2); }
    | KW_BOOL ID SEMI  { $$ = ast_vardecl(T_BOOL, $2, yylineno); free($2); }
    ;

assign_stmt:
      ID ASSIGN expr SEMI { $$ = ast_assign($1, $3, yylineno); free($1); }
    ;

if_stmt:
      KW_IF LPAREN expr RPAREN block KW_ELSE block { $$ = ast_if($3, $5, $7, yylineno); }
    | KW_IF LPAREN expr RPAREN block                { $$ = ast_if($3, $5, NULL, yylineno); }
    ;

while_stmt:
      KW_WHILE LPAREN expr RPAREN block { $$ = ast_while($3, $5, yylineno); }
    ;

print_stmt:
      KW_PRINT expr SEMI { $$ = ast_print($2, yylineno); }
    ;

expr:
      expr PLUS expr    { $$ = ast_binop("+", $1, $3, yylineno); }
    | expr MINUS expr   { $$ = ast_binop("-", $1, $3, yylineno); }
    | expr STAR expr    { $$ = ast_binop("*", $1, $3, yylineno); }
    | expr SLASH expr   { $$ = ast_binop("/", $1, $3, yylineno); }
    | expr PERCENT expr { $$ = ast_binop("%", $1, $3, yylineno); }
    | expr LT expr      { $$ = ast_binop("<", $1, $3, yylineno); }
    | expr GT expr      { $$ = ast_binop(">", $1, $3, yylineno); }
    | expr LE expr      { $$ = ast_binop("<=", $1, $3, yylineno); }
    | expr GE expr      { $$ = ast_binop(">=", $1, $3, yylineno); }
    | expr EQ expr      { $$ = ast_binop("==", $1, $3, yylineno); }
    | expr NE expr      { $$ = ast_binop("!=", $1, $3, yylineno); }
    | expr AND expr     { $$ = ast_binop("&&", $1, $3, yylineno); }
    | expr OR expr      { $$ = ast_binop("||", $1, $3, yylineno); }
    | NOT expr          { $$ = ast_unop("!", $2, yylineno); }
    | MINUS expr %prec UMINUS { $$ = ast_unop("-", $2, yylineno); }
    | LPAREN expr RPAREN { $$ = $2; }
    | ID                { $$ = ast_ident($1, yylineno); free($1); }
    | INT_LIT            { $$ = ast_int($1, yylineno); }
    | FLOAT_LIT           { $$ = ast_float($1, yylineno); }
    | BOOL_LIT             { $$ = ast_bool($1, yylineno); }
    ;

%%

void yyerror(const char *msg) {
    syntax_error_count++;
    fprintf(stderr, "Syntax Error at line %d: %s (near token '%s')\n", yylineno, msg, yytext);
}
