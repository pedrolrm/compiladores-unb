#ifndef SCANNER_H
#define SCANNER_H

#include <stdio.h>

extern int current_column;
extern int lexical_errors_count;

/**
 * Retorna o nome textual do token correspondente ao seu código numérico.
 */
const char *token_to_string(int token);

/**
 * Emite mensagem de erro léxico padronizada para stderr.
 */
void report_lexical_error(int line, int col, const char *msg);

/**
 * Executa o Scanner consumindo o arquivo fonte e imprimindo no terminal
 * a listagem formatada de todos os tokens reconhecidos.
 *
 * Retorna 0 em caso de sucesso ou 1 se houver erros léxicos.
 */
int run_scanner(FILE *source);

#endif /* SCANNER_H */
