CC = gcc
CFLAGS = -Wall -Wextra -Isrc

all: compilador

compilador: src/lex.yy.c src/parser.tab.c src/tokens.c
	$(CC) $(CFLAGS) -o $@ $^

src/lex.yy.c: src/scanner.l src/parser.tab.h src/tokens.h
	flex -o $@ $<

src/parser.tab.c src/parser.tab.h: src/parser.y
	bison -d -o src/parser.tab.c $<

clean:
	rm -f compilador src/lex.yy.c src/parser.tab.c src/parser.tab.h
