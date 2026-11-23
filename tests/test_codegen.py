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
TESTS = [
    (
        """
        int main() {
            int a = 10;
            int b = 3;
            int c = 2;
            int res = a + b * c - 8 / 2 + 17 % 5;
            return res;
        }
        """,
        14,
        None,
        "Aritmética: precedência com +, -, *, / e %",
    ),
    (
        """
        int main() {
            int x = 5;
            int y = 10;
            bool cond = (x < y && y != 0) || !(x == 5);
            if (cond && !(x > y)) {
                return 1;
            }
            return 0;
        }
        """,
        1,
        None,
        "Expressões lógicas e relacionais (&&, ||, !, comparações)",
    ),
    (
        """
        int fatorial(int n) {
            if (n <= 1) {
                return 1;
            }
            return n * fatorial(n - 1);
        }

        int main() {
            return fatorial(5);
        }
        """,
        120,
        None,
        "Recursão: fatorial(5) == 120",
    ),
    (
        """
        int fib(int n) {
            if (n <= 1) {
                return n;
            }
            return fib(n - 1) + fib(n - 2);
        }

        int main() {
            return fib(7);
        }
        """,
        13,
        None,
        "Recursão: fib(7) == 13",
    ),
    (
        """
        int soma3(int a, int b, int c) {
            return a + b + c;
        }

        int combina(int x, int y, int z, int w) {
            return soma3(x + y, z * 2, w - 1);
        }

        int main() {
            return combina(2, 3, soma3(1, 2, 3), 10);
        }
        """,
        26,
        None,
        "Múltiplos parâmetros e chamadas aninhadas",
    ),
    (
        """
        int mdc(int a, int b) {
            if (b == 0) {
                return a;
            }
            return mdc(b, a % b);
        }

        int main() {
            return mdc(48, 18);
        }
        """,
        6,
        None,
        "Recursão: MDC de Euclides mdc(48, 18) == 6",
    ),
    (
        """
        int ack(int m, int n) {
            if (m == 0) {
                return n + 1;
            }
            if (n == 0) {
                return ack(m - 1, 1);
            }
            return ack(m - 1, ack(m, n - 1));
        }

        int main() {
            return ack(2, 3);
        }
        """,
        9,
        None,
        "Recursão aninhada: Ackermann ack(2, 3) == 9",
    ),
    (
        """
        int pot(int base, int exp) {
            if (exp == 0) {
                return 1;
            }
            if (exp % 2 == 0) {
                int meio = pot(base, exp / 2);
                return meio * meio;
            }
            return base * pot(base, exp - 1);
        }

        int main() {
            return pot(3, 4);
        }
        """,
        81,
        None,
        "Recursão: exponenciação rápida pot(3, 4) == 81",
    ),
    (
        """
        int main() {
            int x = -5;
            x = -x + (10 * 2);
            x += 5;
            x -= 3;
            x *= 2;
            x /= 3;
            return x;
        }
        """,
        18,
        None,
        "Operadores unários e atribuições compostas (+=, -=, *=, /=)",
    ),
    (
        """
        int main() {
            int a = 0;
            int b = 10;
            if ((a != 0 && (b / a) > 1) || (a == 0 && b == 10)) {
                return 1;
            }
            return 0;
        }
        """,
        1,
        None,
        "Curto-circuito lógico evitando divisão por zero",
    ),
    (
        """
        int passos = 0;

        void collatz(int n) {
            if (n == 1) {
                return;
            }
            passos += 1;
            if (n % 2 == 0) {
                collatz(n / 2);
            } else {
                collatz(3 * n + 1);
            }
        }

        int main() {
            collatz(6);
            return passos;
        }
        """,
        8,
        None,
        "Recursão com variável global: passos de Collatz(6) == 8",
    ),
]


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
