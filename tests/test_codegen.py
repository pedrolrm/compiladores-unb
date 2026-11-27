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


# Casos de teste E2E (código C, código de saída esperado, stdout esperado, nome)
TESTS = [
    (
        "int main() { return 0; }",
        0,
        None,
        "Retorno simples 0 em main",
    ),
    (
        "int main() { return 42; }",
        42,
        None,
        "Retorno inteiro não-zero em main",
    ),
    (
        "void main() { int x = 1; }",
        0,
        None,
        "Ponto de entrada void main() sem return",
    ),
    (
        "int main() { int a = 2 + 3 * 4 - 10 / 3; return a % 5 + 10; }",
        11,
        None,
        "Aritmética inteira, precedência, divisão inteira (//) e módulo",
    ),
    (
        "int main() { int x = -5; int y = 20; return y + x; }",
        15,
        None,
        "Operador unário de negação (-)",
    ),
    (
        """
        int main() {
            int i = 5;
            float f = i / 2.0;
            if (f > 2.4 && f < 2.6) {
                return 1;
            }
            return 0;
        }
        """,
        1,
        None,
        "Coerção int -> float e divisão real",
    ),
    (
        """
        int main() {
            bool a = true;
            bool b = false;
            if ((a && !b) || false) {
                if (!(1 == 2) && (3 <= 3) && (4 != 5)) {
                    return 7;
                }
            }
            return 0;
        }
        """,
        7,
        None,
        "Expressões booleanas, lógicas (!, &&, ||) e relacionais",
    ),
    (
        """
        int classifica(int x) {
            if (x < 0) {
                return 1;
            } else if (x == 0) {
                return 2;
            } else {
                return 3;
            }
        }
        int main() {
            return classifica(-1) * 10 + classifica(0) * 2 + classifica(5);
        }
        """,
        17,
        None,
        "Condicionais if / else if / else",
    ),
    (
        """
        int main() {
            int i = 0;
            int soma = 0;
            while (i < 10) {
                i += 1;
                if (i % 2 == 0) {
                    continue;
                }
                if (i > 7) {
                    break;
                }
                soma += i;
            }
            return soma;
        }
        """,
        16,
        None,
        "Laço while com continue e break (1+3+5+7 = 16)",
    ),
    (
        """
        int main() {
            int soma = 0;
            for (int i = 0; i < 6; i += 1) {
                if (i == 3) {
                    continue;
                }
                soma += i;
            }
            return soma;
        }
        """,
        12,
        None,
        "Laço for com continue executando o passo (0+1+2+4+5 = 12)",
    ),
    (
        """
        int main() {
            int x = 0;
            for (;;) {
                x += 1;
                if (x == 5) {
                    break;
                }
            }
            return x;
        }
        """,
        5,
        None,
        "Laço for infinito for(;;) com break",
    ),
    (
        """
        int g_v[4];
        int main() {
            int v[4];
            int acc = 0;
            for (int i = 0; i < 4; i += 1) {
                v[i] = i + 1;
                g_v[i] = v[i] * 2;
            }
            v[2] *= 10;
            for (int j = 0; j < 4; j += 1) {
                acc += v[j];
            }
            return acc + g_v[3];
        }
        """,
        45,
        None,
        "Vetor 1D local e global, atribuição composta em elemento (1+2+30+4 + 8 = 45)",
    ),
    (
        """
        int fat(int n) {
            if (n <= 1) {
                return 1;
            }
            return n * fat(n - 1);
        }
        int main() {
            return fat(5);
        }
        """,
        120,
        None,
        "Função recursiva (fatorial de 5 = 120)",
    ),
    (
        """
        bool eh_par(int n) {
            if (n == 0) return true;
            return eh_impar(n - 1);
        }
        bool eh_impar(int n) {
            if (n == 0) return false;
            return eh_par(n - 1);
        }
        int main() {
            if (eh_par(6) && eh_impar(5)) {
                return 1;
            }
            return 0;
        }
        """,
        1,
        None,
        "Recursão mútua (eh_par / eh_impar)",
    ),
    (
        """
        int g = 10;
        void incrementa() {
            g += 5;
        }
        int sombreia(int g) {
            g += 100;
            return g;
        }
        void vazia() {}
        int main() {
            vazia();
            incrementa();
            int local = sombreia(1);
            return g + local;
        }
        """,
        116,
        None,
        "Variáveis globais, sombreamento por parâmetro e função vazia (15 + 101 = 116)",
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
