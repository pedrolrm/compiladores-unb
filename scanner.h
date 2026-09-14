#ifndef SCANNER_H
#define SCANNER_H

#include "tokens.h"

/**
 * Estado do Analisador Léxico (Scanner).
 *
 * Mantém o buffer com o conteúdo-fonte já carregado em memória e a
 * posição de leitura corrente, junto com a linha/coluna que serão
 * atribuídas ao próximo token reconhecido.
 */
typedef struct
{
    const char *source; /** Buffer com o código-fonte completo */
    int pos;             /**Índice do próximo caractere a ser lido em source */
    int line;            /** Linha corrente (1-based) */
    int column;          /** Coluna corrente (1-based) */
} Scanner;

/**
 * Inicializa o scanner para percorrer o buffer source.
 * A leitura começa na linha 1, coluna 1. */
void scanner_init(Scanner *scanner, const char *source);

/**
 * Avança o scanner pelos espaços em branco (' ', '\t', '\r') e por
 * quebras de linha (\n), sem gerar tokens, atualizando line e
 * column para apontar corretamente para o próximo token.
 */
void scanner_skip_whitespace(Scanner *scanner);

#endif /* SCANNER_H */
