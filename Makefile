CC = gcc
CFLAGS = -Wall -Wextra -Isrc

SRCS = src/ast.c src/symtab.c src/semantic.c src/codegen.c src/codegen_vars.c src/codegen_globals.c src/codegen_expr.c src/codegen_loops.c src/codegen_func.c

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
	python3 tests/test_semantic.py || python tests/test_semantic.py
	python3 tests/test_codegen.py || python tests/test_codegen.py
	$(CC) $(CFLAGS) -o tests/_temp_codegen_vars tests/test_codegen_vars.c src/codegen_vars.c
	./tests/_temp_codegen_vars
	$(CC) $(CFLAGS) -o tests/_temp_codegen_globals tests/test_codegen_globals.c src/codegen_globals.c
	./tests/_temp_codegen_globals
	$(CC) $(CFLAGS) -o tests/_temp_codegen_expr tests/test_codegen_expr.c src/codegen_expr.c src/ast.c
	./tests/_temp_codegen_expr
	$(CC) $(CFLAGS) -o tests/_temp_codegen_loops tests/test_codegen_loops.c src/codegen_loops.c src/codegen_expr.c src/ast.c
	./tests/_temp_codegen_loops
	$(CC) $(CFLAGS) -o tests/_temp_codegen_func tests/test_codegen_func.c src/codegen_func.c src/codegen_loops.c src/codegen_expr.c src/ast.c
	./tests/_temp_codegen_func

clean:
	rm -f compilador src/lex.yy.c src/parser.tab.c src/parser.tab.h
