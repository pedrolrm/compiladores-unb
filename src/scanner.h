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

/**
 * Abre o arquivo no caminho especificado e executa o Scanner.
 * Retorna 0 em caso de sucesso ou 1 se houver erros léxicos ou falha de abertura.
 */
int run_scanner_file(const char *filepath);

/**
 * Executa o Scanner redirecionando a saída formatada para out e erros para err.
 */
int run_scanner_output(FILE *source, FILE *out, FILE *err);

/**
 * Executa o Scanner em filepath gravando a saída em out_path e erros em err_path.
 */
int run_scanner_file_to_files(const char *filepath, const char *out_path, const char *err_path);

#endif /* SCANNER_H */
