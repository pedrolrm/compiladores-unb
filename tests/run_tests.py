#!/usr/bin/env python3
"""
Suite de Testes Automatizados para o Analisador Léxico (Scanner)
Disciplina de Compiladores - UnB
"""

import os
import subprocess
import sys
import time

GREEN = "\033[92m"
RED = "\033[91m"
YELLOW = "\033[93m"
CYAN = "\033[96m"
BOLD = "\033[1m"
RESET = "\033[0m"

def find_compiler():
    candidates = [
        os.path.join(".", "compilador"),
        os.path.join(".", "compilador.exe"),
        os.path.join("..", "compilador"),
        os.path.join("..", "compilador.exe"),
    ]
    for c in candidates:
        if os.path.isfile(c) and os.access(c, os.X_OK):
            return c
        if os.path.isfile(c) and sys.platform == "win32":
            return c
    return None

def run_test_case(compiler, filepath, expect_success, expected_tokens=None, expected_errors=None):
    cmd = [compiler, filepath]
    try:
        proc = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, timeout=5)
    except Exception as e:
        return False, f"Falha na execução: {e}"

    stdout = proc.stdout
    stderr = proc.stderr
    exit_code = proc.returncode

    if expect_success:
        if exit_code != 0:
            return False, f"Esperava código de saída 0, obteve {exit_code}. Stderr: {stderr.strip()}"
        if "Erro léxico" in stderr:
            return False, f"Erros léxicos inesperados reportados em stderr: {stderr.strip()}"
        if expected_tokens:
            for tok in expected_tokens:
                if tok not in stdout:
                    return False, f"Token esperado '{tok}' não encontrado na saída."
    else:
        if exit_code == 0:
            return False, f"Esperava erro (código != 0), mas execução retornou 0."
        if "Erro léxico" not in stderr:
            return False, f"Esperava mensagem 'Erro léxico' em stderr, obteve: '{stderr.strip()}'"
        if expected_errors:
            for err in expected_errors:
                if err not in stderr:
                    return False, f"Padrão de erro esperado '{err}' não encontrado em stderr: {stderr.strip()}"

    return True, "OK"

