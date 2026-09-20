CC = gcc
CFLAGS = -Wall -Wextra -Isrc

all: compilador

compilador: src/lex.yy.c src/parser.tab.c
	$(CC) $(CFLAGS) -o $@ $^

src/lex.yy.c: src/scanner.l src/parser.tab.h
	flex -o $@ $<

src/parser.tab.c src/parser.tab.h: src/parser.y
	bison -d -o src/parser.tab.c $<

test: compilador
	python3 tests/run_tests.py || python tests/run_tests.py

clean:
	rm -f compilador src/lex.yy.c src/parser.tab.c src/parser.tab.h
