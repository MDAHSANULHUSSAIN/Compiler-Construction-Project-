CC = gcc
CFLAGS = -Wall -Wno-unused-function -g
BIN = minicompiler

SRC_DIR = src
PARSER_DIR = $(SRC_DIR)/parser
LEXER_DIR = $(SRC_DIR)/lexer

.PHONY: all clean

all: $(BIN)

# Bison: generates parser.tab.c and parser.tab.h
$(PARSER_DIR)/parser.tab.c $(PARSER_DIR)/parser.tab.h: $(PARSER_DIR)/parser.y
	bison -d -o $(PARSER_DIR)/parser.tab.c $(PARSER_DIR)/parser.y

# Flex: generates lex.yy.c (depends on parser.tab.h for token defs)
$(LEXER_DIR)/lex.yy.c: $(LEXER_DIR)/lexer.l $(PARSER_DIR)/parser.tab.h
	flex -o $(LEXER_DIR)/lex.yy.c $(LEXER_DIR)/lexer.l

$(BIN): $(PARSER_DIR)/parser.tab.c $(LEXER_DIR)/lex.yy.c \
        $(SRC_DIR)/ast/ast.c $(SRC_DIR)/symbol_table/symtab.c \
        $(SRC_DIR)/semantic/semantic.c $(SRC_DIR)/codegen/codegen.c \
        $(SRC_DIR)/main.c
	$(CC) $(CFLAGS) -o $(BIN) \
        $(PARSER_DIR)/parser.tab.c $(LEXER_DIR)/lex.yy.c \
        $(SRC_DIR)/ast/ast.c $(SRC_DIR)/symbol_table/symtab.c \
        $(SRC_DIR)/semantic/semantic.c $(SRC_DIR)/codegen/codegen.c \
        $(SRC_DIR)/main.c -lfl

clean:
	rm -f $(BIN) $(PARSER_DIR)/parser.tab.c $(PARSER_DIR)/parser.tab.h \
          $(PARSER_DIR)/parser.output $(LEXER_DIR)/lex.yy.c
