#include "scanner.h"

void scanner_init(Scanner *scanner, const char *source)
{
    scanner->source = source;
    scanner->pos = 0;
    scanner->line = 1;
    scanner->column = 1;
}

/**
 * Retorna o caractere atual sem consumi-lo, ou '\0' se o fim do
 * buffer já foi alcançado.*/
static char scanner_peek(const Scanner *scanner)
{
    return scanner->source[scanner->pos];
}

/**
 * Consome o caractere atual, atualizando linha/coluna:
 * '\n' pula para a próxima linha (coluna volta a 1); qualquer outro
 * caractere apenas avança a coluna.
 */
static void scanner_advance(Scanner *scanner)
{
    char c = scanner->source[scanner->pos];
    scanner->pos++;

    if (c == '\n')
    {
        scanner->line++;
        scanner->column = 1;
    }
    else
    {
        scanner->column++;
    }
}

void scanner_skip_whitespace(Scanner *scanner)
{
    for (;;)
    {
        char c = scanner_peek(scanner);

        if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
        {
            scanner_advance(scanner);
        }
        else
        {
            break;
        }
    }
}
