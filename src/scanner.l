%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.tab.h"
#include "scanner.h"

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

const char *token_to_string(int token)
{
    switch (token) {
        case 0:                 return "EOF";
        case TOKEN_ERROR:       return "TOKEN_ERROR";

        /* Palavras-chave */
        case KW_INT:            return "KW_INT";
        case KW_FLOAT:          return "KW_FLOAT";
        case KW_CHAR:           return "KW_CHAR";
        case KW_VOID:           return "KW_VOID";
        case KW_IF:             return "KW_IF";
        case KW_ELSE:           return "KW_ELSE";
        case KW_WHILE:          return "KW_WHILE";
        case KW_FOR:            return "KW_FOR";
        case KW_RETURN:         return "KW_RETURN";
        case KW_BREAK:          return "KW_BREAK";
        case KW_CONTINUE:       return "KW_CONTINUE";
        case KW_SWITCH:         return "KW_SWITCH";
        case KW_CASE:           return "KW_CASE";
        case KW_DEFAULT:        return "KW_DEFAULT";
        case KW_DOUBLE:         return "KW_DOUBLE";
        case KW_BOOL:           return "KW_BOOL";
        case KW_LONG:           return "KW_LONG";
        case KW_SHORT:          return "KW_SHORT";
        case KW_UNSIGNED:       return "KW_UNSIGNED";
        case KW_CONST:          return "KW_CONST";
        case KW_STRUCT:         return "KW_STRUCT";
        case KW_TYPEDEF:        return "KW_TYPEDEF";
        case KW_DO:             return "KW_DO";
        case KW_TRUE:           return "KW_TRUE";
        case KW_FALSE:          return "KW_FALSE";

        /* Identificadores e Literais */
        case IDENTIFIER:        return "IDENTIFIER";
        case INT_LITERAL:       return "INT_LITERAL";
        case FLOAT_LITERAL:     return "FLOAT_LITERAL";
        case STRING_LITERAL:    return "STRING_LITERAL";
        case CHAR_LITERAL:      return "CHAR_LITERAL";

        /* Operadores */
        case OP_PLUS:           return "OP_PLUS";
        case OP_MINUS:          return "OP_MINUS";
        case OP_MULT:           return "OP_MULT";
        case OP_DIV:            return "OP_DIV";
        case OP_MOD:            return "OP_MOD";
        case OP_ASSIGN:         return "OP_ASSIGN";
        case OP_EQ:             return "OP_EQ";
        case OP_NEQ:            return "OP_NEQ";
        case OP_LT:             return "OP_LT";
        case OP_LE:             return "OP_LE";
        case OP_GT:             return "OP_GT";
        case OP_GE:             return "OP_GE";
        case OP_AND:            return "OP_AND";
        case OP_OR:             return "OP_OR";
        case OP_NOT:            return "OP_NOT";
        case OP_INC:            return "OP_INC";
        case OP_DEC:            return "OP_DEC";
        case OP_PLUS_ASSIGN:    return "OP_PLUS_ASSIGN";
        case OP_MINUS_ASSIGN:   return "OP_MINUS_ASSIGN";
        case OP_MULT_ASSIGN:    return "OP_MULT_ASSIGN";
        case OP_DIV_ASSIGN:     return "OP_DIV_ASSIGN";
        case OP_QUESTION:       return "OP_QUESTION";
        case OP_COLON:          return "OP_COLON";

        /* Delimitadores */
        case DELIM_LPAREN:      return "DELIM_LPAREN";
        case DELIM_RPAREN:      return "DELIM_RPAREN";
        case DELIM_LBRACE:      return "DELIM_LBRACE";
        case DELIM_RBRACE:      return "DELIM_RBRACE";
        case DELIM_LBRACKET:    return "DELIM_LBRACKET";
        case DELIM_RBRACKET:    return "DELIM_RBRACKET";
        case DELIM_SEMICOLON:   return "DELIM_SEMICOLON";
        case DELIM_COMMA:       return "DELIM_COMMA";

        default:                return "UNKNOWN_TOKEN";
    }
}

int run_scanner(FILE *source)
{
    yyin = source;
    current_column = 1;
    yylineno = 1;

    int token;
    int token_count = 0;

    printf("======================================================================\n");
    printf("%-8s %-8s %-20s %s\n", "LINHA", "COLUNA", "TOKEN", "LEXEMA");
    printf("======================================================================\n");

    while ((token = yylex()) != 0) {
        token_count++;
        const char *name = token_to_string(token);

        if (token == INT_LITERAL) {
            printf("%-8d %-8d %-20s '%s' (valor: %d)\n",
                   yylloc.first_line, yylloc.first_column, name, yytext, yylval.intval);
        } else if (token == FLOAT_LITERAL) {
            printf("%-8d %-8d %-20s '%s' (valor: %g)\n",
                   yylloc.first_line, yylloc.first_column, name, yytext, yylval.floatval);
        } else if (token == STRING_LITERAL) {
            printf("%-8d %-8d %-20s '%s' (valor: \"%s\")\n",
                   yylloc.first_line, yylloc.first_column, name, yytext, yylval.strval ? yylval.strval : "");
            if (yylval.strval) {
                free(yylval.strval);
                yylval.strval = NULL;
            }
        } else if (token == CHAR_LITERAL) {
            printf("%-8d %-8d %-20s '%s' (valor: '%c')\n",
                   yylloc.first_line, yylloc.first_column, name, yytext, yylval.charval);
        } else if (token == IDENTIFIER) {
            printf("%-8d %-8d %-20s '%s'\n",
                   yylloc.first_line, yylloc.first_column, name, yytext);
            if (yylval.strval) {
                free(yylval.strval);
                yylval.strval = NULL;
            }
        } else {
            printf("%-8d %-8d %-20s '%s'\n",
                   yylloc.first_line, yylloc.first_column, name, yytext);
        }
    }

    printf("======================================================================\n");
    printf("Total de tokens processados: %d\n", token_count);
    if (lexical_errors_count > 0) {
        printf("Total de erros léxicos encontrados: %d\n", lexical_errors_count);
    }
    printf("======================================================================\n");

    return (lexical_errors_count > 0) ? 1 : 0;
}
