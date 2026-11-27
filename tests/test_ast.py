#!/usr/bin/env python3
"""
Suíte de Testes Automatizados para a Árvore Sintática Abstrata (AST)
Compiladores - UnB (Escopo Simplificado)
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
            "int main() { int v[10]; v[3] = 99; }",
            ["VarDecl: v (Type: int[10])", "ArrayAccess", "Base:", "Identifier: v", "Index:", "LiteralInt: 3", "LiteralInt: 99"],
            "Declaração de Vetor 1D e Indexação (v[3])"
        ),
        (
            "void teste() { if (x > 0) { x = 1; } else { x = 2; } while (x < 10) { x += 1; if (x == 5) break; } }",
            ["If", "Cond:", "BinOp: >", "Then:", "Else:", "While", "Assign: +=", "Break"],
            "Estruturas de Controle de Fluxo (if-else, while, break)"
        ),
        (
            "void teste_for() { for (int i = 0; i < 10; i += 1) { continue; } }",
            ["For", "Init:", "VarDecl: i (Type: int)", "Cond:", "BinOp: <", "Step / Body:", "Assign: +=", "Continue"],
            "Laço For Estruturado com Inicialização e Passo"
        ),
        (
            "bool checa(bool a, bool b) { return !a && (b || false); }",
            ["FuncDef: checa (Return: bool)", "BinOp: &&", "UnOp: !", "BinOp: ||", "LiteralBool: false"],
            "Expressões Lógicas e Booleanas na AST (!, &&, ||)"
        ),
        (
            "void noop(void) { return; }",
            ["FuncDef: noop (Return: void)", "Return"],
            "Função Void com Lista de Parâmetros (void)"
        ),
        (
            "int g = 1; int main() { int a = 1, b, c = 3; return a + c; }",
            ["VarDecl: g (Type: int)", "VarDecl: a (Type: int)", "VarDecl: b (Type: int)", "VarDecl: c (Type: int)"],
            "Declaração Global e Múltiplos Declaradores na Mesma Linha"
        ),
        (
            "int main() { if (a) if (b) return 1; else return 2; return 0; }",
            ["If", "Then:", "Else:", "LiteralInt: 1", "LiteralInt: 2"],
            "Desambiguação de Dangling Else na AST"
        ),
        (
            "int main() { for (;;) { break; } return 0; }",
            ["For", "Step / Body:", "Break"],
            "Laço For Infinito for(;;) na AST"
        ),
        (
            "int main() { int x = -5; x -= 2; x *= 3; x /= 2; return x % 4; }",
            ["UnOp: -", "Assign: -=", "Assign: *=", "Assign: /=", "BinOp: %"],
            "Operador Unário (-), Atribuições Compostas e Módulo"
        ),
        (
            'int main() { char s[] = "oi"; return 0; }',
            ["VarDecl: s (Type: char[])", "LiteralString: \"oi\""],
            "Vetor char[] com Tamanho Inferido por Literal String"
        ),
        (
            "int f(int x, float y) { return x; } int main() { return f(10, 2.5); }",
            ["Call: f", "Args:", "LiteralInt: 10", "LiteralFloat: 2.500000"],
            "Chamada de Função com Múltiplos Argumentos"
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
