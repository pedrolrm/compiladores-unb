// Teste de Erros Léxicos e Caracteres Inválidos (Issue #20)

// 1. Caracteres desconhecidos/inválidos
@
$
#
`
~

// 2. Caracteres válidos intercalados com inválidos
int a = 10;
@ int b = 20; $
float c = 3.14 #;

// 3. String literal não fechada
"string sem fechar
int continua_executando = 1;

// 4. Literal de caractere não fechado
'a
int ainda_executando = 2;
