# Compiladores UnB

Bem-vindo à documentação oficial do projeto prático desenvolvido para a disciplina de **Compiladores** da **Universidade de Brasília (UnB)**.

---

## Visão Geral do Projeto

O objetivo deste projeto é o desenvolvimento de um compilador/transpilador com *frontend* construído utilizando **C**, **Flex** e **Bison**. O compilador tem como meta processar uma linguagem fonte (um subconjunto expressivo e simplificado de **C**) e realizar a análise e geração de código alvo para **Python 3**.

### Recursos Suportados pela Linguagem
- **Tipos de dados primitivos:** `int`, `float`, `char`, `bool` e `void`.
- **Coerção e compatibilidade:** Promoção implícita de `int` para `float` em atribuições e expressões aritméticas.
- **Estruturas de controle de fluxo:** Condicionais (`if`, `if-else`) e laços de repetição (`while`, `for`, `break`, `continue`).
- **Funções e recursão:** Declaração e definição de funções com múltiplos parâmetros, retorno obrigatório em funções não-void, recursão simples e recursão mútua.
- **Vetores unidimensionais (1D):** Declaração estática (`int v[10]`), inferência por literal string (`char s[] = "oi"`) e indexação com checagem estática de limites.
- **Operadores:** Aritméticos (`+`, `-`, `*`, `/`, `%`), relacionais (`==`, `!=`, `<`, `<=`, `>`, `>=`), lógicos (`&&`, `||`, `!`) e atribuições compostas (`+=`, `-=`, `*=`, `/=`).
- **Ponto de entrada:** Validação obrigatória da função `main` no escopo global.

> **Decisão de Arquitetura:** Para viabilizar a transpilação direta e idiomática para Python sem a necessidade de simular um emulador de memória RAM, a linguagem **não** implementa manipulação direta de ponteiros (`malloc`, `free`, `*ptr`, `&var`), `structs`, `switch-case` nem sombreamento de variáveis dentro do escopo da função. Consulte [Decisões de Projeto](decisoes.md) para o detalhamento técnico.

---

## Estrutura da Pipeline

```mermaid
flowchart LR
    A["Código Fonte (.c)"] --> B["Analisador Léxico (Flex)"]
    B --> C["Analisador Sintático (Bison)"]
    C --> D["Analisador Semântico & AST"]
    D --> E["Geração de Código (Python 3)"]
```

1. **Analisador Léxico (Scanner):** Responsável pela conversão do fluxo de caracteres em tokens padronizados, descarte de comentários/espaços e reporte detalhado de erros léxicos com linha e coluna.
2. **Analisador Sintático (Parser):** Aplica a Gramática Livre de Contexto (GLC) sobre os tokens, validando a estrutura sintática das declarações, comandos e expressões, instanciando a AST.
3. **Analisador Semântico:** Resolução de escopos hierárquicos via Tabela de Símbolos, validação de l-values, segurança aritmética, detecção estática de divisão por zero, limites de vetores, análise de fluxo de retorno e ponto de entrada `main`.
4. **Geração de Código:** Tradução da representação intermediária/AST para código executável e idiomático em **Python 3**.

---

## Guias e Especificações

Navegue pela documentação técnica através das seções:

- [**Decisões de Projeto**](decisoes.md): Registro das decisões de arquitetura e justificativa técnica da exclusão de ponteiros, structs e switch-case.
- [**Especificação Léxica (Tokens e Expressões Regulares)**](tokens.md): Tabela de palavras reservadas, operadores, literais e validação das expressões regulares do Flex.
- [**Gramática Livre de Contexto (GLC)**](gramatica.md): Especificação formal em EBNF de todas as produções sintáticas implementadas no Bison.
- [**Árvore Sintática Abstrata (AST) e Tabela de Símbolos**](ast_e_tabela_simbolos.md): Estrutura de nós da AST, tabela de símbolos com múltiplos escopos, regras completas do analisador semântico e CLI.
