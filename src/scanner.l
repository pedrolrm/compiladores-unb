%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.tab.h"

/* Variavel global para controlar a coluna atual */
int current_column = 1;

/* Macro executada antes de cada ação para atualizar yylloc */
#define YY_USER_ACTION \
    yylloc.first_line = yylloc.last_line = yylineno; \
    yylloc.first_column = current_column; \
    yylloc.last_column = current_column + yyleng - 1; \
    current_column += yyleng;

/*
 * Converte uma sequencia de escape para seu caractere correspondente.
 *
 * Exemplos:
 *   \n -> newline
 *   \t -> tab
 *   \\ -> backslash
 *   \" -> aspas
 *   \' -> apostrofo
 */
static char decode_escape(char c)
{
    switch (c) {
        case 'n':  return '\n';
        case 't':  return '\t';
        case 'r':  return '\r';
        case 'b':  return '\b';
        case 'f':  return '\f';
        case 'v':  return '\v';
        case '\\': return '\\';
        case '"':  return '"';
        case '\'': return '\'';
        case '0':  return '\0';
        default:   return c;
    }
}

/*
 * Remove as aspas da string e interpreta as sequencias de escape.
 *
 * Exemplo:
 *   "ola\nmundo"
 *
 * vira:
 *   ola
 *   mundo
 */
static char *decode_string_literal(const char *text)
{
    size_t len = strlen(text);

    /* Remove as aspas inicial e final */
    char *result = malloc(len);

    if (!result) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    size_t j = 0;

    for (size_t i = 1; i < len - 1; i++) {
        if (text[i] == '\\' && i + 1 < len - 1) {
            result[j++] = decode_escape(text[++i]);
        } else {
            result[j++] = text[i];
        }
    }

    result[j] = '\0';

    return result;
}

/*
 * Converte um literal de caractere.
 *
 * Exemplos:
 *   'a'  -> 'a'
 *   '\n' -> newline
 *   '\'' -> apostrofo
 */
static char decode_char_literal(const char *text)
{
    if (text[1] == '\\') {
        return decode_escape(text[2]);
    }

    return text[1];
}
%}

%option noyywrap
%option yylineno
%option nounput noinput

DIGIT       [0-9]
DIGITS      {DIGIT}+
EXPONENT    [eE][+-]?{DIGITS}

ESCAPE      \\([ntrbfv\\'\"0])

%%
\n {
    current_column = 1;
}

[ \t\r]+ {
    /* Ignora espacos em branco */
}

/*
 * Literais inteiros.
 *
 * Exemplos:
 *   0
 *   10
 *   12345
 */
{DIGITS} {
    yylval.intval = atoi(yytext);
    return INT_LITERAL;
}

/*
 * Literais float.
 *
 * Exemplos:
 *   10.5
 *   3.14
 *   10.
 *   .5
 *   1e10
 *   2.5e-3
 *
 * A regra de FLOAT vem antes da de INT para que
 * valores como 10.5 sejam reconhecidos como um unico token.
 */
(({DIGITS}"."{DIGIT}*|"."{DIGITS})({EXPONENT})?|{DIGITS}{EXPONENT}) {
    yylval.floatval = strtof(yytext, NULL);
    return FLOAT_LITERAL;
}

/*
 * String literal fechado corretamente.
 *
 * Permite qualquer caractere exceto:
 *   - aspas
 *   - barra invertida
 *   - quebra de linha
 *
 * Escapes validos sao tratados por decode_string_literal().
 */
\"([^\"\\\n]|{ESCAPE})*\" {
    yylval.strval = decode_string_literal(yytext);
    return STRING_LITERAL;
}

/*
 * String iniciada mas nao fechada antes da quebra de linha.
 */
\"([^\"\\\n]|{ESCAPE})*\n {
    fprintf(
        stderr,
        "Lexical Error (%d:%d): string literal nao fechada\n",
        yylineno,
        yylloc.first_column
    );

    current_column = 1;
    return TOKEN_ERROR;
}

/*
 * String iniciada mas nao fechada ate o final do arquivo.
 */
\"([^\"\\\n]|{ESCAPE})*$ {
    fprintf(
        stderr,
        "Lexical Error (%d:%d): string literal nao fechada\n",
        yylineno,
        yylloc.first_column
    );

    return TOKEN_ERROR;
}

/*
 * Literal de caractere fechado corretamente.
 *
 * Exemplos:
 *   'a'
 *   '1'
 *   '\n'
 *   '\''
 */
'([^'\\\n]|{ESCAPE})' {
    yylval.charval = decode_char_literal(yytext);
    return CHAR_LITERAL;
}

/*
 * Caractere iniciado mas nao fechado antes da quebra de linha.
 */
'([^'\\\n]|{ESCAPE})*\n {
    fprintf(
        stderr,
        "Lexical Error (%d:%d): char literal nao fechado\n",
        yylineno,
        yylloc.first_column
    );

    current_column = 1;
    return TOKEN_ERROR;
}

/*
 * Caractere iniciado mas nao fechado ate o final do arquivo.
 */
'([^'\\\n]|{ESCAPE})*$ {
    fprintf(
        stderr,
        "Lexical Error (%d:%d): char literal nao fechado\n",
        yylineno,
        yylloc.first_column
    );

    return TOKEN_ERROR;
}

/*
 * Qualquer outro caractere ainda nao implementado pelo scanner.
 */
. {
    printf(
        "Lexical Error (%d:%d): %s\n",
        yylineno,
        yylloc.first_column,
        yytext
    );

    return TOKEN_ERROR;
}
%%
```
