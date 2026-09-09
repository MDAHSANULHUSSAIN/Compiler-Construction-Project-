# Mini Language Compiler — Compiler Construction Lab Project

A compiler front-end (lexer → parser → AST → symbol table → semantic
analysis → three-address-code generation) for the custom mini language
specified in the project manual, built with **Flex** and **Bison**,
plus a **Tkinter GUI** front-end.



## 1. Project Structure

```
project-root/
├── src/
│   ├── lexer/lexer.l          Flex specification (tokens, comments, whitespace, lexical errors)
│   ├── parser/parser.y        Bison grammar (CFG, AST construction, error recovery)
│   ├── ast/                   AST node definitions + printer
│   ├── symbol_table/          Nested-scope symbol table
│   ├── semantic/              Semantic analyzer (type checking, scope rules)
│   ├── codegen/                Three Address Code (TAC) generator
│   └── main.c                 Driver (tokenize / full pipeline modes)
├── gui/gui.py                 Tkinter GUI front-end
├── examples/                  Sample valid programs
├── tests/                     Valid + invalid test programs and expected output
├── docs/                      (put your project report / diagrams here)
├── Makefile
└── README.md
```

## 2. Requirements

- Linux (Ubuntu/Debian recommended)
- `flex`, `bison`, `gcc`, `make`
- `python3` with `tkinter` (for the GUI)

Install prerequisites (Ubuntu/Debian):
```bash
sudo apt-get update
sudo apt-get install -y flex bison gcc make python3 python3-tk
```

## 3. Build Instructions

From the project root:
```bash
make
```
This runs Bison on `src/parser/parser.y` (generating `parser.tab.c`/`.h`),
Flex on `src/lexer/lexer.l` (generating `lex.yy.c`), and compiles
everything into a single binary: `./minicompiler`.

To clean generated files:
```bash
make clean
```

## 4. Execution Instructions

### 4.1 Command line
```bash
# Print just the token stream
./minicompiler tokenize < examples/valid_arith.mc

# Run the full pipeline: AST, symbol table, semantic checks, TAC
./minicompiler full < examples/valid_arith.mc
```

### 4.2 GUI
```bash
python3 gui/gui.py
```
The GUI (see screenshot in `docs/`) provides:
- **Enter your code** — a text box to type/paste source code
- **Tokenize** — runs only the lexer and fills the *Lexical Analyzer* tab
  with the raw token stream (Token | Type | Line)
- **Analyze** — runs the full pipeline (lex → parse → AST → symbol
  table → semantic analysis → TAC) and fills every tab in one go
- **Clear** — resets the source box and every output tab
- **Show Symbol Table** — runs Analyze (if needed) and jumps straight
  to the *Symbol Table* tab

The output side is a tabbed notebook with one tab per compiler stage,
so each stage of the pipeline is independently visible:
1. **Lexical Analyzer** — token stream, with any invalid-token errors
2. **Syntax Analyzer** — parse result: "no syntax errors" or the
   `Syntax Error at line N: ...` diagnostics
3. **AST Construction** — the printed Abstract Syntax Tree
4. **Symbol Table** — declared identifiers with type/scope/line
5. **Semantic Analyzer** — type-checking / scope-rule diagnostics, or
   confirmation that none were found
6. **Intermediate Code (TAC)** — the generated Three Address Code
   (skipped with a note if syntax/semantic errors were found)

> The GUI is a thin wrapper: it calls the compiled `minicompiler`
> binary via subprocess and formats its output. Build the binary
> (`make`) before launching the GUI.

## 5. Language Overview

See Section 5 of the project manual for the authoritative specification.
Types: `int`, `float`, `bool`. Statements: declarations, assignment,
`if`/`if-else`, `while`, `print`, nested blocks `{ }`. Operators:
arithmetic `+ - * / %`, relational `< > <= >= == !=`, logical `&& || !`.

Example program (`examples/valid_leap_year_style.mc`):
```c
int x;
int y;
bool flag;
x = 10;
y = 0;
flag = true;
while (x > 0) {
    y = y + x;
    x = x - 1;
}
if (flag == true) {
    print y;
} else {
    print x;
}
```

## 6. Testing

`tests/` contains one program per required test category (see Section
15 of the manual), each with its captured expected output under
`tests/expected_output/`:

| File | Demonstrates |
|------|---------------|
| `valid_nested_scope.mc` | Successful compilation through TAC, nested scoping |
| `lexical_error.mc` | Invalid token (`@`) reported with line number |
| `syntax_error.mc` | Grammar violation, reported + recovered from |
| `semantic_undeclared.mc` | Undeclared variable use |
| `semantic_redeclare.mc` | Redeclaration in same scope |
| `semantic_scope_violation.mc` | Variable used outside its declaring block |
| `semantic_type_mismatch.mc` | `bool = int + float` type mismatch |
| `semantic_invalid_assignment.mc` | Assigning `bool` to an `int` variable |

Regenerate expected output any time with:
```bash
for f in tests/*.mc examples/*.mc; do
  base=$(basename "$f" .mc)
  ./minicompiler full     < "$f" > "tests/expected_output/${base}.full.out"
  ./minicompiler tokenize < "$f" > "tests/expected_output/${base}.tokens.out"
done
```

## 7. Notes on Design

- **Lexer**: keyword patterns are matched before the generic identifier
  pattern (Flex longest-match + rule order), so keywords are never
  misclassified as identifiers. Invalid characters are reported via
  `LEXERR|<line>|<message>` and skipped — the lexer never halts.
- **Parser**: uses Bison's `error` token with a `stmt_list: stmt_list
  error SEMI` recovery rule, so a single syntax error is reported and
  parsing resumes at the next statement instead of aborting outright.
- **Symbol Table**: implemented as a stack of scopes (linked lists),
  plus a flat history log of every entry ever declared, so a full
  symbol table (including closed inner-block variables) can be
  printed for the "Show Symbol Table" GUI action.
- **Semantic Analyzer**: a recursive AST walk that declares variables
  into the current scope, pushes/pops a scope per block, and computes
  a `resolved_type` for every expression node while checking each
  rule in Section 4.5 of the manual.
- **Code Generator**: a straightforward recursive TAC emitter using
  temporaries (`t1, t2, ...`) and labels (`L1, L2, ...`) for `if`/
  `while` control flow, matching the format illustrated in Section
  4.6 of the manual. TAC generation is skipped if any syntax or
  semantic error was found.

## 8. Out of Scope

Per Section 6 of the manual: no machine code, assembly, register
allocation, linking, optimization, or executable generation.
