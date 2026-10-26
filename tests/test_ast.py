#!/usr/bin/env python3
"""
Suíte de Testes Automatizados para a Árvore Sintática Abstrata (AST)
Compiladores - UnB
"""

import os
import subprocess
import sys
import tempfile

GREEN = "\033[92m"
RED = "\033[91m"
BOLD = "\033[1m"
RESET = "\033[0m"

COMPILER = "./compilador"

def run_ast_test(source_code, expected_substrings, test_name):
    with tempfile.NamedTemporaryFile(mode="w", suffix=".c", delete=False) as tmp:
        tmp.write(source_code)
        tmp_name = tmp.name

    try:
        proc = subprocess.run([COMPILER, "--ast", tmp_name],
                              stdout=subprocess.PIPE,
                              stderr=subprocess.PIPE,
                              text=True,
                              timeout=5)
        if proc.returncode != 0:
            print(f"[{RED}FAIL{RESET}] {test_name} - Erro de compilação (código {proc.returncode}):\n{proc.stderr}")
            return False

        output = proc.stdout
        for substr in expected_substrings:
            if substr not in output:
                print(f"[{RED}FAIL{RESET}] {test_name} - Substring esperada '{substr}' não encontrada na AST:\n{output}")
                return False

        print(f"[{GREEN}PASS{RESET}] {test_name}")
        return True
    finally:
        if os.path.exists(tmp_name):
            os.remove(tmp_name)

def main():
    print("=" * 60)
    print(" Executando Testes Automatizados da AST (Compiladores UnB)")
    print("=" * 60)

    tests = [
        (
            "int main() { int x = a + b * c; }",
            ["FuncDef: main (Return: int)", "VarDecl: x (Type: int)", "BinOp: +", "Identifier: a", "BinOp: *", "Identifier: b", "Identifier: c"],
            "Precedência Aritmética na AST (a + b * c)"
        ),
        (
            "int soma(int a, int b) { return a + b; }",
            ["FuncDef: soma (Return: int)", "Params:", "Param: a (Type: int)", "Param: b (Type: int)", "Return", "BinOp: +"],
            "Definição de Função com Parâmetros e Return"
        ),
        (
            "struct Ponto { int x; int y; }; void f() { struct Ponto p; p.x = 42; }",
            ["StructDecl: Ponto", "Members:", "VarDecl: x (Type: int)", "VarDecl: y (Type: int)", "VarDecl: p (Type: struct Ponto)", "MemberAccess (.x)", "LiteralInt: 42"],
            "Declaração de Struct e Acesso a Campo (.x)"
        ),
        (
            "int main() { int v[10]; v[3] = 99; }",
            ["VarDecl: v (Type: int[10])", "ArrayAccess", "Base:", "Identifier: v", "Index:", "LiteralInt: 3", "LiteralInt: 99"],
            "Declaração de Array e Indexação (v[3])"
        ),
        (
            "void teste() { if (x > 0) { x = 1; } else { x = 2; } while (x < 10) { x += 1; if (x == 5) break; } }",
            ["If", "Cond:", "BinOp: >", "Then:", "Else:", "While", "Assign: +=", "Break"],
            "Estruturas de Controle de Fluxo (if-else, while, break)"
        ),
        (
            "void teste_switch(int op) { switch (op) { case 1: return 10; default: return 0; } }",
            ["Switch", "Case", "LiteralInt: 1", "Default"],
            "Comando Switch e Rótulos Case/Default"
        )
    ]

    passed = 0
    total = len(tests)

    for code, expected, name in tests:
        if run_ast_test(code, expected, name):
            passed += 1

    print("=" * 60)
    print(f"Resultado AST: {passed}/{total} testes aprovados")
    print("=" * 60)

    if passed != total:
        sys.exit(1)

if __name__ == "__main__":
    main()
