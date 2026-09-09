#!/usr/bin/env python3
"""
gui.py - Graphical front-end for the Mini Language Compiler.

Wraps the compiled `minicompiler` C binary (built via Flex + Bison,
see the top-level Makefile) and presents every stage of the pipeline
through a desktop GUI, each stage on its own clearly labelled tab:

    1. Lexical Analyzer   (Token stream)
    2. Syntax Analyzer    (Parsing / grammar check)
    3. AST Construction   (Abstract Syntax Tree)
    4. Symbol Table       (Declared identifiers, nested scopes)
    5. Semantic Analyzer  (Type checking, scope rules)
    6. Intermediate Code  (Three Address Code / TAC)

Buttons:
    - "Enter your code"   : text box to type/paste source code
    - "Tokenize"           : runs only the lexer, fills the Lexical tab
    - "Analyze"             : runs the full pipeline (parser, AST,
                               symbol table, semantic analysis, TAC)
                               and fills every tab in one go
    - "Clear"               : resets the source box and every tab
    - "Show Symbol Table"  : runs Analyze (if needed) and jumps
                               straight to the Symbol Table tab

Run with:  python3 gui/gui.py
(the compiler must already be built: run `make` in the project root first)
"""

import os
import subprocess
import tkinter as tk
from tkinter import ttk, scrolledtext

# ---------------------------------------------------------------- theme ----
BG_MAIN     = "#A9D6CB"   # mint / teal background
BG_PANEL    = "#6C82D6"   # periwinkle blue header / button panels
BG_PANEL_LT = "#8698DE"   # lighter periwinkle for inactive tabs
BG_TEXTAREA = "#D9EEE6"   # pale mint text areas
FG_PANEL    = "#0B0B0B"
FG_OK       = "#0B5E2E"
FG_ERR      = "#8A1414"
FONT_HEADER = ("Segoe UI", 15, "bold", "underline")
FONT_BTN    = ("Segoe UI", 13, "bold", "underline")
FONT_TAB    = ("Segoe UI", 11, "bold")
FONT_MONO   = ("Consolas", 11)

HERE = os.path.dirname(os.path.abspath(__file__))
COMPILER_BIN = os.path.join(HERE, "..", "minicompiler")

# Order mirrors the compiler-construction pipeline (and the project checklist):
#   Lexical -> Syntax -> AST -> Symbol Table -> Semantic -> Intermediate Code
TABS = [
    ("lexical",  "1. Lexical Analyzer"),
    ("syntax",   "2. Syntax Analyzer"),
    ("ast",      "3. AST Construction"),
    ("symtab",   "4. Symbol Table"),
    ("semantic", "5. Semantic Analyzer"),
    ("tac",      "6. Intermediate Code (TAC)"),
]


def run_compiler(mode: str, source: str) -> tuple:
    """Runs the compiled binary in the given mode, feeding `source` on
    stdin. Returns (stdout_text, stderr_text) separately so callers can
    tell parser/runtime diagnostics apart from normal pipeline output."""
    if not os.path.exists(COMPILER_BIN):
        return ("", "ERROR: compiler binary not found at '{}'.\n"
                     "Run 'make' in the project root first.".format(COMPILER_BIN))
    try:
        proc = subprocess.run(
            [COMPILER_BIN, mode],
            input=source,
            capture_output=True,
            text=True,
            timeout=10,
        )
        return proc.stdout, proc.stderr
    except Exception as exc:
        return "", f"ERROR running compiler: {exc}"


