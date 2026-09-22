# Tokens e Expressões Regulares

Esta página documenta o levantamento de tokens da linguagem alvo do compilador, conforme a issue **#5**, cobrindo palavras reservadas, identificadores, literais, operadores, delimitadores e as validações feitas sobre as expressões regulares propostas.

Este documento cobre apenas a **especificação léxica**: os nomes de token seguem exatamente o enum `TokenType` definido em `docs/tokens_reference.h`. A implementação do analisador léxico em si (`src/scanner.l`) é tratada separadamente na issue **#6**.

## Visão Geral

A linguagem alvo é uma variação simplificada de C. Os tokens se dividem em seis categorias:

- **Palavras reservadas**: identificadores com significado fixo na linguagem (`int`, `if`, `while`, etc).
- **Identificadores**: nomes de variáveis, funções e tipos definidos pelo usuário.
- **Literais**: valores constantes escritos diretamente no código-fonte (inteiros, ponto flutuante, strings, caracteres, booleanos).
- **Operadores**: símbolos que representam operações aritméticas, relacionais, lógicas, de atribuição e ternárias.
- **Delimitadores**: símbolos estruturais como parênteses, chaves e ponto-e-vírgula.
- **Comentários e espaços em branco**: ignorados pelo analisador léxico, não geram token.

Alguns blocos de construção reutilizados em várias regex:

| Nome | Regex | Descrição |
|---|---|---|
| `DIGIT` | `[0-9]` | Um dígito decimal |
| `DIGITS` | `[0-9]+` | Uma ou mais dígitos |
| `LETTER` | `[a-zA-Z_]` | Uma letra ou underscore |
| `EXPONENT` | `[eE][+-]?[0-9]+` | Notação exponencial (`e10`, `E-3`) |
| `ESCAPE` | `\[ntrbfv\'"0]` | Sequência de escape válida |

## Palavras Reservadas

Palavras reservadas não podem ser usadas como identificadores. Todas em minúsculas.

| Palavra reservada | Token |
|---|---|
| `int` | `KW_INT` |
| `float` | `KW_FLOAT` |
| `char` | `KW_CHAR` |
| `void` | `KW_VOID` |
| `if` | `KW_IF` |
| `else` | `KW_ELSE` |
| `while` | `KW_WHILE` |
| `for` | `KW_FOR` |
| `return` | `KW_RETURN` |
| `break` | `KW_BREAK` |
| `continue` | `KW_CONTINUE` |
| `switch` | `KW_SWITCH` |
| `case` | `KW_CASE` |
| `default` | `KW_DEFAULT` |
| `double` | `KW_DOUBLE` |
| `bool` | `KW_BOOL` |
| `long` | `KW_LONG` |
| `short` | `KW_SHORT` |
| `unsigned` | `KW_UNSIGNED` |
| `const` | `KW_CONST` |
| `struct` | `KW_STRUCT` |
| `typedef` | `KW_TYPEDEF` |
| `do` | `KW_DO` |
| `true` | `KW_TRUE` |
| `false` | `KW_FALSE` |

Cada palavra reservada é reconhecida como um literal exato (ex.: `"int"`, `"while"`), sem variação.

## Identificadores

Um identificador começa com uma letra ou `_`, seguido de zero ou mais letras, dígitos ou `_`.

Regex (`IDENTIFIER`):

```regex
[a-zA-Z_][a-zA-Z_0-9]*
```

Se a sequência casar com uma palavra reservada (ex.: `if`), o token gerado é o da palavra-chave, não `IDENTIFIER` — a regra de palavra-chave tem prioridade sobre a regra genérica de identificador.

Exemplos válidos: `salario_base`, `inicial`, `_temp`, `x1`.
Exemplos inválidos: `1x` (começa com dígito), `sal-ario` (hífen não é permitido).

## Literais

### Números Inteiros

Token: `INT_LITERAL`.

```regex
[0-9]+
```

Exemplos válidos: `123`, `0`, `42`.

### Números de Ponto Flutuante

Token: `FLOAT_LITERAL`. Cobre notação decimal com parte fracionária opcional em qualquer lado do ponto, e notação exponencial.

```regex
([0-9]+\.[0-9]*|\.[0-9]+)([eE][+-]?[0-9]+)?|[0-9]+[eE][+-]?[0-9]+
```

Esta regra precisa ser avaliada antes da regra de inteiro para que `10.5` seja reconhecido como um único token de ponto flutuante, e não como `10` seguido de `.` e `5`.

Exemplos válidos: `123.45`, `0.5`, `123.`, `.75`, `1e10`, `1.5e-2`.

### Strings

Token: `STRING_LITERAL`. Uma string é delimitada por aspas duplas, sem quebra de linha literal dentro dela, aceitando sequências de escape.

```regex
"([^"\\\n]|\[ntrbfv\'"0])*"
```

Exemplos válidos: `"hello"`, `"hello world"`, `"linha\nnova"`, `"tab\ttexto"`, `"aspas: \"hello\""`, `"barra: \\"`.

