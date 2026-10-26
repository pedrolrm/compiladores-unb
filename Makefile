CC = gcc
CFLAGS = -Wall -Wextra -Isrc

SRCS = src/ast.c src/symtab.c src/semantic.c

all: compilador

compilador: src/lex.yy.c src/parser.tab.c $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^

src/lex.yy.c: src/scanner.l src/parser.tab.h
	flex -o $@ $<

src/parser.tab.c src/parser.tab.h: src/parser.y src/ast.h
	bison -d -o src/parser.tab.c $<

test: compilador
	python3 tests/run_tests.py || python tests/run_tests.py
	python3 tests/test_ast.py || python tests/test_ast.py

clean:
	rm -f compilador src/lex.yy.c src/parser.tab.c src/parser.tab.h