class CompilerGUI:
    def __init__(self, root):
        self.root = root
        root.title("Mini Language Compiler")
        root.configure(bg=BG_MAIN)
        root.geometry("1360x860")
        root.minsize(1040, 680)

        self._configure_ttk_style()

        # ---------- header row ----------
        top = tk.Frame(root, bg=BG_MAIN)
        top.pack(fill="x", padx=20, pady=(20, 10))
        top.columnconfigure(0, weight=1)
        top.columnconfigure(1, weight=1)

        left_header = tk.Label(top, text="Enter your code:", font=FONT_HEADER,
                                bg=BG_PANEL, fg=FG_PANEL, height=3)
        left_header.grid(row=0, column=0, sticky="nsew", padx=(0, 10))

        right_header = tk.Label(top, text="Compiler Pipeline Output", font=FONT_HEADER,
                                 bg=BG_PANEL, fg=FG_PANEL, height=3)
        right_header.grid(row=0, column=1, sticky="nsew", padx=(10, 0))

        # ---------- main area: code editor (left) + tabbed output (right) ----------
        mid = tk.Frame(root, bg=BG_MAIN)
        mid.pack(fill="both", expand=True, padx=20, pady=10)
        mid.columnconfigure(0, weight=1)
        mid.columnconfigure(1, weight=1)
        mid.rowconfigure(0, weight=1)

        self.code_text = scrolledtext.ScrolledText(
            mid, font=FONT_MONO, bg=BG_TEXTAREA, fg="#111111",
            insertbackground="#111111", wrap="none", undo=True)
        self.code_text.grid(row=0, column=0, sticky="nsew", padx=(0, 10))
        self.code_text.insert("1.0", SAMPLE_PROGRAM)

        # Notebook holding one tab per pipeline stage.
        self.notebook = ttk.Notebook(mid, style="Compiler.TNotebook")
        self.notebook.grid(row=0, column=1, sticky="nsew", padx=(10, 0))

        self.tab_text = {}
        for key, label in TABS:
            frame = tk.Frame(self.notebook, bg=BG_TEXTAREA)
            self.notebook.add(frame, text=label)
            txt = scrolledtext.ScrolledText(
                frame, font=FONT_MONO, bg=BG_TEXTAREA, fg="#111111",
                wrap="none", state="disabled", borderwidth=0, highlightthickness=0)
            txt.pack(fill="both", expand=True, padx=6, pady=6)
            self.tab_text[key] = txt

        self._set_tab_placeholder("lexical", "Click \"Tokenize\" or \"Analyze\" to see the token stream here.")
        self._set_tab_placeholder("syntax", "Click \"Analyze\" to run the parser and check the grammar.")
        self._set_tab_placeholder("ast", "Click \"Analyze\" to build and display the Abstract Syntax Tree.")
        self._set_tab_placeholder("symtab", "Click \"Analyze\" (or \"Show Symbol Table\") to see declared identifiers.")
        self._set_tab_placeholder("semantic", "Click \"Analyze\" to run type checking and scope-rule validation.")
        self._set_tab_placeholder("tac", "Click \"Analyze\" to generate Three Address Code.")

        # ---------- bottom button row ----------
        bottom = tk.Frame(root, bg=BG_MAIN)
        bottom.pack(fill="x", padx=20, pady=(10, 20))
        for c in range(4):
            bottom.columnconfigure(c, weight=1)

        tk.Button(bottom, text="Tokenize", font=FONT_BTN, bg=BG_PANEL,
                  fg=FG_PANEL, height=2, command=self.on_tokenize
                  ).grid(row=0, column=0, sticky="nsew", padx=8)
        tk.Button(bottom, text="Analyze", font=FONT_BTN, bg=BG_PANEL,
                  fg=FG_PANEL, height=2, command=self.on_analyze
                  ).grid(row=0, column=1, sticky="nsew", padx=8)
        tk.Button(bottom, text="Show Symbol Table", font=FONT_BTN, bg=BG_PANEL,
                  fg=FG_PANEL, height=2, command=self.on_symbol_table
                  ).grid(row=0, column=2, sticky="nsew", padx=8)
        tk.Button(bottom, text="Clear", font=FONT_BTN, bg=BG_PANEL,
                  fg=FG_PANEL, height=2, command=self.on_clear
                  ).grid(row=0, column=3, sticky="nsew", padx=8)

        # status bar
        self.status = tk.Label(root, text="Ready.", bg=BG_MAIN, fg="#08331f",
                                font=("Segoe UI", 10), anchor="w")
        self.status.pack(fill="x", padx=22, pady=(0, 8))

    # ------------------------------------------------------------- style ----
    def _configure_ttk_style(self):
        style = ttk.Style()
        try:
            style.theme_use("clam")
        except tk.TclError:
            pass
        style.configure("Compiler.TNotebook", background=BG_MAIN, borderwidth=0)
        style.configure("Compiler.TNotebook.Tab", background=BG_PANEL_LT,
                         foreground=FG_PANEL, font=FONT_TAB, padding=(10, 8))
        style.map("Compiler.TNotebook.Tab",
                   background=[("selected", BG_PANEL)],
                   foreground=[("selected", FG_PANEL)])

    # ---------------------------------------------------------- helpers ----
    def _set_tab(self, key: str, text: str):
        widget = self.tab_text[key]
        widget.configure(state="normal")
        widget.delete("1.0", "end")
        widget.insert("1.0", text)
        widget.configure(state="disabled")

    def _set_tab_placeholder(self, key: str, text: str):
        self._set_tab(key, text)

    def _source(self) -> str:
        return self.code_text.get("1.0", "end")

    def _select_tab(self, key: str):
        keys = [k for k, _ in TABS]
        self.notebook.select(keys.index(key))

    # -------------------------------------------------- lexical (tokens) ----
    def _format_tokens(self, stdout: str, stderr: str) -> str:
        lines = []
        header = f"{'Token':<20} | {'Type':<20} | Line"
        lines.append(header)
        lines.append("-" * len(header))
        error_lines = []
        for line in stdout.splitlines():
            if line.startswith("TOKEN|"):
                _, tok, cat, ln = line.split("|", 3)
                lines.append(f"{tok:<20} | {cat:<20} | {ln}")
            elif line.startswith("LEXERR|"):
                _, ln, msg = line.split("|", 2)
                error_lines.append(f"Line {ln}: Lexical Error - {msg}")
            elif line.strip():
                error_lines.append(line)
        if stderr.strip():
            error_lines.append(stderr.strip())
        if error_lines:
            lines.append("")
            lines.append("=== Lexical Errors ===")
            lines.extend(error_lines)
        else:
            lines.append("")
            lines.append("(No lexical errors.)")
        return "\n".join(lines)

    def on_tokenize(self):
        self.status.config(text="Running lexical analysis...")
        self.root.update_idletasks()
        stdout, stderr = run_compiler("tokenize", self._source())
        self._set_tab("lexical", self._format_tokens(stdout, stderr))
        self._select_tab("lexical")
        self.status.config(text="Tokenize complete.")

    # --------------------------------------------------- full pipeline ----
    def _parse_full_output(self, stdout: str, stderr: str) -> dict:
        """Splits the driver's `full` output into per-stage line lists."""
        sections = {"AST": [], "SYMBOL TABLE": [], "SUMMARY": [], "TAC": []}
        semerr_lines = []
        current = None
        fatal = None
        for line in stdout.splitlines():
            if line.startswith("=== ") and line.endswith(" ==="):
                current = line.strip("= ").strip()
                continue
            if line.startswith("FATAL|"):
                fatal = line.split("|", 1)[1]
                continue
            if line.startswith("SEMERR|"):
                _, ln, msg = line.split("|", 2)
                semerr_lines.append(f"Line {ln}: Semantic Error - {msg}")
                continue
            if current and current in sections:
                sections[current].append(line)

        synerr_lines = [l.strip() for l in stderr.splitlines()
                         if "Syntax Error" in l]

        syntax_error_count = 0
        semantic_error_count = 0
        for line in sections["SUMMARY"]:
            if line.startswith("Syntax errors:"):
                syntax_error_count = int(line.split(":")[1].strip())
            elif line.startswith("Semantic errors:"):
                semantic_error_count = int(line.split(":")[1].strip())

        return {
            "fatal": fatal,
            "ast": sections["AST"],
            "symtab": sections["SYMBOL TABLE"],
            "tac": sections["TAC"],
            "synerr": synerr_lines,
            "semerr": semerr_lines,
            "syntax_error_count": syntax_error_count,
            "semantic_error_count": semantic_error_count,
        }

    def on_analyze(self):
        self.status.config(text="Running full pipeline (parse -> AST -> symbol table -> semantic -> TAC)...")
        self.root.update_idletasks()

        # Lexical tab (independent lexer pass, same as Tokenize).
        tok_stdout, tok_stderr = run_compiler("tokenize", self._source())
        self._set_tab("lexical", self._format_tokens(tok_stdout, tok_stderr))

        # Full pipeline pass.
        stdout, stderr = run_compiler("full", self._source())
        data = self._parse_full_output(stdout, stderr)

        if data["fatal"]:
            msg = f"Parsing failed — no usable syntax tree.\n\n{data['fatal']}\n\n" \
                  + "\n".join(data["synerr"])
            for key in ("syntax", "ast", "symtab", "semantic", "tac"):
                self._set_tab(key, msg)
            self.status.config(text="Analysis failed: parser could not recover.")
            return

        # ---- Syntax tab ----
        if data["synerr"]:
            syntax_text = "\n".join(data["synerr"])
            syntax_text += f"\n\nTotal syntax errors: {data['syntax_error_count']}"
            syntax_text += ("\n\nNote: the parser recovers after a syntax error and "
                             "continues parsing the next statement, so the AST below "
                             "may still contain the surrounding, successfully-parsed code.")
        else:
            syntax_text = "No syntax errors. The input conforms to the language grammar."
        self._set_tab("syntax", syntax_text)

        # ---- AST tab ----
        ast_text = "\n".join(data["ast"]) if data["ast"] else "(empty)"
        self._set_tab("ast", ast_text)

        # ---- Symbol Table tab ----
        symtab_text = "\n".join(data["symtab"]) if data["symtab"] else "(empty)"
        self._set_tab("symtab", symtab_text)

        # ---- Semantic tab ----
        if data["semerr"]:
            semantic_text = "\n".join(data["semerr"])
            semantic_text += f"\n\nTotal semantic errors: {data['semantic_error_count']}"
        else:
            semantic_text = "No semantic errors. All type-checking and scope rules passed."
        self._set_tab("semantic", semantic_text)

        # ---- TAC tab ----
        tac_text = "\n".join(data["tac"]) if data["tac"] else "(empty)"
        self._set_tab("tac", tac_text)

        errors_total = data["syntax_error_count"] + data["semantic_error_count"]
        if errors_total == 0:
            self.status.config(text="Analysis complete — no errors. TAC generated successfully.")
        else:
            self.status.config(
                text=f"Analysis complete — {data['syntax_error_count']} syntax error(s), "
                     f"{data['semantic_error_count']} semantic error(s).")

    def on_clear(self):
        self.code_text.delete("1.0", "end")
        for key, _ in TABS:
            self._set_tab(key, "")
        self.status.config(text="Cleared.")

    def on_symbol_table(self):
        self.on_analyze()
        self._select_tab("symtab")


SAMPLE_PROGRAM = """int x;
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
"""


def main():
    root = tk.Tk()
    CompilerGUI(root)
    root.mainloop()


if __name__ == "__main__":
    main()
