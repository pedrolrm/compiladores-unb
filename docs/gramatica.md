# Gramática Livre de Contexto (GLC)

Este documento descreve, em EBNF, a gramática da linguagem alvo **tal como implementada** em [`src/parser.y`](https://github.com/pedrolrm/compiladores-unb/blob/main/src/parser.y) (bison), com os terminais léxicos definidos em [`src/scanner.l`](https://github.com/pedrolrm/compiladores-unb/blob/main/src/scanner.l) (flex). Cada produção abaixo corresponde 1:1 a uma regra do parser simplificado para a futura geração de código Python.

## Notação

- `|` — alternativa.
- `[ x ]` — `x` é opcional.
- `{ x }` — `x` se repete zero ou mais vezes.
- `"texto"` — terminal literal (palavra-chave, operador ou delimitador).
- `IDENTIFICADOR`, `LIT_INT`, `LIT_FLOAT`, `LIT_CHAR`, `LIT_STRING` — terminais léxicos com valor semântico (vindos do scanner).

## Terminais léxicos na Gramática

| Categoria | Lexemas |
|---|---|
| Palavras-chave de tipo | `int`, `float`, `char`, `bool`, `void` |
| Palavras-chave de controle de fluxo | `if`, `else`, `while`, `for`, `break`, `continue`, `return` |
| Literais booleanos | `true`, `false` |
| Identificador / literais | `IDENTIFICADOR`, `LIT_INT`, `LIT_FLOAT`, `LIT_CHAR`, `LIT_STRING` |
| Operadores aritméticos | `+`, `-`, `*`, `/`, `%` |
| Operadores relacionais/lógicos | `==`, `!=`, `<`, `<=`, `>`, `>=`, `&&`, `\|\|`, `!` |
| Atribuição | `=`, `+=`, `-=`, `*=`, `/=` |
| Delimitadores | `(`, `)`, `{`, `}`, `[`, `]`, `;`, `,` |

> As palavras-chave lexadas pelo scanner como reserva de compatibilidade (`struct`, `switch`, `case`, `default`, `double`, `long`, `short`, `unsigned`, `const`, `typedef`, `do`) e operadores como `++`, `--`, `?`, `:`, `.` não participam das produções ativas do parser conforme a Decisão de Projeto nº 2 ([`docs/decisoes.md`](decisoes.md)).

## Produções

### Programa e comandos

```ebnf
programa            ::= { comando }

comando              ::= declaracao
                        | definicao_funcao
                        | comando_expressao
                        | bloco
                        | comando_selecao
                        | comando_iteracao
                        | comando_desvio
```

### Funções

```ebnf
definicao_funcao     ::= especificador_tipo IDENTIFICADOR "(" lista_parametros ")" bloco
                        | "void" IDENTIFICADOR "(" lista_parametros ")" bloco

lista_parametros      ::= [ "void" | parametro { "," parametro } ]

parametro             ::= especificador_tipo IDENTIFICADOR
```

### Bloco

```ebnf
bloco                 ::= "{" { comando } "}"
```

### Seleção (`if`/`else`)

```ebnf
comando_selecao       ::= "if" "(" expressao ")" comando [ "else" comando ]
```

> A ambiguidade clássica do *dangling else* é resolvida por precedência:
> `else` tem prioridade maior que um `if` sem `else` (`%prec LOWER_THAN_ELSE`
> vs. `KW_ELSE` em `src/parser.y`), fazendo o parser preferir shift — o
> `else` sempre se liga ao `if` mais próximo sem `else` associado.

### Iteração (`while`, `for`)

```ebnf
comando_iteracao      ::= "while" "(" expressao ")" comando
                        | "for" "(" inicio_for [ expressao ] ";" [ expressao ] ")" comando

inicio_for             ::= declaracao
                        | comando_expressao
```

### Desvio de fluxo

```ebnf
comando_desvio        ::= "break" ";"
                        | "continue" ";"
                        | "return" [ expressao ] ";"
```

### Declarações e tipos

```ebnf
declaracao             ::= especificador_tipo lista_declaradores_init ";"

especificador_tipo     ::= "int" | "float" | "char" | "bool"

lista_declaradores_init ::= declarador_init { "," declarador_init }

declarador_init        ::= declarador [ "=" expressao ]

declarador              ::= IDENTIFICADOR [ "[" [ expressao ] "]" ]
```

> `declarador` cobre variáveis escalares e vetores unidimensionais com tamanho fixo (`v[10]`) ou inferido (`buf[]`).

### Comando de expressão

```ebnf
comando_expressao      ::= [ expressao ] ";"
```

### Expressões

```ebnf
expressao               ::= expressao_pos ( "=" | "+=" | "-=" | "*=" | "/=" ) expressao
                        | expressao "||" expressao
                        | expressao "&&" expressao
                        | expressao ( "==" | "!=" ) expressao
                        | expressao ( "<" | "<=" | ">" | ">=" ) expressao
                        | expressao ( "+" | "-" ) expressao
                        | expressao ( "*" | "/" | "%" ) expressao
                        | "-" expressao
                        | "!" expressao
                        | "(" expressao ")"
                        | expressao_pos

expressao_pos           ::= expressao_primaria
                        | expressao_pos "[" expressao "]"

expressao_primaria      ::= IDENTIFICADOR
                        | IDENTIFICADOR "(" lista_argumentos ")"
                        | LIT_INT | LIT_FLOAT | LIT_CHAR | LIT_STRING
                        | "true" | "false"

lista_argumentos        ::= [ expressao { "," expressao } ]
```

## Precedência e associatividade

Resolvida via as declarações de precedência de `src/parser.y` (da menor para a maior):

| Nível (menor → maior) | Operadores | Associatividade |
|---|---|---|
| 1 | `=`, `+=`, `-=`, `*=`, `/=` | não-associativo |
| 2 | `\|\|` | esquerda |
| 3 | `&&` | esquerda |
| 4 | `==`, `!=` | esquerda |
| 5 | `<`, `<=`, `>`, `>=` | esquerda |
| 6 | `+`, `-` (binários) | esquerda |
| 7 | `*`, `/`, `%` | esquerda |
| 8 | `!`, `-` (unário) | não-associativo |
