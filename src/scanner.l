%{
#include <stdio.h>
#include <string.h>
#include "parser.tab.h" /* Header gerado pelo Bison com a definição dos tokens */

/* Variavel global para controlar a coluna atual */
int current_column = 1;

/* Macro executada antes de cada ação para atualizar yylloc (linha/coluna) */
#define YY_USER_ACTION \
    yylloc.first_line = yylloc.last_line = yylineno; \
    yylloc.first_column = current_column; \
    yylloc.last_column = current_column + yyleng - 1; \
    current_column += yyleng;
%}

%option noyywrap
%option yylineno
%option nounput noinput

/* Definições Regulares */
DIGIT       [0-9]
LETTER      [a-zA-Z_]
IDENTIFIER  {LETTER}({LETTER}|{DIGIT})*

%%

\n          { current_column = 1; /* Reseta a coluna ao pular linha */ }
[ \t\r]+    { /* Ignora espaços em branco, yy_user_action já atualiza a coluna */ }

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

{IDENTIFIER} {
    yylval.strval = strdup(yytext);
    return IDENTIFIER;
}

.           { 
                printf("Lexical Error (%d:%d): %s\n", yylineno, yylloc.first_column, yytext); 
                return TOKEN_ERROR; 
            }

%%