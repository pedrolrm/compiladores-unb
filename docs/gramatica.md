# Gramática Livre de Contexto (GLC)

Este documento descreve, em EBNF, a gramática da linguagem alvo **tal como
implementada** em [`src/parser.y`](https://github.com/pedrolrm/compiladores-unb/blob/main/src/parser.y) (bison), com os terminais
léxicos definidos em [`src/scanner.l`](https://github.com/pedrolrm/compiladores-unb/blob/main/src/scanner.l) (flex). Cada
produção abaixo corresponde 1:1 a uma regra do parser — este documento não
descreve construções planejadas e ainda não implementadas.

## Notação

- `|` — alternativa.
- `[ x ]` — `x` é opcional.
- `{ x }` — `x` se repete zero ou mais vezes.
- `"texto"` — terminal literal (palavra-chave, operador ou delimitador).
- `IDENTIFICADOR`, `LIT_INT`, `LIT_FLOAT`, `LIT_CHAR`, `LIT_STRING` — terminais
  léxicos com valor semântico (vindos do scanner).

## Terminais léxicos

| Categoria     | Lexemas |
|---------------|---------|
| Palavras-chave de tipo | `int`, `float`, `char`, `bool`, `void`, `struct` |
| Palavras-chave de controle de fluxo | `if`, `else`, `while`, `for`, `switch`, `case`, `default`, `break`, `continue`, `return` |
| Literais booleanos | `true`, `false` |
| Identificador / literais | `IDENTIFICADOR`, `LIT_INT`, `LIT_FLOAT`, `LIT_CHAR`, `LIT_STRING` |
| Operadores aritméticos | `+`, `-`, `*`, `/`, `%` |
| Operadores relacionais/lógicos | `==`, `!=`, `<`, `<=`, `>`, `>=`, `&&`, `\|\|`, `!` |
| Atribuição | `=`, `+=`, `-=`, `*=`, `/=` |
| Delimitadores | `(`, `)`, `{`, `}`, `[`, `]`, `;`, `,`, `.` |

> **Nota:** o parser declara o token `.` (`DELIM_DOT`, usado no acesso a campo
> de struct em `postfix_expression`), mas o scanner atual
> (`src/scanner.l`) não possui uma regra que emita esse token — o caractere
> `.` cai hoje na regra de caractere inválido. Isso é um gap de sincronização
> entre scanner e parser fora do escopo desta issue (documentação da
> gramática), registrado aqui para ciência da equipe.

As demais palavras-chave lexadas pelo scanner (`double`, `long`, `short`,
`unsigned`, `const`, `typedef`, `do`) e os operadores `++`, `--`, `?`, `:`
(fora do uso de `:` em `case`/`default`) ainda não participam de nenhuma
produção do parser e por isso não aparecem na gramática abaixo.

## Produções

### Programa e comandos

```ebnf
programa            ::= { comando }

comando              ::= declaracao
                        | definicao_funcao
                        | comando_expressao
                        | bloco
                        | comando_selecao
                        | comando_rotulado
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

### Seleção (`if`/`else`, `switch`)

```ebnf
comando_selecao       ::= "if" "(" expressao ")" comando [ "else" comando ]
                        | "switch" "(" expressao ")" comando

comando_rotulado      ::= "case" expressao ":" comando
                        | "default" ":" comando
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

### Declarações, tipos e structs

```ebnf
declaracao             ::= especificador_tipo lista_declaradores_init ";"
                        | especificador_tipo ";"

especificador_tipo     ::= "int" | "float" | "char" | "bool" | especificador_struct

especificador_struct   ::= "struct" IDENTIFICADOR "{" { declaracao } "}"
                        | "struct" IDENTIFICADOR

lista_declaradores_init ::= declarador_init { "," declarador_init }

declarador_init        ::= declarador [ "=" expressao ]

declarador              ::= IDENTIFICADOR { "[" [ expressao ] "]" }
```

> `declarador` cobre variáveis escalares, arrays (`v[10]`), arrays
> multidimensionais (`m[3][4]`) e arrays sem tamanho definido (`buf[]`), via
> repetição de colchetes.

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
                        | expressao_pos "." IDENTIFICADOR

expressao_primaria      ::= IDENTIFICADOR
                        | IDENTIFICADOR "(" lista_argumentos ")"
                        | LIT_INT | LIT_FLOAT | LIT_CHAR | LIT_STRING
                        | "true" | "false"

lista_argumentos        ::= [ expressao { "," expressao } ]
```

> `expressao_pos` (indexação `[]` e acesso a campo `.`) é encadeável, o que
> permite expressões como `a[i].campo`, `m[i][j]` e `f().campo`. O lado
> esquerdo de uma atribuição é restrito a `expressao_pos` (não é possível
> atribuir a uma expressão arbitrária, ex.: `(a + b) = 1` é inválido).

## Precedência e associatividade

Resolvida via as declarações de precedência de `src/parser.y` (da menor para
a maior):

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

Essas regras eliminam a ambiguidade da gramática de expressões (que, sem
precedência, seria ambígua por permitir múltiplas árvores de derivação para
a mesma expressão), garantindo, por exemplo, que `a + b * c` seja
interpretado como `a + (b * c)`.

## Estado do documento

Esta gramática reflete o estado **atual** do parser (branch `dev`). Ela deve
ser atualizada em conjunto com qualquer PR que adicione ou altere produções
em `src/parser.y` (ex.: operador ternário, `++`/`--`, `do-while`, `typedef`,
tipos adicionais), para não divergir do compilador real.
