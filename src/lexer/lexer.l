/* ============================================================
 * lexer.l - Flex specification for the custom mini language.
 * Recognizes keywords, identifiers, constants, operators,
 * delimiters, comments (discarded) and reports invalid tokens
 * as lexical errors with line numbers, without halting.
 * ============================================================ */
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../ast/ast.h"
#include "../parser/parser.tab.h"

extern void lex_error(int line, const char *text);
%}

%option yylineno
%option noinput
%option nounput

DIGIT     [0-9]
LETTER    [a-zA-Z_]
ID        {LETTER}({LETTER}|{DIGIT})*
INT       {DIGIT}+
FLOAT     {DIGIT}+"."{DIGIT}+

%%

"//".*                              { /* single-line comment: discard */ }
"/*"([^*]|\*+[^*/])*\*+"/"          { /* block comment: discard */ }

"int"     { return KW_INT; }
"float"   { return KW_FLOAT; }
"bool"    { return KW_BOOL; }
"if"      { return KW_IF; }
"else"    { return KW_ELSE; }
"while"   { return KW_WHILE; }
"print"   { return KW_PRINT; }
"true"    { yylval.bval = 1; return BOOL_LIT; }
"false"   { yylval.bval = 0; return BOOL_LIT; }

{FLOAT}   { yylval.fval = atof(yytext); return FLOAT_LIT; }
{INT}     { yylval.ival = atol(yytext); return INT_LIT; }
{ID}      { yylval.sval = strdup(yytext); return ID; }

"&&"      { return AND; }
"||"      { return OR; }
"!"       { return NOT; }
"<="      { return LE; }
">="      { return GE; }
"=="      { return EQ; }
"!="      { return NE; }
"<"       { return LT; }
">"       { return GT; }
"="       { return ASSIGN; }
"+"       { return PLUS; }
"-"       { return MINUS; }
"*"       { return STAR; }
"/"       { return SLASH; }
"%"       { return PERCENT; }

"{"       { return LBRACE; }
"}"       { return RBRACE; }
"("       { return LPAREN; }
")"       { return RPAREN; }
";"       { return SEMI; }

[ \t\r]+  { /* whitespace: discard */ }
\n        { /* newline: discard, yylineno auto-increments */ }

.         { lex_error(yylineno, yytext); /* invalid token: report & skip */ }

%%

int yywrap(void) { return 1; }
