#!/usr/bin/env python3
"""
Framework de Testes End-to-End (E2E) para Geração de Código Python 3.
Compiladores - UnB (Escopo Simplificado)

Fluxo de validação:
1. Grava o código C em arquivo temporário `.c`.
2. Executa o compilador (`./compilador -o temp.py temp.c`).
3. Executa o código Python gerado (`python3 temp.py`).
4. Valida código de saída da função `main()` e saída padrão (`stdout`).
"""

import os
import subprocess
import sys
import tempfile

GREEN = "\033[92m"
RED = "\033[91m"
RESET = "\033[0m"


def get_compiler_path():
    """Retorna o caminho do binário do compilador compatível com Windows e Linux."""
    exe_name = "compilador.exe" if os.name == "nt" else "compilador"
    return os.path.join(".", exe_name)


COMPILER = get_compiler_path()


def _create_temp_path(suffix):
    """Cria um arquivo temporário fechado para evitar bloqueios no Windows."""
    fd, path = tempfile.mkstemp(suffix=suffix)
    os.close(fd)
    return path


def run_e2e_test(c_code, expected_exit_code, expected_stdout=None,
                 test_name="", compiler=None, timeout=5):
    """
    Compila `c_code` para Python 3, executa o script gerado e valida o resultado.

    Retorna True se o código de saída e (opcionalmente) o stdout coincidirem
    com os valores esperados; False caso contrário.
    """
    label = test_name or "Teste E2E"
    compiler_bin = compiler or COMPILER
    c_path = _create_temp_path(".c")
    py_path = _create_temp_path(".py")

    try:
        with open(c_path, "w", encoding="utf-8") as f:
            f.write(c_code)

        compile_proc = subprocess.run(
            [compiler_bin, "-o", py_path, c_path],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            timeout=timeout,
        )
        if compile_proc.returncode != 0:
            print(
                f"[{RED}FAIL{RESET}] {label} - Erro de compilação "
                f"(código {compile_proc.returncode}):\n{compile_proc.stderr}"
            )
            return False

        if not os.path.exists(py_path) or os.path.getsize(py_path) == 0:
            print(
                f"[{RED}FAIL{RESET}] {label} - Arquivo Python '{py_path}' "
                f"não foi gerado ou está vazio."
            )
            return False

        exec_proc = subprocess.run(
            [sys.executable, py_path],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            timeout=timeout,
        )

        if exec_proc.returncode != expected_exit_code:
            print(
                f"[{RED}FAIL{RESET}] {label} - Código de saída esperado "
                f"{expected_exit_code}, obtido {exec_proc.returncode}.\n"
                f"stderr:\n{exec_proc.stderr}"
            )
            return False

        if expected_stdout is not None and exec_proc.stdout != expected_stdout:
            print(
                f"[{RED}FAIL{RESET}] {label} - Saída padrão divergente.\n"
                f"Esperado: {expected_stdout!r}\n"
                f"Obtido:   {exec_proc.stdout!r}"
            )
            return False

        print(f"[{GREEN}PASS{RESET}] {label}")
        return True
    except subprocess.TimeoutExpired as exc:
        print(f"[{RED}FAIL{RESET}] {label} - Timeout ({exc.timeout}s) excedido.")
        return False
    finally:
        for path in (c_path, py_path):
            if os.path.exists(path):
                os.remove(path)


# Casos de teste E2E (populados nas issues #88 e #89)
TESTS = []


def main():
    print("=" * 60)
    print(" Executando Testes End-to-End de Geração de Código (UnB)")
    print("=" * 60)

    passed = 0
    total = len(TESTS)

    for c_code, expected_exit_code, expected_stdout, name in TESTS:
        if run_e2e_test(c_code, expected_exit_code, expected_stdout, name):
            passed += 1

    print("=" * 60)
    print(f"Resultado Codegen E2E: {passed}/{total} testes aprovados")
    print("=" * 60)

    if passed != total:
        sys.exit(1)


if __name__ == "__main__":
    main()
