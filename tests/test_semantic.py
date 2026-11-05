
#!/usr/bin/env python3
"""
Suíte de Testes Automatizados para a Tabela de Símbolos e Análise Semântica
Compiladores - UnB (Escopo Simplificado)
"""

import os
import subprocess
import sys
import tempfile

GREEN = "\033[92m"
RED = "\033[91m"
RESET = "\033[0m"

COMPILER = "./compilador"


def run_semantic_test(source_code, expect_success, expected_msg=None, test_name=""):
    with tempfile.NamedTemporaryFile(mode="w", suffix=".c", delete=False) as tmp:
        tmp.write(source_code)
        tmp_name = tmp.name

    try:
        proc = subprocess.run(
            [COMPILER, "--check", tmp_name],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            timeout=5
        )

        if expect_success:
            if proc.returncode != 0:
                print(
                    f"[{RED}FAIL{RESET}] {test_name} - "
                    f"Esperava sucesso (0), obteve {proc.returncode}:\n{proc.stderr}"
                )
                return False
        else:
            if proc.returncode == 0:
                print(
                    f"[{RED}FAIL{RESET}] {test_name} - "
                    "Esperava falha semântica, mas execução retornou 0."
                )
                return False

            if expected_msg and expected_msg not in proc.stderr:
                print(
                    f"[{RED}FAIL{RESET}] {test_name} - "
                    f"Mensagem esperada '{expected_msg}' não encontrada no stderr:\n"
                    f"{proc.stderr}"
                )
                return False

        print(f"[{GREEN}PASS{RESET}] {test_name}")
        return True
    finally:
        if os.path.exists(tmp_name):
            os.remove(tmp_name)


def run_symbols_output_test():
    test_code = """
    int soma(int a, int b) { return a + b; }
    int main() {
        int res = soma(1, 2);
        float val = 10.5;
        return 0;
    }
    """

    with tempfile.NamedTemporaryFile(mode="w", suffix=".c", delete=False) as tmp:
        tmp.write(test_code)
        tmp_name = tmp.name

    try:
        proc = subprocess.run(
            [COMPILER, "--symbols", tmp_name],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            timeout=5
        )

        if proc.returncode != 0:
            print(f"[{RED}FAIL{RESET}] Flag --symbols retornou erro: {proc.stderr}")
            return False

        output = proc.stdout
        expected = ["Escopo Nível 0", "soma", "main", "res", "val"]

        for exp in expected:
            if exp not in output:
                print(
                    f"[{RED}FAIL{RESET}] Flag --symbols não exibiu "
                    f"'{exp}':\n{output}"
                )
                return False

        print(
            f"[{GREEN}PASS{RESET}] "
            "Exibição Hierárquica da Tabela de Símbolos (--symbols)"
        )
        return True
    finally:
        if os.path.exists(tmp_name):
            os.remove(tmp_name)


# Testes da subissue #73: expressões unárias e binárias na AST.
def run_lvalue_ast_test():
    """Confere nós binários/unários que o parser não aceita como alvo."""
    with tempfile.TemporaryDirectory() as tmp_dir:
        binary = os.path.join(tmp_dir, "test_lvalue_ast")

        build = subprocess.run(
            [
                "gcc", "-Wall", "-Wextra", "-Isrc",
                "-o", binary,
                "tests/test_lvalue_ast.c",
                "src/ast.c",
                "src/symtab.c",
                "src/semantic.c"
            ],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            timeout=20
        )

        if build.returncode != 0:
            print(
                f"[{RED}FAIL{RESET}] "
                f"L-values binário/unário - compilação: {build.stderr}"
            )
            return False

        proc = subprocess.run(
            [binary],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            timeout=5
        )

        if (
            proc.returncode != 0
            or proc.stderr.count("não é um l-value válido") != 2
        ):
            print(
                f"[{RED}FAIL{RESET}] "
                f"L-values binário/unário: {proc.stderr}"
            )
            return False

        print(
            f"[{GREEN}PASS{RESET}] "
            "Rejeição de nós AST_BIN_OP e AST_UN_OP"
        )
        return True


