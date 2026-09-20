%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.tab.h"

int current_column = 1;

/* Contador global de erros léxicos */
int lexical_errors_count = 0;

#define YY_USER_ACTION \
    yylloc.first_line = yylloc.last_line = yylineno; \
    yylloc.first_column = current_column; \
    yylloc.last_column = current_column + yyleng - 1; \
    current_column += yyleng;

/*
 * Reporta um erro léxico com mensagem padronizada, exibindo linha e coluna.
 * Não quebra a execução abruptamente.
 */
void report_lexical_error(int line, int col, const char *msg)
{
    lexical_errors_count++;
    fprintf(stderr, "Erro léxico na linha %d, coluna %d: %s\n", line, col, msg);
}

/*
 * Converte uma sequencia de escape para seu caractere correspondente.
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
 */
static char *decode_string_literal(const char *text)
{
    size_t len = strlen(text);

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

%x BLOCK_COMMENT

/* Definições Regulares */
DIGIT       [0-9]
DIGITS      {DIGIT}+
LETTER      [a-zA-Z_]
IDENTIFIER  {LETTER}({LETTER}|{DIGIT})*
EXPONENT    [eE][+-]?{DIGITS}

ESCAPE      \\([ntrbfv\\'\"0])

%%
\n                  { current_column = 1; }
[ \t\r]+            { /* Ignora espaços */ }

"//".*              { /* Comentário de linha */ }
"/*"                { BEGIN(BLOCK_COMMENT); }

<BLOCK_COMMENT>"*/" { BEGIN(INITIAL); }
<BLOCK_COMMENT>\n   { current_column = 1; }
<BLOCK_COMMENT>.    { /* Ignora dentro do bloco */ }
<BLOCK_COMMENT><<EOF>> { 
                      report_lexical_error(yylineno, yylloc.first_column, "comentário de bloco não fechado (EOF inesperado)"); 
                      BEGIN(INITIAL);
                      return TOKEN_ERROR; 
                    }

/* Palavras-chave */
"int"       { return KW_INT; }
"float"     { return KW_FLOAT; }
"char"      { return KW_CHAR; }
"void"      { return KW_VOID; }
"if"        { return KW_IF; }
"else"      { return KW_ELSE; }
"while"     { return KW_WHILE; }
"for"       { return KW_FOR; }
"return"    { return KW_RETURN; }
"break"     { return KW_BREAK; }
"continue"  { return KW_CONTINUE; }
"switch"    { return KW_SWITCH; }
"case"      { return KW_CASE; }
"default"   { return KW_DEFAULT; }
"double"    { return KW_DOUBLE; }
"bool"      { return KW_BOOL; }
"long"      { return KW_LONG; }
"short"     { return KW_SHORT; }
"unsigned"  { return KW_UNSIGNED; }
"const"     { return KW_CONST; }
"struct"    { return KW_STRUCT; }
"typedef"   { return KW_TYPEDEF; }
"do"        { return KW_DO; }
"true"      { return KW_TRUE; }
"false"     { return KW_FALSE; }

/* Identificadores */
{IDENTIFIER} {
    yylval.strval = strdup(yytext);
    return IDENTIFIER;
}

/*
 * Literais float (deve vir antes de int para casar 10.5 como um unico token).
 */
(({DIGITS}"."{DIGIT}*|"."{DIGITS})({EXPONENT})?|{DIGITS}{EXPONENT}) {
    yylval.floatval = strtof(yytext, NULL);
    return FLOAT_LITERAL;
}

/*
 * Literais inteiros.
 */
{DIGITS} {
    yylval.intval = atoi(yytext);
    return INT_LITERAL;
}

/*
 * String literal fechado corretamente.
 */
\"([^\"\\\n]|{ESCAPE})*\" {
    yylval.strval = decode_string_literal(yytext);
    return STRING_LITERAL;
}

/*
 * String iniciada mas nao fechada antes da quebra de linha.
 */
\"([^\"\\\n]|{ESCAPE})*\n {
    report_lexical_error(yylloc.first_line, yylloc.first_column, "string literal não fechada");
    current_column = 1;
    return TOKEN_ERROR;
}

/*
 * String iniciada mas nao fechada ate o final do arquivo.
 */
\"([^\"\\\n]|{ESCAPE})*$ {
    report_lexical_error(yylloc.first_line, yylloc.first_column, "string literal não fechada");
    return TOKEN_ERROR;
}

/*
 * Literal de caractere fechado corretamente.
 */
'([^'\\\n]|{ESCAPE})' {
    yylval.charval = decode_char_literal(yytext);
    return CHAR_LITERAL;
}

/*
 * Caractere iniciado mas nao fechado antes da quebra de linha.
 */
'([^'\\\n]|{ESCAPE})*\n {
    report_lexical_error(yylloc.first_line, yylloc.first_column, "literal de caractere não fechado");
    current_column = 1;
    return TOKEN_ERROR;
}

/*
 * Caractere iniciado mas nao fechado ate o final do arquivo.
 */
'([^'\\\n]|{ESCAPE})*$ {
    report_lexical_error(yylloc.first_line, yylloc.first_column, "literal de caractere não fechado");
    return TOKEN_ERROR;
}

/* Operadores */
"=="                { return OP_EQ; }
"!="                { return OP_NEQ; }
"<="                { return OP_LE; }
">="                { return OP_GE; }
"&&"                { return OP_AND; }
"||"                { return OP_OR; }

"++"                { return OP_INC; }
"--"                { return OP_DEC; }
"+="                { return OP_PLUS_ASSIGN; }
"-="                { return OP_MINUS_ASSIGN; }
"*="                { return OP_MULT_ASSIGN; }
"/="                { return OP_DIV_ASSIGN; }

"+"                 { return OP_PLUS; }
"-"                 { return OP_MINUS; }
"*"                 { return OP_MULT; }
"/"                 { return OP_DIV; }
"%"                 { return OP_MOD; }
"="                 { return OP_ASSIGN; }
"<"                 { return OP_LT; }
">"                 { return OP_GT; }
"!"                 { return OP_NOT; }

"?"                 { return OP_QUESTION; }
":"                 { return OP_COLON; }

/* Delimitadores */
"("                 { return DELIM_LPAREN; }
")"                 { return DELIM_RPAREN; }
"{"                 { return DELIM_LBRACE; }
"}"                 { return DELIM_RBRACE; }
"["                 { return DELIM_LBRACKET; }
"]"                 { return DELIM_RBRACKET; }
";"                 { return DELIM_SEMICOLON; }
","                 { return DELIM_COMMA; }

/* Caracteres inválidos / desconhecidos */
.                   { 
                        char err_msg[128];
                        snprintf(err_msg, sizeof(err_msg), "caractere inválido '%s'", yytext);
                        report_lexical_error(yylloc.first_line, yylloc.first_column, err_msg); 
                        return TOKEN_ERROR; 
                    }
%%
