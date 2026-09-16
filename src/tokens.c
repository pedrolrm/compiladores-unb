#include <string.h>

#include "tokens.h"

/*
 * Tabela hash (enderecamento aberto com sondagem linear) usada para separar
 * palavras reservadas de identificadores comuns em tempo constante.
 */
#define KEYWORD_SLOTS 64 /* potencia de 2 e > 2x o numero de palavras-chave */

typedef struct
{
    const char *lexeme;
    TokenType type;
    const char *name;
} Keyword;

#define KW(lexeme, type) {lexeme, type, #type}

static const Keyword keywords[] = {
    /* Tipos */
    KW("int", KW_INT),
    KW("float", KW_FLOAT),
    KW("char", KW_CHAR),
    KW("void", KW_VOID),
    KW("double", KW_DOUBLE),
    KW("bool", KW_BOOL),
    KW("long", KW_LONG),
    KW("short", KW_SHORT),
    KW("unsigned", KW_UNSIGNED),
    KW("const", KW_CONST),

    /* Controle de fluxo */
    KW("if", KW_IF),
    KW("else", KW_ELSE),
    KW("while", KW_WHILE),
    KW("for", KW_FOR),
    KW("do", KW_DO),
    KW("return", KW_RETURN),
    KW("break", KW_BREAK),
    KW("continue", KW_CONTINUE),
    KW("switch", KW_SWITCH),
    KW("case", KW_CASE),
    KW("default", KW_DEFAULT),

    /* Estruturas de dados */
    KW("struct", KW_STRUCT),
    KW("typedef", KW_TYPEDEF),

    /* Literais booleanos */
    KW("true", KW_TRUE),
    KW("false", KW_FALSE),
};

#define KEYWORD_COUNT ((int)(sizeof(keywords) / sizeof(keywords[0])))

static const Keyword *slots[KEYWORD_SLOTS];
static int table_ready;

/* djb2 */
static unsigned long hash(const char *s)
{
    unsigned long h = 5381;

    while (*s)
    {
        h = ((h << 5) + h) + (unsigned char)*s++;
    }
    return h;
}

static void build_table(void)
{
    int i;

    for (i = 0; i < KEYWORD_COUNT; i++)
    {
        unsigned long slot = hash(keywords[i].lexeme) % KEYWORD_SLOTS;

        while (slots[slot] != NULL)
        {
            slot = (slot + 1) % KEYWORD_SLOTS;
        }
        slots[slot] = &keywords[i];
    }
    table_ready = 1;
}

TokenType keyword_lookup(const char *lexeme)
{
    unsigned long slot;

    if (!table_ready)
    {
        build_table();
    }

    slot = hash(lexeme) % KEYWORD_SLOTS;
    while (slots[slot] != NULL)
    {
        if (strcmp(slots[slot]->lexeme, lexeme) == 0)
        {
            return slots[slot]->type;
        }
        slot = (slot + 1) % KEYWORD_SLOTS;
    }
    return IDENTIFIER;
}

const char *token_name(TokenType token)
{
    int i;

    if (token == IDENTIFIER)
    {
        return "IDENTIFIER";
    }
    for (i = 0; i < KEYWORD_COUNT; i++)
    {
        if (keywords[i].type == token)
        {
            return keywords[i].name;
        }
    }
    return "TOKEN_ERROR";
}
