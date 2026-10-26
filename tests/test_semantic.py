#!/usr/bin/env python3
"""
Suíte de Testes Automatizados para a Tabela de Símbolos e Análise Semântica
Compiladores - UnB
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
        proc = subprocess.run([COMPILER, "--check", tmp_name],
                              stdout=subprocess.PIPE,
                              stderr=subprocess.PIPE,
                              text=True,
                              timeout=5)

        if expect_success:
            if proc.returncode != 0:
                print(f"[{RED}FAIL{RESET}] {test_name} - Esperava sucesso (0), obteve {proc.returncode}:\n{proc.stderr}")
                return False
        else:
            if proc.returncode == 0:
                print(f"[{RED}FAIL{RESET}] {test_name} - Esperava falha semântica, mas execução retornou 0.")
                return False
            if expected_msg and expected_msg not in proc.stderr:
                print(f"[{RED}FAIL{RESET}] {test_name} - Mensagem esperada '{expected_msg}' não encontrada no stderr:\n{proc.stderr}")
                return False

        print(f"[{GREEN}PASS{RESET}] {test_name}")
        return True
    finally:
        if os.path.exists(tmp_name):
            os.remove(tmp_name)

def run_symbols_output_test():
    test_code = """
    struct Ponto { int x; int y; };
    int soma(int a, int b) { return a + b; }
    int main() {
        struct Ponto pt;
        int res = soma(1, 2);
        return 0;
    }
    """
    with tempfile.NamedTemporaryFile(mode="w", suffix=".c", delete=False) as tmp:
        tmp.write(test_code)
        tmp_name = tmp.name

    try:
        proc = subprocess.run([COMPILER, "--symbols", tmp_name],
                              stdout=subprocess.PIPE,
                              stderr=subprocess.PIPE,
                              text=True,
                              timeout=5)
        if proc.returncode != 0:
            print(f"[{RED}FAIL{RESET}] Flag --symbols retornou erro: {proc.stderr}")
            return False

        output = proc.stdout
        expected = ["Escopo Nível 0", "Ponto", "soma", "main", "pt", "res"]
        for exp in expected:
            if exp not in output:
                print(f"[{RED}FAIL{RESET}] Flag --symbols não exibiu '{exp}':\n{output}")
                return False

        print(f"[{GREEN}PASS{RESET}] Exibição Hierárquica da Tabela de Símbolos (--symbols)")
        return True
    finally:
        if os.path.exists(tmp_name):
            os.remove(tmp_name)

def main():
    print("=" * 60)
    print(" Executando Testes de Semântica e Tabela de Símbolos (UnB)")
    print("=" * 60)

    tests = [
        # Casos válidos
        (
            """
            struct Ponto { int x; int y; };
            int dobro(int n) { return n * 2; }
            int main() {
                int a = 10;
                float b = 20.5;
                float c = a + b; // coerção int -> float permitida
                struct Ponto p;
                p.x = 5;
                p.y = dobro(p.x);
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
            "Programa Válido com Funções, Structs, Escopos e Laços"
        ),
        (
            """
            int x = 100;
            int main() {
                int x = 10; // shadowing válido
                {
                    int x = 1; // shadowing válido em bloco interno
                }
                return x;
            }
            """,
            True,
            None,
            "Sombreamento de Variáveis (Shadowing) em Escopos Aninhados"
        ),

        # Casos de erro semântico
        (
            "int main() { x = 10; }",
            False,
            "variável 'x' não declarada",
            "Erro: Uso de Variável Não Declarada"
        ),
        (
            "int main() { int a; int a; }",
            False,
            "redeclaração da variável 'a'",
            "Erro: Redeclaração no Mesmo Escopo"
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
            "comando 'break' fora de laço",
            "Erro: 'break' Fora de Laço ou Switch"
        ),
        (
            "int main() { continue; }",
            False,
            "comando 'continue' fora de laço",
            "Erro: 'continue' Fora de Laço"
        ),
        (
            "struct Ponto { int x; }; int main() { struct Ponto p; p.z = 10; }",
            False,
            "membro 'z' não existe na struct 'Ponto'",
            "Erro: Acesso a Membro Inexistente na Struct"
        ),
        (
            "int main() { int x = 10; x.campo = 5; }",
            False,
            "acesso de membro '.' em identificador que não é struct",
            "Erro: Acesso de Membro '.' em Tipo Não-Struct"
        ),
        (
            "int main() { int v[10]; v[\"str\"] = 1; }",
            False,
            "índice de array deve ser do tipo inteiro",
            "Erro: Índice de Array com Tipo Não-Inteiro"
        ),
        (
            "int main() { int x = 10; x[0] = 1; }",
            False,
            "indexação '[]' aplicada em variável que não é array",
            "Erro: Indexação em Variável Escalar Não-Array"
        )
    ]

    passed = 0
    total = len(tests) + 1

    if run_symbols_output_test():
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
