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

# Casos de teste E2E (combinados de main e dev)
TESTS = [
    ('\n        int main() {\n            int a = 10;\n            int b = 3;\n            int c = 2;\n            int res = a + b * c - 8 / 2 + 17 % 5;\n            return res;\n        }\n        ', 14, None, 'Aritmética: precedência com +, -, *, / e %'),
    ('\n        int main() {\n            int x = 5;\n            int y = 10;\n            bool cond = (x < y && y != 0) || !(x == 5);\n            if (cond && !(x > y)) {\n                return 1;\n            }\n            return 0;\n        }\n        ', 1, None, 'Expressões lógicas e relacionais (&&, ||, !, comparações)'),
    ('\n        int fatorial(int n) {\n            if (n <= 1) {\n                return 1;\n            }\n            return n * fatorial(n - 1);\n        }\n\n        int main() {\n            return fatorial(5);\n        }\n        ', 120, None, 'Recursão: fatorial(5) == 120'),
    ('\n        int fib(int n) {\n            if (n <= 1) {\n                return n;\n            }\n            return fib(n - 1) + fib(n - 2);\n        }\n\n        int main() {\n            return fib(7);\n        }\n        ', 13, None, 'Recursão: fib(7) == 13'),
    ('\n        int soma3(int a, int b, int c) {\n            return a + b + c;\n        }\n\n        int combina(int x, int y, int z, int w) {\n            return soma3(x + y, z * 2, w - 1);\n        }\n\n        int main() {\n            return combina(2, 3, soma3(1, 2, 3), 10);\n        }\n        ', 26, None, 'Múltiplos parâmetros e chamadas aninhadas'),
    ('\n        int mdc(int a, int b) {\n            if (b == 0) {\n                return a;\n            }\n            return mdc(b, a % b);\n        }\n\n        int main() {\n            return mdc(48, 18);\n        }\n        ', 6, None, 'Recursão: MDC de Euclides mdc(48, 18) == 6'),
    ('\n        int ack(int m, int n) {\n            if (m == 0) {\n                return n + 1;\n            }\n            if (n == 0) {\n                return ack(m - 1, 1);\n            }\n            return ack(m - 1, ack(m, n - 1));\n        }\n\n        int main() {\n            return ack(2, 3);\n        }\n        ', 9, None, 'Recursão aninhada: Ackermann ack(2, 3) == 9'),
    ('\n        int pot(int base, int exp) {\n            if (exp == 0) {\n                return 1;\n            }\n            if (exp % 2 == 0) {\n                int meio = pot(base, exp / 2);\n                return meio * meio;\n            }\n            return base * pot(base, exp - 1);\n        }\n\n        int main() {\n            return pot(3, 4);\n        }\n        ', 81, None, 'Recursão: exponenciação rápida pot(3, 4) == 81'),
    ('\n        int main() {\n            int x = -5;\n            x = -x + (10 * 2);\n            x += 5;\n            x -= 3;\n            x *= 2;\n            x /= 3;\n            return x;\n        }\n        ', 18, None, 'Operadores unários e atribuições compostas (+=, -=, *=, /=)'),
    ('\n        int main() {\n            int a = 0;\n            int b = 10;\n            if ((a != 0 && (b / a) > 1) || (a == 0 && b == 10)) {\n                return 1;\n            }\n            return 0;\n        }\n        ', 1, None, 'Curto-circuito lógico evitando divisão por zero'),
    ('\n        int passos = 0;\n\n        void collatz(int n) {\n            if (n == 1) {\n                return;\n            }\n            passos += 1;\n            if (n % 2 == 0) {\n                collatz(n / 2);\n            } else {\n                collatz(3 * n + 1);\n            }\n        }\n\n        int main() {\n            collatz(6);\n            return passos;\n        }\n        ', 8, None, 'Recursão com variável global: passos de Collatz(6) == 8'),
    ('\n        int main() {\n            int n = 5;\n            int soma = 0;\n            while (n > 0) {\n                soma += n;\n                n -= 1;\n            }\n            return soma;\n        }\n        ', 15, None, 'Laço while: acumulador e contagem regressiva'),
    ('\n        int main() {\n            int total = 0;\n            for (int i = 1; i <= 10; i += 1) {\n                total += i;\n            }\n            return total;\n        }\n        ', 55, None, 'Laço for: somatório com inicialização local e incremento'),
    ('\n        int main() {\n            int soma = 0;\n            for (int i = 0; i < 10; i += 1) {\n                if (i % 2 == 0) {\n                    continue;\n                }\n                if (i > 7) {\n                    break;\n                }\n                soma += i;\n            }\n            return soma;\n        }\n        ', 16, None, 'Controle de laço: break e continue dentro de for'),
    ('\n        int main() {\n            int v[6];\n            for (int i = 0; i < 6; i += 1) {\n                v[i] = i * 3 + 1;\n            }\n            int alvo = 13;\n            int idx = -1;\n            int j = 0;\n            while (j < 6) {\n                if (v[j] == alvo) {\n                    idx = j;\n                    break;\n                }\n                j += 1;\n            }\n            if (idx == 4 && v[0] == 1 && v[5] == 16) {\n                return idx + v[2];\n            }\n            return 0;\n        }\n        ', 11, None, 'Vetor 1D: preenchimento, busca linear e indexação'),
    ('\n        int main() {\n            int arr[5];\n            arr[0] = 40;\n            arr[1] = 10;\n            arr[2] = 50;\n            arr[3] = 20;\n            arr[4] = 30;\n\n            for (int i = 0; i < 5; i += 1) {\n                for (int j = 0; j < 4 - i; j += 1) {\n                    if (arr[j] > arr[j + 1]) {\n                        int tmp = arr[j];\n                        arr[j] = arr[j + 1];\n                        arr[j + 1] = tmp;\n                    }\n                }\n            }\n\n            bool ordenado = true;\n            for (int k = 0; k < 4; k += 1) {\n                if (arr[k] > arr[k + 1]) {\n                    ordenado = false;\n                }\n            }\n\n            if (ordenado && arr[0] == 10 && arr[4] == 50) {\n                return arr[2];\n            }\n            return 0;\n        }\n        ', 30, None, 'Vetor 1D: Bubble Sort com laços aninhados'),
    ('\n        int main() {\n            int a[6];\n            for (int i = 0; i < 6; i += 1) {\n                a[i] = i + 1;\n            }\n            int esq = 0;\n            int dir = 5;\n            while (esq < dir) {\n                int aux = a[esq];\n                a[esq] = a[dir];\n                a[dir] = aux;\n                esq += 1;\n                dir -= 1;\n            }\n            return a[0] * 10 + a[5];\n        }\n        ', 61, None, 'Vetor 1D: inversão in-place com while'),
    ('int main() { return 0; }', 0, None, 'Retorno simples 0 em main'),
    ('int main() { return 42; }', 42, None, 'Retorno inteiro não-zero em main'),
    ('void main() { int x = 1; }', 0, None, 'Ponto de entrada void main() sem return'),
    ('int main() { int a = 2 + 3 * 4 - 10 / 3; return a % 5 + 10; }', 11, None, 'Aritmética inteira, precedência, divisão inteira (//) e módulo'),
    ('int main() { int x = -5; int y = 20; return y + x; }', 15, None, 'Operador unário de negação (-)'),
    ('\n        int main() {\n            int i = 5;\n            float f = i / 2.0;\n            if (f > 2.4 && f < 2.6) {\n                return 1;\n            }\n            return 0;\n        }\n        ', 1, None, 'Coerção int -> float e divisão real'),
    ('\n        int main() {\n            bool a = true;\n            bool b = false;\n            if ((a && !b) || false) {\n                if (!(1 == 2) && (3 <= 3) && (4 != 5)) {\n                    return 7;\n                }\n            }\n            return 0;\n        }\n        ', 7, None, 'Expressões booleanas, lógicas (!, &&, ||) e relacionais'),
    ('\n        int classifica(int x) {\n            if (x < 0) {\n                return 1;\n            } else if (x == 0) {\n                return 2;\n            } else {\n                return 3;\n            }\n        }\n        int main() {\n            return classifica(-1) * 10 + classifica(0) * 2 + classifica(5);\n        }\n        ', 17, None, 'Condicionais if / else if / else'),
    ('\n        int main() {\n            int i = 0;\n            int soma = 0;\n            while (i < 10) {\n                i += 1;\n                if (i % 2 == 0) {\n                    continue;\n                }\n                if (i > 7) {\n                    break;\n                }\n                soma += i;\n            }\n            return soma;\n        }\n        ', 16, None, 'Laço while com continue e break (1+3+5+7 = 16)'),
    ('\n        int main() {\n            int soma = 0;\n            for (int i = 0; i < 6; i += 1) {\n                if (i == 3) {\n                    continue;\n                }\n                soma += i;\n            }\n            return soma;\n        }\n        ', 12, None, 'Laço for com continue executando o passo (0+1+2+4+5 = 12)'),
    ('\n        int main() {\n            int x = 0;\n            for (;;) {\n                x += 1;\n                if (x == 5) {\n                    break;\n                }\n            }\n            return x;\n        }\n        ', 5, None, 'Laço for infinito for(;;) com break'),
    ('\n        int g_v[4];\n        int main() {\n            int v[4];\n            int acc = 0;\n            for (int i = 0; i < 4; i += 1) {\n                v[i] = i + 1;\n                g_v[i] = v[i] * 2;\n            }\n            v[2] *= 10;\n            for (int j = 0; j < 4; j += 1) {\n                acc += v[j];\n            }\n            return acc + g_v[3];\n        }\n        ', 45, None, 'Vetor 1D local e global, atribuição composta em elemento (1+2+30+4 + 8 = 45)'),
    ('\n        int fat(int n) {\n            if (n <= 1) {\n                return 1;\n            }\n            return n * fat(n - 1);\n        }\n        int main() {\n            return fat(5);\n        }\n        ', 120, None, 'Função recursiva (fatorial de 5 = 120)'),
    ('\n        bool eh_par(int n) {\n            if (n == 0) return true;\n            return eh_impar(n - 1);\n        }\n        bool eh_impar(int n) {\n            if (n == 0) return false;\n            return eh_par(n - 1);\n        }\n        int main() {\n            if (eh_par(6) && eh_impar(5)) {\n                return 1;\n            }\n            return 0;\n        }\n        ', 1, None, 'Recursão mútua (eh_par / eh_impar)'),
    ('\n        int g = 10;\n        void incrementa() {\n            g += 5;\n        }\n        int sombreia(int g) {\n            g += 100;\n            return g;\n        }\n        void vazia() {}\n        int main() {\n            vazia();\n            incrementa();\n            int local = sombreia(1);\n            return g + local;\n        }\n        ', 116, None, 'Variáveis globais, sombreamento por parâmetro e função vazia (15 + 101 = 116)'),
    ('// Exemplo de programa em C para teste do Scanner\nint main() {\n    float salario = 1250.50;\n    char letra = \'A\';\n    char mensagem[] = "Olá, Compiladores!";\n    \n    if (salario > 1000.0) {\n        salario += 150.0;\n    }\n    \n    return 0;\n}\n', 0, None, 'Programa completo tests/example.c (float, char, char[] inferido, if)'),
    ('\n        int main() {\n            char s[] = "oi";\n            if (s[0] == \'o\' && s[1] == \'i\') {\n                s[1] = \'k\';\n                if (s[1] == \'k\') {\n                    return 1;\n                }\n            }\n            return 0;\n        }\n        ', 1, None, 'Vetor char[] com tamanho inferido por string, leitura e mutação'),
    ('\n        void noop(void) {\n            return;\n        }\n        int main(void) {\n            noop();\n            return 9;\n        }\n        ', 9, None, 'Funções com lista de parâmetros (void)'),
    ('\n        int main() {\n            int a = 1, b = 2, c;\n            c = a + b + 4;\n            return c;\n        }\n        ', 7, None, 'Múltiplos declaradores na mesma linha com valor padrão'),
    ('\n        float media(float a, float b) {\n            return (a + b) / 2.0;\n        }\n        int main() {\n            float m = media(3, 5);\n            if (m == 4.0) {\n                return 1;\n            }\n            return 0;\n        }\n        ', 1, None, 'Promoção implícita int -> float em parâmetros e expressão'),
    ('\n        int main() {\n            int x = 100;\n            x -= 20;\n            x *= 2;\n            x /= 5;\n            float f = 9.0;\n            f /= 2;\n            if (x == 32 && f == 4.5) {\n                return x;\n            }\n            return 0;\n        }\n        ', 32, None, 'Atribuições compostas (-=, *=, /= inteiro -> //= e /= float -> /=)'),
    ('\n        int main() {\n            int x = 0;\n            if (false && (10 / x == 0)) {\n                return 1;\n            }\n            if (true || (10 / x == 0)) {\n                return 2;\n            }\n            return 0;\n        }\n        ', 2, None, 'Avaliação em curto-circuito de && e ||'),
    ('\n        int classifica(int n) {\n            if (n > 0)\n                if (n % 2 == 0)\n                    return 2;\n                else\n                    return 1;\n            else\n                return 0;\n        }\n        int main() {\n            return classifica(4) * 10 + classifica(3);\n        }\n        ', 21, None, 'Dangling else preservado corretamente em Python'),
    ('\n        int main() {\n            int total = 0;\n            for (int i = 0; i < 4; i += 1) {\n                for (int j = 0; j < 4; j += 1) {\n                    if (j == 2) {\n                        break;\n                    }\n                    total += 1;\n                }\n            }\n            return total;\n        }\n        ', 8, None, 'Laços aninhados: break afeta apenas o laço mais interno (4 * 2 = 8)'),
    ('\n        int main() {\n            int v[5];\n            v[0] = 5; v[1] = 2; v[2] = 4; v[3] = 1; v[4] = 3;\n            for (int i = 0; i < 5; i += 1) {\n                for (int j = 0; j < 4 - i; j += 1) {\n                    if (v[j] > v[j + 1]) {\n                        int tmp = v[j];\n                        v[j] = v[j + 1];\n                        v[j + 1] = tmp;\n                    }\n                }\n            }\n            return v[0] * 100 + v[2] * 10 + v[4];\n        }\n        ', 135, None, 'Algoritmo completo: Bubble Sort em vetor 1D'),
    ('\n        int main() {\n            int v[6];\n            v[0] = 2; v[1] = 5; v[2] = 9; v[3] = 14; v[4] = 20; v[5] = 31;\n            int alvo = 20;\n            int esq = 0;\n            int dir = 5;\n            int achou = -1;\n            while (esq <= dir) {\n                int meio = (esq + dir) / 2;\n                if (v[meio] == alvo) {\n                    achou = meio;\n                    break;\n                } else if (v[meio] < alvo) {\n                    esq = meio + 1;\n                } else {\n                    dir = meio - 1;\n                }\n            }\n            return achou;\n        }\n        ', 4, None, 'Algoritmo completo: Busca Binária em vetor ordenado'),
    ('\n        int mdc(int a, int b) {\n            while (b != 0) {\n                int r = a % b;\n                a = b;\n                b = r;\n            }\n            return a;\n        }\n        int main() {\n            return mdc(48, 18);\n        }\n        ', 6, None, 'Algoritmo completo: Máximo Divisor Comum (Euclides)'),
    ('\n        bool eh_primo(int n) {\n            if (n < 2) return false;\n            for (int d = 2; d * d <= n; d += 1) {\n                if (n % d == 0) {\n                    return false;\n                }\n            }\n            return true;\n        }\n        int main() {\n            int cont = 0;\n            for (int i = 1; i <= 20; i += 1) {\n                if (eh_primo(i)) {\n                    cont += 1;\n                }\n            }\n            return cont;\n        }\n        ', 8, None, 'Algoritmo completo: Contagem de números primos até 20 (8 primos)'),
    ('\n        int gv[3];\n        void preenche() {\n            gv[0] = 10;\n            gv[1] = 20;\n            gv[2] = 30;\n        }\n        int main() {\n            preenche();\n            return gv[0] + gv[1] + gv[2];\n        }\n        ', 60, None, 'Vetor global modificado dentro de função auxiliar'),
    ('\n        int g = 5;\n        int f(int x) {\n            int g = 20;\n            return g + x;\n        }\n        int main() {\n            int r = f(3);\n            return g + r;\n        }\n        ', 28, None, 'Sombreamento de variável global por variável local (sem global)'),
    ("\n        int main() {\n            int i;\n            float f;\n            char c;\n            bool b;\n            if (i == 0 && f == 0.0 && !b) {\n                c = 'Z';\n                if (c == 'Z') {\n                    return 1;\n                }\n            }\n            return 0;\n        }\n        ", 1, None, 'Valores padrão de variáveis locais sem inicializador'),
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
