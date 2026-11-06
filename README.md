# Compiladores UnB

Repositório destinado ao projeto da disciplina de Compiladores (Universidade de Brasília - UnB).

## Escopo e Definição do Projeto

Este projeto consiste no desenvolvimento de um **Compilador/Transpilador** construído na linguagem **C**, com frontend gerado via **Flex** e **Bison**, tendo como objetivo traduzir um subconjunto estruturado da linguagem C para **Python 3**.

### Recursos da Linguagem Suportados
- **Tipos primitivos:** `int`, `float`, `char`, `bool` e `void`.
- **Coerção de tipos:** Promoção implícita de `int` para `float` em atribuições e expressões aritméticas.
- **Vetores estáticos (1D):** Declaração com tamanho fixo positivo (`int v[10]`), tamanho inferido por literal string (`char s[] = "oi"`) e indexação com checagem estática de limites.
- **Estruturas de controle de fluxo:** Condicionais (`if`, `if-else`) e laços de repetição (`while`, `for`, `break`, `continue`).
- **Funções:** Declaração com múltiplos parâmetros, verificação de aridade, chamadas recursivas, recursão mútua e análise de retorno obrigatório em funções não-void.
- **Ponto de entrada canônico:** Validação obrigatória da função `main` no escopo global (com retorno `int` ou `void`).

> **Nota de Escopo:** Para viabilizar a transpilação direta e idiomática para Python sem necessidade de um emulador de memória RAM, construções como ponteiros/alocação dinâmica (`malloc`/`free`), `structs`, `switch-case` e sombreamento de variáveis dentro da mesma função foram deliberadamente excluídos do escopo (consulte [Decisões de Projeto](docs/decisoes.md)).

### Pipeline do Compilador
1. **Analisador Léxico (Scanner):** Identificação de Tokens via Flex (palavras-chave, identificadores, literais, operadores).
2. **Analisador Sintático (Parser):** Reconhecimento de gramática livre de contexto via Bison e construção da Árvore Sintática Abstrata (AST).
3. **Analisador Semântico:** Tabela de símbolos com múltiplos escopos, validação de l-values, restrições aritméticas, detecção de divisão por zero, verificação de limites de vetores, fluxo de retorno e validação da função `main`.
4. **Geração de Código:** Transpilação da AST para código Python 3 executável e legível.

---

## Execução Local

### Pré-requisitos
- Compilador C (`gcc` ou `clang`).
- Geradores de analisadores: `flex` e `bison` (ou `win_flex` e `win_bison` no Windows).
- Utilitário `make` (ou `mingw32-make`).
- `python3` (para execução das suítes de teste).

### Passo a Passo

1. Compile o projeto utilizando o Makefile:
   ```bash
   make
   ```

2. Execute o compilador com uma das opções disponíveis:
   ```bash
   ./compilador [opções] <arquivo.c>
   ```

   **Opções disponíveis:**
   - `--scan` (`-s`): Executa o Scanner e exibe a lista formatada de tokens.
   - `--parse` (`-p`): Executa o Parser e valida a sintaxe.
   - `--ast` (`-a`): Exibe a Árvore Sintática Abstrata (AST) formatada.
   - `--symbols` (`-t`): Exibe a Tabela de Símbolos hierárquica por escopos.
   - `--check` (`-c`): Executa a validação sintática e semântica completa.

3. Para executar as suítes de testes automatizados:
   ```bash
   make test
   ```
