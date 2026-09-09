# Formal Context-Free Grammar (CFG)

This is the formal grammar implemented in `src/parser/parser.y` (Bison),
written in BNF. It corresponds exactly to the language specified in
Section 5 of the Project Manual.

## Notation
- `::=` means "is defined as"
- `|` separates alternative productions
- Terminals (actual tokens) are shown in `code font`
- Non-terminals are shown in *italics*

## Grammar

```
program        ::= stmt_list

stmt_list      ::= /* empty */
                 | stmt_list stmt

stmt           ::= decl_stmt
                 | assign_stmt
                 | if_stmt
                 | while_stmt
                 | print_stmt
                 | block

block          ::= '{' stmt_list '}'

decl_stmt      ::= 'int' ID ';'
                 | 'float' ID ';'
                 | 'bool' ID ';'

assign_stmt    ::= ID '=' expr ';'

if_stmt        ::= 'if' '(' expr ')' block 'else' block
                 | 'if' '(' expr ')' block

while_stmt     ::= 'while' '(' expr ')' block

print_stmt     ::= 'print' expr ';'

expr           ::= expr '+' expr
                 | expr '-' expr
                 | expr '*' expr
                 | expr '/' expr
                 | expr '%' expr
                 | expr '<' expr
                 | expr '>' expr
                 | expr '<=' expr
                 | expr '>=' expr
                 | expr '==' expr
                 | expr '!=' expr
                 | expr '&&' expr
                 | expr '||' expr
                 | '!' expr
                 | '-' expr            %prec UMINUS
                 | '(' expr ')'
                 | ID
                 | INT_LIT
                 | FLOAT_LIT
                 | BOOL_LIT
```

## Operator Precedence and Associativity

Listed lowest to highest precedence, exactly as declared in `parser.y`:

| Precedence (low -> high) | Operators           | Associativity |
|---------------------------|----------------------|---------------|
| 1                          | `=`                  | right         |
| 2                          | `\|\|`                | left          |
| 3                          | `&&`                 | left          |
| 4                          | `!` (unary)          | right         |
| 5                          | `<  >  <=  >=  ==  !=` | non-associative |
| 6                          | `+  -` (binary)      | left          |
| 7                          | `*  /  %`            | left          |
| 8                          | `-` (unary, UMINUS)  | right         |

This ordering ensures, for example, that `a + b * c` parses as
`a + (b * c)`, and `a == b && c == d` parses as `(a == b) && (c == d)`.

## Error Recovery

```
stmt_list ::= stmt_list error ';'
```

When the parser encounters a token sequence that does not match any
valid statement, Bison's `error` token causes it to discard input up
to the next `;` and resume parsing the following statement, rather
than aborting the entire compilation on the first syntax error.

## Notes

- Grouping parentheses `( )` and statement-terminating semicolons `;`
  are consumed by the grammar but are **not** represented as nodes in
  the AST (see `src/ast/`) — only the meaningful structure survives,
  per Section 4.3 of the manual.
- This file documents the grammar as implemented in code. It should be
  copied into the relevant chapter (Section 12, "Parser Design") of the
  full written project report, alongside the precedence/associativity
  rationale and any shift/reduce conflicts encountered.
