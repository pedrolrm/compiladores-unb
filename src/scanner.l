%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.tab.h"

int current_column = 1;

#define YY_USER_ACTION \
    yylloc.first_line = yylloc.last_line = yylineno; \
    yylloc.first_column = current_column; \
    yylloc.last_column = current_column + yyleng - 1; \
    current_column += yyleng;
%}

%option noyywrap
%option yylineno
%option nounput noinput

%x BLOCK_COMMENT

/* Definições Regulares */
DIGIT       [0-9]
LETTER      [a-zA-Z_]
IDENTIFIER  {LETTER}({LETTER}|{DIGIT})*

%%
\n                  { current_column = 1; }
[ \t\r]+            { /* Ignora espaços */ }

"//".*              { /* Comentário de linha */ }
"/*"                { BEGIN(BLOCK_COMMENT); }

<BLOCK_COMMENT>"*/" { BEGIN(INITIAL); }
<BLOCK_COMMENT>\n   { current_column = 1; }
<BLOCK_COMMENT>.    { /* Ignora dentro do bloco */ }
<BLOCK_COMMENT><<EOF>> { 
                      fprintf(stderr, "Erro Léxico na linha %d: Comentário de bloco não fechado (EOF inesperado).\n", yylineno); 
                      exit(1); 
                    }

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

"("                 { return DELIM_LPAREN; }
")"                 { return DELIM_RPAREN; }
"{"                 { return DELIM_LBRACE; }
"}"                 { return DELIM_RBRACE; }
"["                 { return DELIM_LBRACKET; }
"]"                 { return DELIM_RBRACKET; }
";"                 { return DELIM_SEMICOLON; }
","                 { return DELIM_COMMA; }

.                   { 
                        printf("Lexical Error (%d:%d): %s\n", yylineno, yylloc.first_column, yytext); 
                        return TOKEN_ERROR; 
                    }
%%