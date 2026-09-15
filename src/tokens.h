#ifndef TOKENS_H
#define TOKENS_H

/*
 * Vocabulario de tokens do scanner.
 *
 * Os codigos (TokenType) sao os gerados pelo Bison em parser.tab.h, de forma
 * que o scanner e o parser compartilhem a mesma numeracao.
 */
#include "parser.tab.h"

typedef enum yytokentype TokenType;

/*
 * Classifica um lexema que casa com [a-zA-Z_][a-zA-Z0-9_]*.
 * Retorna o TokenType da palavra reservada correspondente ou IDENTIFIER
 * quando o lexema nao consta na tabela de palavras-chave.
 */
TokenType keyword_lookup(const char *lexeme);

/* Nome legivel do token, para depuracao e para o modo --tokens. */
const char *token_name(TokenType token);

#endif /* TOKENS_H */
