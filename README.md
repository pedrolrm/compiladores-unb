# Compiladores UnB

Repositório destinado ao projeto da disciplina de Compiladores (UnB).

## Escopo e Definição do Projeto

Este projeto consiste no desenvolvimento de um **Compilador** construído na linguagem **C**. O compilador terá como objetivo analisar e traduzir uma linguagem alvo (uma variação simplificada de C), com suporte aos seguintes recursos básicos:
- Variáveis locais e globais (`int`, `float`, `char`, `bool`, etc).
- Estruturas de controle de fluxo (`if`, `else`, `while`, `for`, `switch`).
- Estruturas de dados complexas (`struct`, arrays).
- Funções, retornos e chamadas recursivas.

O projeto é dividido em fases, correspondentes à pipeline tradicional de um compilador:
1. **Analisador Léxico (Scanner):** Identificação de Tokens (palavras-chave, identificadores, literais, operadores).
2. **Analisador Sintático (Parser):** Reconhecimento de gramática e geração da Árvore Sintática Abstrata (AST).
3. **Analisador Semântico:** Tabela de símbolos, checagem de tipos e regras de escopo.
4. **Geração de Código:** Geração de código intermediário ou assembly final.

## Execução Local

### Pré-requisitos
Para compilar e executar o projeto localmente, você precisará de:
- Um compilador C (como `gcc` ou `clang`).
- Utilitário `make` (se utilizarmos Makefile futuramente).

### Passo a Passo (Compilação Manual)

Como o projeto está nas fases iniciais e o Makefile definitivo ainda será configurado, você pode compilar os arquivos fonte diretamente utilizando o GCC na pasta `src/`:

1. Clone o repositório:
   ```bash
   git clone https://github.com/pedrolrm/compiladores-unb.git
   cd compiladores-unb
   ```

2. Compile os arquivos C localizados na pasta fonte:
   ```bash
   gcc src/*.c -o compilador
   ```

3. Execute o programa gerado:
   ```bash
   ./compilador
   ```