def main():
    print("=" * 60)
    print(" Executando Testes de Semântica e Tabela de Símbolos (UnB)")
    print("=" * 60)

    tests = [
        # Casos válidos
        (
            """
            int dobro(int n) { return n * 2; }
            int main() {
                int a = 10;
                float b = 20.5;
                float c = a + b; // coerção int -> float permitida
                int res = dobro(a);
                int v[5];
                v[0] = 1;
                while (a > 0) {
                    a -= 1;
                    if (a == 5) continue;
                    if (a == 2) break;
                }
                return 0;
            }
            """,
            True,
            None,
            "Programa Válido com Funções, Tipos, Coerções e Laços"
        ),
        (
            """
            int g = 100;
            int f1() { int x = 10; return x + g; }
            int f2() { int x = 20; return x + g; }
            int main() { return f1() + f2(); }
            """,
            True,
            None,
            "Escopos Independentes em Funções Distintas com Variável Global"
        ),

        # Casos de erro semântico existentes
        (
            "int main() { x = 10; }",
            False,
            "variável 'x' não declarada",
            "Erro: Uso de Variável Não Declarada"
        ),
        (
            "int main() { int a; int a; }",
            False,
            "redeclaração da variável 'a' no mesmo escopo",
            "Erro: Redeclaração no Mesmo Escopo"
        ),
        (
            """
            int main() {
                int x = 10;
                if (x > 0) {
                    int x = 20; // Proibido no escopo simplificado
                }
            }
            """,
            False,
            "sombreamento não permitido",
            "Erro: Sombreamento de Variável em Bloco Proibido"
        ),
        (
            """
            void teste(int p) {
                if (p > 0) {
                    int p = 5; // Proibido sombrear parâmetro
                }
            }
            """,
            False,
            "sombreamento não permitido",
            "Erro: Sombreamento de Parâmetro da Função Proibido"
        ),
        (
            "void f(int a, int b) {} int main() { f(10); }",
            False,
            "função 'f' espera 2 argumentos, mas recebeu 1",
            "Erro: Aridade Incorreta em Chamada de Função"
        ),
        (
            "int main() { break; }",
            False,
            "comando 'break' fora de laço de repetição",
            "Erro: 'break' Fora de Laço"
        ),
        (
            "int main() { continue; }",
            False,
            "comando 'continue' fora de laço de repetição",
            "Erro: 'continue' Fora de Laço"
        ),
        (
            "void func_vazia() { return 10; }",
            False,
            "função com retorno 'void' não deve retornar valor",
            "Erro: Retorno com Expressão em Função Void"
        ),
        (
            'int main() { int v[10]; v["str"] = 1; }',
            False,
            "índice de array deve ser do tipo inteiro",
            "Erro: Índice de Array com Tipo Não-Inteiro"
        ),
        (
            "int main() { int x = 10; x[0] = 1; }",
            False,
            "indexação '[]' aplicada em variável que não é array",
            "Erro: Indexação em Variável Escalar Não-Array"
        ),

        # Novos testes: subissue #73
        # Rejeição de l-values inválidos
        (
            "int main() { 10 = 1; return 0; }",
            False,
            "não é um l-value válido",
            "Erro: Atribuição a Literal"
        ),
        (
            "int f() { return 0; } int main() { f() = 1; return 0; }",
            False,
            "não é um l-value válido",
            "Erro: Atribuição a Chamada"
        ),
        (
            "int main() { true += false; return 0; }",
            False,
            "não é um l-value válido",
            "Erro: Atribuição Composta a Literal"
        ),
        (
            "int main() { 1 -= 1; return 0; }",
            False,
            "não é um l-value válido",
            "Erro: Literal com -="
        ),
        (
            "int main() { 1 *= 1; return 0; }",
            False,
            "não é um l-value válido",
            "Erro: Literal com *="
        ),
        (
            "int main() { 1 /= 1; return 0; }",
            False,
            "não é um l-value válido",
            "Erro: Literal com /="
        ),

        # Destinos válidos devem continuar funcionando
        (
            "int main() { int x = 1; x *= 2; return 0; }",
            True,
            None,
            "Válido: Atribuição Composta a Identificador"
        ),
        (
            "int main() { int v[2]; v[0] += 1; return 0; }",
            True,
            None,
            "Válido: Atribuição Composta a Elemento de Vetor"
        )
    ]

    passed = 0
    total = len(tests) + 2

    if run_symbols_output_test():
        passed += 1

    if run_lvalue_ast_test():
        passed += 1

    for code, expect_success, expected_msg, name in tests:
        if run_semantic_test(code, expect_success, expected_msg, name):
            passed += 1

    print("=" * 60)
    print(f"Resultado Semântica: {passed}/{total} testes aprovados")
    print("=" * 60)

    if passed != total:
        sys.exit(1)


if __name__ == "__main__":
    main()
