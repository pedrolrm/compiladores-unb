#ifndef TOKENS_H
#define TOKENS_H

/**
 * Enumeração dos tipos de tokens suportados pelo Analisador Léxico.
 *
 * NOTA: este arquivo é apenas referência/documentação. Os códigos usados pelo
 * compilador são gerados pelo Bison (src/parser.tab.h) a partir das
 * declarações %token em src/parser.y e expostos em src/tokens.h.
 *
 * Contém categorias para palavras-chave, identificadores, literais,
 * operadores lógicos/matemáticos e delimitadores estruturais do C.
 */
typedef enum
{
    /* Fim de arquivo e Erro */
    TOKEN_EOF = 0,
    TOKEN_ERROR,

    /* Palavras-chave */
    KW_INT,
    KW_FLOAT,
    KW_CHAR,
    KW_VOID,
    KW_IF,
    KW_ELSE,
    KW_WHILE,
    KW_FOR,
    KW_RETURN,

    /* Controle de fluxo e Selecao */
    KW_BREAK,
    KW_CONTINUE,
    KW_SWITCH,
    KW_CASE,
    KW_DEFAULT,

    /* Tipos e modificadores extras */
    KW_DOUBLE,
    KW_BOOL,
    KW_LONG,
    KW_SHORT,
    KW_UNSIGNED,
    KW_CONST,

    /* Estruturas de dados */
    KW_STRUCT,
    KW_TYPEDEF,

    /* Outros lacos */
    KW_DO,

    /* Identificadores */
    IDENTIFIER,

    /* Literais */
    INT_LITERAL,
    FLOAT_LITERAL,
    STRING_LITERAL,
    CHAR_LITERAL,
    KW_TRUE,
    KW_FALSE, /* Literais booleanos */

    /* Operadores */
    OP_PLUS,
    OP_MINUS,
    OP_MULT,
    OP_DIV,
    OP_MOD,
    OP_ASSIGN,
    OP_EQ,
    OP_NEQ,
    OP_LT,
    OP_LE,
    OP_GT,
    OP_GE,
    OP_AND,
    OP_OR,
    OP_NOT,

    /* Incremento e decremento */
    OP_INC,
    OP_DEC,

    /* Atribuicao composta */
    OP_PLUS_ASSIGN,
    OP_MINUS_ASSIGN,
    OP_MULT_ASSIGN,
    OP_DIV_ASSIGN,

    /* Operador ternario */
    OP_QUESTION,
    OP_COLON,

    /* Delimitadores */
    DELIM_LPAREN,
    DELIM_RPAREN, /* () */
    DELIM_LBRACE,
    DELIM_RBRACE, /* {} */
    DELIM_LBRACKET,
    DELIM_RBRACKET, /* [] */
    DELIM_SEMICOLON,
    DELIM_COMMA /* ; , */
} TokenType;

/**
 * Estrutura que representa um Token gerado pelo Scanner.
 *
 * Armazena as informações essenciais para a etapa de parsing sintático
 * e também para reportar erros com precisão no código-fonte.
 */
typedef struct
{
    TokenType type; /**< Categoria/Tipo do token */
    char *lexeme;   /**< O valor literal (string) lido do arquivo fonte */
    int line;       /**< Linha onde o token começou (para debug/erros) */
    int column;     /**< Coluna onde o token começou (para debug/erros) */
} Token;

#endif /* TOKENS_H */