def main():
    print(f"{BOLD}{CYAN}======================================================================{RESET}")
    print(f"{BOLD}{CYAN}  Executando Suíte de Testes Automatizados do Scanner (Compiladores UnB)  {RESET}")
    print(f"{BOLD}{CYAN}======================================================================{RESET}\n")

    compiler = find_compiler()
    if not compiler:
        print(f"{YELLOW}Compilador não encontrado pré-compilado. Tentando compilar via make...{RESET}")
        try:
            subprocess.run(["make", "compilador"], check=True)
            compiler = find_compiler()
        except Exception:
            pass

    if not compiler:
        print(f"{RED}Erro: Binário 'compilador' não encontrado e não pôde ser compilado.{RESET}")
        print("Por favor, execute 'make' ou compile o projeto antes de rodar os testes.")
        sys.exit(1)

    print(f"Utilizando binário: {BOLD}{compiler}{RESET}\n")

    tests = [
        # Casos Válidos
        {
            "name": "Todos os Tokens Válidos",
            "file": os.path.join("tests", "valid_all_tokens.c"),
            "expect_success": True,
            "expected_tokens": ["KW_INT", "KW_FLOAT", "KW_CHAR", "KW_VOID", "KW_DOUBLE", "KW_BOOL",
                                "KW_STRUCT", "KW_TYPEDEF", "KW_IF", "KW_ELSE", "KW_WHILE", "KW_FOR",
                                "KW_RETURN", "IDENTIFIER", "INT_LITERAL", "FLOAT_LITERAL", "OP_ASSIGN",
                                "OP_EQ", "OP_AND", "OP_PLUS_ASSIGN", "DELIM_SEMICOLON"],
        },
        {
            "name": "Comentários de Linha e Bloco",
            "file": os.path.join("tests", "valid_comments.c"),
            "expect_success": True,
            "expected_tokens": ["KW_INT", "KW_FLOAT", "KW_CHAR", "IDENTIFIER", "INT_LITERAL"],
        },
        {
            "name": "Casos de Borda Válidos (Floats, Escapes, Identificadores)",
            "file": os.path.join("tests", "valid_edge_cases.c"),
            "expect_success": True,
            "expected_tokens": ["FLOAT_LITERAL", "STRING_LITERAL", "CHAR_LITERAL", "IDENTIFIER", "OP_INC"],
        },
        {
            "name": "Arquivo de Exemplo Básico",
            "file": os.path.join("tests", "example.c"),
            "expect_success": True,
            "expected_tokens": ["KW_INT", "IDENTIFIER", "FLOAT_LITERAL", "STRING_LITERAL", "OP_GT"],
        },
        {
            "name": "Arquivo de Teste de Palavras-Chave",
            "file": os.path.join("tests", "keywords.txt"),
            "expect_success": True,
            "expected_tokens": ["KW_INT", "KW_FLOAT", "KW_CHAR", "KW_IF", "KW_ELSE", "KW_WHILE", "KW_CONTINUE"],
        },
        {
            "name": "Arquivo de Teste de Literais",
            "file": os.path.join("tests", "literals.txt"),
            "expect_success": True,
            "expected_tokens": ["INT_LITERAL", "FLOAT_LITERAL", "STRING_LITERAL", "CHAR_LITERAL"],
        },

        # Casos com Erro Léxico
        {
            "name": "Erro: Caracteres Inválidos (@, $, #, ~, `)",
            "file": os.path.join("tests", "error_invalid_chars.c"),
            "expect_success": False,
            "expected_errors": ["caractere inválido '@'", "caractere inválido '$'", "caractere inválido '#'"],
        },
        {
            "name": "Erro: String Literal Não Fechada",
            "file": os.path.join("tests", "error_unclosed_string.c"),
            "expect_success": False,
            "expected_errors": ["string literal não fechada"],
        },
        {
            "name": "Erro: Literal de Caractere Não Fechado",
            "file": os.path.join("tests", "error_unclosed_char.c"),
            "expect_success": False,
            "expected_errors": ["literal de caractere não fechado"],
        },
        {
            "name": "Erro: Comentário de Bloco Não Fechado no EOF",
            "file": os.path.join("tests", "error_unclosed_comment.c"),
            "expect_success": False,
            "expected_errors": ["comentário de bloco não fechado (EOF inesperado)"],
        },
        {
            "name": "Erro: Bateria de Erros Léxicos Variados",
            "file": os.path.join("tests", "test_lexical_errors.c"),
            "expect_success": False,
            "expected_errors": ["Erro léxico"],
        },
    ]

    passed = 0
    failed = 0
    start_time = time.time()

    for idx, test in enumerate(tests, 1):
        if not os.path.exists(test["file"]):
            print(f"[{idx:02d}/{len(tests):02d}] {YELLOW}[SKIP]{RESET} {test['name']} - Arquivo não encontrado: {test['file']}")
            continue

        ok, msg = run_test_case(
            compiler,
            test["file"],
            test["expect_success"],
            test.get("expected_tokens"),
            test.get("expected_errors"),
        )

        if ok:
            print(f"[{idx:02d}/{len(tests):02d}] {GREEN}[PASS]{RESET} {test['name']}")
            passed += 1
        else:
            print(f"[{idx:02d}/{len(tests):02d}] {RED}[FAIL]{RESET} {test['name']}")
            print(f"       {RED}Motivo: {msg}{RESET}")
            failed += 1

    elapsed = time.time() - start_time
    print(f"\n{BOLD}{CYAN}======================================================================{RESET}")
    print(f"Resultado Final: {GREEN}{passed} aprovados{RESET}, {RED if failed else GREEN}{failed} falhas{RESET} ({elapsed:.2f}s)")
    print(f"{BOLD}{CYAN}======================================================================{RESET}")

    sys.exit(0 if failed == 0 else 1)

if __name__ == "__main__":
    main()