Uma string iniciada e não fechada antes do fim da linha ou do arquivo é um erro léxico (ver seção de Validação).

### Caracteres

Token: `CHAR_LITERAL`. Exatamente um caractere (ou uma sequência de escape) entre aspas simples.

```regex
'([^'\\\n]|\[ntrbfv\'"0])'
```

Exemplos válidos: `'a'`, `'Z'`, `'1'`, `'\n'`, `'\t'`, `'\\'`, `'\''`, `'\"'`.

### Literais Booleanos

`true` e `false` são tratados como palavras reservadas (`KW_TRUE`, `KW_FALSE`), listadas na seção de Palavras Reservadas — não têm regex própria além da correspondência literal.

## Operadores

| Categoria | Símbolo | Token |
|---|---|---|
| Aritmético | `+` | `OP_PLUS` |
| Aritmético | `-` | `OP_MINUS` |
| Aritmético | `*` | `OP_MULT` |
| Aritmético | `/` | `OP_DIV` |
| Aritmético | `%` | `OP_MOD` |
| Relacional | `==` | `OP_EQ` |
| Relacional | `!=` | `OP_NEQ` |
| Relacional | `<` | `OP_LT` |
| Relacional | `<=` | `OP_LE` |
| Relacional | `>` | `OP_GT` |
| Relacional | `>=` | `OP_GE` |
| Lógico | `&&` | `OP_AND` |
| Lógico | `||` | `OP_OR` |
| Lógico | `!` | `OP_NOT` |
| Atribuição | `=` | `OP_ASSIGN` |
| Atribuição composta | `+=` | `OP_PLUS_ASSIGN` |
| Atribuição composta | `-=` | `OP_MINUS_ASSIGN` |
| Atribuição composta | `*=` | `OP_MULT_ASSIGN` |
| Atribuição composta | `/=` | `OP_DIV_ASSIGN` |
| Incremento/Decremento | `++` | `OP_INC` |
| Incremento/Decremento | `--` | `OP_DEC` |
| Ternário | `?` | `OP_QUESTION` |
| Ternário | `:` | `OP_COLON` |

Todos os operadores são literais exatos (sem classes de caracteres). Operadores de dois caracteres (`==`, `!=`, `<=`, `>=`, `&&`, `||`, `++`, `--`, `+=`, `-=`, `*=`, `/=`) devem ser reconhecidos antes de seus prefixos de um caractere (`=`, `!`, `<`, `>`, `+`, `-`, `*`, `/`), para que `==` não seja lido como dois `=` separados.

## Delimitadores

| Símbolo | Token |
|---|---|
| `(` | `DELIM_LPAREN` |
| `)` | `DELIM_RPAREN` |
| `{` | `DELIM_LBRACE` |
| `}` | `DELIM_RBRACE` |
| `[` | `DELIM_LBRACKET` |
| `]` | `DELIM_RBRACKET` |
| `;` | `DELIM_SEMICOLON` |
| `,` | `DELIM_COMMA` |

## Comentários e Espaços em Branco

Não geram token — são descartados pelo analisador léxico, mas fazem parte da especificação léxica da linguagem.

- Comentário de linha: `//.*`
- Comentário de bloco: inicia com `/*` e termina com `*/`, podendo abranger múltiplas linhas (tratado com uma condição de início exclusiva no scanner, não uma única regex linear).
- Espaços em branco: `[ \t\r\n]+`.

## Validação das Expressões Regulares

Critério de aceitação da issue: validar cada regex com exemplos válidos e inválidos (regex101 ou similar). A lista abaixo reúne casos de teste de referência para conferência manual (colar a regex correspondente em regex101.com, flavor PCRE, e testar contra as strings abaixo).

### Casos Válidos

Inteiros e floats:

```text
123
0
42
123.45
0.5
123.
.75
1e10
1.5e-2
```

Strings com escapes:

```text
"hello"
"hello world"
"linha\nnova"
"tab\ttexto"
"aspas: \"hello\""
"barra: \\"
```

Caracteres com escapes:

```text
'a'
'Z'
'1'
'\n'
'\t'
'\\'
'\''
'\"'
```

Palavras-chave e identificadores em contexto:

```c
int main() {
    float salario_base;
    char inicial;

    if (salario_base) {
        return 1;
    } else {
        while (true) {
            continue;
        }
    }
}
```

### Casos Inválidos

String não fechada (erro léxico esperado):

```c
char *invalido = "esta string nunca foi fechada
```

Caractere não fechado (erro léxico esperado):

```c
char c = 'a
```

Caracteres não reconhecidos pela linguagem (erro léxico esperado):

```text
@  $  #  `  ~
```

Identificador inválido (não casa com a regex de `IDENTIFIER`):

```text
1x
sal-ario
```

## Referências

- Enum canônico de tokens: `docs/tokens_reference.h`
- Implementação do analisador léxico: issue #6 (`src/scanner.l`)
- Sub-issues desta especificação: #23 (palavras reservadas/identificadores), #24 (literais), #25 (operadores/delimitadores), #26 (validação)
