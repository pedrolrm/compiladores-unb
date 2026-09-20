// Teste de Cobertura Completa de Tokens Válidos
int main() {
    // Tipos primitivos e modificadores
    int a = 10;
    float b = 3.14;
    char c = 'Z';
    void *ptr = 0;
    double d = 2.71828;
    bool flag = true;
    long l = 100000;
    short s = 2;
    unsigned int u = 42;
    const int constante = 99;

    // Estruturas
    struct Ponto {
        int x;
        int y;
    };
    typedef struct Ponto Ponto;

    // Estruturas de controle
    if (a == 10 && b != 0.0) {
        a += 1;
    } else if (a <= 5 || a >= 20) {
        a -= 1;
    } else {
        a *= 2;
    }

    // Laços de repetição
    while (a > 0) {
        a--;
        if (a == 5) continue;
        if (a == 2) break;
    }

    for (int i = 0; i < 10; i++) {
        b /= 2.0;
    }

    do {
        a++;
    } while (false);

    // Switch case
    switch (a) {
        case 1:
            b = 1.0;
            break;
        default:
            b = 0.0;
            break;
    }

    // Operadores lógicos, ternário e delimitadores
    int res = (!flag) ? (a % 2) : (b > 1.0 ? 1 : 0);

    return 0;
}
