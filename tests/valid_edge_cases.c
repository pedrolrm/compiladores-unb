// Casos de Borda Válidos para o Scanner

// 1. Literais numéricos especiais
float f1 = .5;
float f2 = 10.;
float f3 = 1e5;
float f4 = 2.5e-3;
float f5 = 3.E+2;
int zero = 0;
int max_int = 2147483647;

// 2. Identificadores com prefixos de palavras-chave
int int_var = 1;
float floating = 2.0;
char character = 'c';
int while1 = 3;
int for_each = 4;
int _underscore_identifier_123 = 5;

// 3. String vazia e escapes
char *s1 = "";
char *s2 = "linha 1\nlinha 2\ttabulado\\barra\"aspas\"";

// 4. Caracteres especiais e escapes
char c1 = '\n';
char c2 = '\t';
char c3 = '\\';
char c4 = '\'';
char c5 = '\"';
char c6 = '\0';

// 5. Operadores compostos adjacentes
int a = 0;
int b = ++a;
int c = a + + b;
bool eq = (a == b);
int atrb = (a = b);
