# docs/

This folder holds the project's non-code deliverables, per Section 8 and
Section 11 of the Project Manual.

## Already included

- `grammar.md` — the formal CFG (Context-Free Grammar) for the language,
  derived directly from `src/parser/parser.y`. Required by Section 5
  ("you must design and document a complete, formal CFG ... in your
  project report").

## Still needed before submission (must be written by the team)

These require your group's own explanation, reasoning, and reflections —
they cannot be generated for you, since the individual viva (Section 10)
specifically checks that you understand and can explain your own work:

- [ ] **Full project report** (Section 12), covering:
  - Introduction, Objectives
  - Language Specification (can reuse `grammar.md`)
  - Compiler Architecture
  - Lexer Design, Parser Design, AST, Semantic Analysis, Symbol Table,
    Intermediate Code — explain the *design decisions*, not just what
    the code does
  - Challenges faced and how you solved them
  - Testing summary
  - Conclusion, References
- [ ] **Presentation slides** (Section 13)
- [ ] **Screenshots** of successful builds/runs and of error handling
      (Section 11) — you can capture these directly from the GUI's
      Lexical / Syntax / AST / Symbol Table / Semantic / TAC tabs
- [ ] **Compilation & execution instructions** — a short version already
      lives in the top-level `README.md`; copy/expand it here if your
      report needs it inline
- [ ] Optional: a short video demonstration (Section 11, not required)

## Tip

Section 10 (AI Usage Policy) explicitly requires every member to be able
to explain *any* part of the implementation during the individual viva —
including anything AI-assisted. Before submission, make sure each team
member can walk through: the lexer rules, the grammar, how the AST is
built, how scoping works in the symbol table, each semantic check, and
how TAC is emitted for `if`/`while`.
