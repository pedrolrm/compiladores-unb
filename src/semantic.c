
#include "semantic.h"
#include "parser.tab.h"
#include <string.h>

int semantic_errors_count = 0;

static Scope *current_scope = NULL;
static Scope *global_scope = NULL;

static bool in_function = false;
static DataType current_func_return_type;
static int loop_depth = 0;

void report_semantic_error(int line, int col, const char *msg) {
    semantic_errors_count++;
    fprintf(stderr, "Erro semântico na linha %d, coluna %d: %s\n", line, col, msg);
}

/*
 * Checagem de compatibilidade de tipos (suporta promoção de int para float)
 */
static bool is_type_compatible(const DataType *dest, const DataType *src) {
    if (!dest || !src) return false;
    if (data_type_equals(dest, src)) return true;

    /* Promoção implícita: int pode ser atribuído a float */
    if (dest->base == TYPE_FLOAT && src->base == TYPE_INT && !dest->is_array && !src->is_array) {
        return true;
    }

    return false;
}

/*
 * Declaração adiantada para visita recursiva da AST
 */

static bool is_literal_zero(const ASTNode *n) {
    if (n->type == AST_LIT_INT) {
        return n->int_val == 0;
    } else if (n->type == AST_LIT_FLOAT) {
        return n->float_val == 0.0f;
    }
    return false;
}

/*
 * Análise de Fluxo de Controle (Control Flow Analysis):
 * Verifica recursivamente se todos os caminhos de execução possíveis terminam
 * em um nó AST_RETURN com expressão válida:
 * - Em comandos sequenciais / blocos: percorre as instruções e um return
 *   no fluxo principal satisfaz a condição.
 * - Em seleções (AST_IF): ambos os ramos (then e else) devem conter retornos garantidos;
 *   if sem else não garante retorno.
 */
bool has_guaranteed_return(ASTNode *body) {
    if (!body) return false;

    switch (body->type) {
        case AST_RETURN:
            return body->left != NULL;

        case AST_BLOCK: {
            ASTNode *stmt = body->left;
            while (stmt) {
                if (has_guaranteed_return(stmt)) {
                    return true;
                }
                stmt = stmt->next;
            }
            return false;
        }

        case AST_IF: {
            if (!body->extra) {
                return false; /* if sem else não garante retorno */
            }
            return has_guaranteed_return(body->right) && has_guaranteed_return(body->extra);
        }

        default:
            return false;
    }
}

static void check_node(ASTNode *node);

/*
 * Passo 1: Pré-coleta de declarações globais de funções
 */
static void collect_global_declarations(ASTNode *node) {
    while (node) {
        if (node->type == AST_FUNC_DEF) {
            if (symtab_lookup_local(global_scope, node->name) != NULL) {
                char msg[128];
                snprintf(msg, sizeof(msg), "redeclaração da função '%s'", node->name);
                report_semantic_error(node->line, node->column, msg);
            } else {
                int param_count = 0;
                ASTNode *p = node->left;
                while (p) {
                    param_count++;
                    p = p->next;
                }

                DataType *param_types = NULL;
                if (param_count > 0) {
                    param_types = (DataType *)calloc(param_count, sizeof(DataType));
                    int i = 0;
                    p = node->left;
                    while (p) {
                        param_types[i++] = p->data_type;
                        p = p->next;
                    }
                }

                symtab_insert_func(global_scope, node->name, node->data_type, param_count, param_types, node->line, node->column);
                if (param_types) free(param_types);
            }
        }
        node = node->next;
    }
}

/*
 * Passo 2: Verificação semântica, resolução de escopos e tipos
 */
static void check_node(ASTNode *node) {
    if (!node) return;

    switch (node->type) {
        case AST_PROGRAM:
            check_node(node->left);
            break;

        case AST_VAR_DECL: {
            /* Se houver expressão de inicialização, checa primeiro a expressão */
            if (node->left) {
                check_node(node->left);
                if (node->left->data_type.base != TYPE_UNKNOWN) {
                    if (!is_type_compatible(&node->data_type, &node->left->data_type)) {
                        char msg[128];
                        snprintf(msg, sizeof(msg), "tipo incompatível na inicialização da variável '%s'", node->name);
                        report_semantic_error(node->line, node->column, msg);
                    }
                }
            }

            /* Tamanho de vetor declarado precisa ser estritamente positivo */
            if (node->data_type.is_array && node->data_type.array_size != -1 &&
                node->data_type.array_size <= 0) {
                char msg[128];
                snprintf(msg, sizeof(msg), "tamanho do vetor '%s' deve ser maior que zero", node->name);
                report_semantic_error(node->line, node->column, msg);
            }

            /* Bloqueia redeclaração no mesmo escopo e sombreamento (shadowing) dentro da função */
            Symbol *existing = symtab_lookup(current_scope, node->name);
            if (existing && (current_scope->level == 0 || existing->scope_level > 0)) {
                char msg[128];
                if (existing->scope_level == current_scope->level) {
                    snprintf(msg, sizeof(msg), "redeclaração da variável '%s' no mesmo escopo", node->name);
                } else {
                    snprintf(msg, sizeof(msg), "redeclaração da variável '%s' no escopo da função (sombreamento não permitido)", node->name);
                }
                report_semantic_error(node->line, node->column, msg);
            } else {
                symtab_insert_var(current_scope, node->name, node->data_type, node->line, node->column);
            }
            break;
        }

        case AST_FUNC_DEF: {
            DataType prev_ret = current_func_return_type;
            bool prev_in_func = in_function;

            in_function = true;
            current_func_return_type = node->data_type;

            /* Abre novo escopo para a função */
            current_scope = symtab_enter_scope(current_scope);

            /* Insere parâmetros no escopo da função */
            ASTNode *param = node->left;
            while (param) {
                if (param->type == AST_PARAM_DECL) {
                    if (symtab_insert_param(current_scope, param->name, param->data_type, param->line, param->column) == NULL) {
                        char msg[128];
                        snprintf(msg, sizeof(msg), "parâmetro duplicado '%s' na função '%s'", param->name, node->name);
                        report_semantic_error(param->line, param->column, msg);
                    }
                }
                param = param->next;
            }

            /* Analisa corpo da função */
            check_node(node->right);

            /* Análise de fluxo de controle: retorno obrigatório em funções não-void */
            if (node->data_type.base != TYPE_VOID && node->data_type.base != TYPE_UNKNOWN) {
                if (!has_guaranteed_return(node->right)) {
                    char msg[256];
                    snprintf(msg, sizeof(msg), "função '%s' com retorno não-void sem comando 'return' garantido em todos os caminhos", node->name);
                    report_semantic_error(node->line, node->column, msg);
                }
            }

            /* Fecha escopo da função */
            current_scope = symtab_exit_scope(current_scope);

            in_function = prev_in_func;
            current_func_return_type = prev_ret;
            break;
        }

        case AST_BLOCK: {
            current_scope = symtab_enter_scope(current_scope);
            check_node(node->left);
            current_scope = symtab_exit_scope(current_scope);
            break;
        }

        case AST_EXPR_STMT:
            check_node(node->left);
            break;

        case AST_IF: {
            check_node(node->left);

            DataType *t = &node->left->data_type;
            bool tipo_valido = !t->is_array && (t->base == TYPE_INT || t->base == TYPE_FLOAT || t->base == TYPE_BOOL);

            if (t->base != TYPE_UNKNOWN && !tipo_valido) {
                report_semantic_error(node->left->line, node->left->column, "condição do comando 'if' deve ser do tipo int, float ou bool");
            }

            check_node(node->right);
            if (node->extra) {
                check_node(node->extra);
            }
            break;
        }

        case AST_WHILE: {
            loop_depth++;
            check_node(node->left);
            DataType *t = &node->left->data_type;
            bool tipo_valido = !t->is_array && (t->base == TYPE_INT || t->base == TYPE_FLOAT || t->base == TYPE_BOOL);

            if (t->base != TYPE_UNKNOWN && !tipo_valido) {
                report_semantic_error(node->left->line, node->left->column, "condição do comando 'while' deve ser do tipo int, float ou bool");
            }
            check_node(node->right);
            loop_depth--;
            break;
        }

        case AST_FOR: {
            current_scope = symtab_enter_scope(current_scope);
            loop_depth++;
            check_node(node->left);  /* init */

            check_node(node->right); /* cond */
            if (node->right) {
                DataType *t = &node->right->data_type;
                bool tipo_valido = !t->is_array && (t->base == TYPE_INT || t->base == TYPE_FLOAT || t->base == TYPE_BOOL);

                if (t->base != TYPE_UNKNOWN && !tipo_valido) {
                    report_semantic_error(node->right->line, node->right->column, "condição do comando 'for' deve ser do tipo int, float ou bool");
                }
            }
            check_node(node->extra); /* step e body */
            loop_depth--;
            current_scope = symtab_exit_scope(current_scope);
            break;
        }

        case AST_BREAK:
            if (loop_depth == 0) {
                report_semantic_error(node->line, node->column, "comando 'break' fora de laço de repetição");
            }
            break;

        case AST_CONTINUE:
            if (loop_depth == 0) {
                report_semantic_error(node->line, node->column, "comando 'continue' fora de laço de repetição");
            }
            break;

        case AST_RETURN:
            if (!in_function) {
                report_semantic_error(node->line, node->column, "comando 'return' fora do corpo de uma função");
            } else {
                if (node->left) {
                    check_node(node->left);
                    if (current_func_return_type.base == TYPE_VOID) {
                        report_semantic_error(node->line, node->column, "função com retorno 'void' não deve retornar valor");
                    } else if (node->left->data_type.base != TYPE_UNKNOWN &&
                               !is_type_compatible(&current_func_return_type, &node->left->data_type)) {
                        report_semantic_error(node->line, node->column, "tipo de retorno incompatível com a declaração da função");
                    }
                } else {
                    if (current_func_return_type.base != TYPE_VOID) {
                        report_semantic_error(node->line, node->column, "função com retorno não-void requer expressão no 'return'");
                    }
                }
            }
            break;

        case AST_IDENTIFIER: {
            Symbol *sym = symtab_lookup(current_scope, node->name);
            if (!sym) {
                char msg[128];
                snprintf(msg, sizeof(msg), "variável '%s' não declarada", node->name);
                report_semantic_error(node->line, node->column, msg);
                node->data_type = data_type_primitive(TYPE_UNKNOWN);
            } else {
                node->data_type = sym->type;
            }
            break;
        }

        case AST_CALL: {
            Symbol *func = symtab_lookup(current_scope, node->name);
            if (!func) {
                char msg[128];
                snprintf(msg, sizeof(msg), "função '%s' não declarada", node->name);
                report_semantic_error(node->line, node->column, msg);
                node->data_type = data_type_primitive(TYPE_UNKNOWN);
            } else if (func->kind != SYM_FUNC) {
                char msg[128];
                snprintf(msg, sizeof(msg), "'%s' não é uma função", node->name);
                report_semantic_error(node->line, node->column, msg);
                node->data_type = data_type_primitive(TYPE_UNKNOWN);
            } else {
                node->data_type = func->return_type;

                /* Checa argumentos */
                int arg_count = 0;
                ASTNode *arg = node->left;
                while (arg) {
                    check_node(arg);
                    if (arg_count < func->param_count && arg->data_type.base != TYPE_UNKNOWN) {
                        if (!is_type_compatible(&func->param_types[arg_count], &arg->data_type)) {
                            char msg[128];
                            snprintf(msg, sizeof(msg), "argumento %d da função '%s' possui tipo incompatível", arg_count + 1, node->name);
                            report_semantic_error(arg->line, arg->column, msg);
                        }
                    }
                    arg_count++;
                    arg = arg->next;
                }

                if (arg_count != func->param_count) {
                    char msg[128];
                    snprintf(msg, sizeof(msg), "função '%s' espera %d argumentos, mas recebeu %d", node->name, func->param_count, arg_count);
                    report_semantic_error(node->line, node->column, msg);
                }
            }
            break;
        }

        case AST_ARRAY_ACCESS: {
            check_node(node->left);
            check_node(node->right);

            if (node->left->data_type.base != TYPE_UNKNOWN && !node->left->data_type.is_array) {
                report_semantic_error(node->line, node->column, "indexação '[]' aplicada em variável que não é array");
            }
            if (node->right->data_type.base != TYPE_UNKNOWN && node->right->data_type.base != TYPE_INT) {
                report_semantic_error(node->line, node->column, "índice de array deve ser do tipo inteiro");
            }

            /* Checagem estática de limites: só é possível quando o índice é um
             * literal e o tamanho do vetor é conhecido em tempo de compilação */
            int idx = 0;
            if (node->left->data_type.is_array && node->left->data_type.array_size > 0 &&
                ast_literal_int_value(node->right, &idx)) {
                if (idx < 0 || idx >= node->left->data_type.array_size) {
                    char msg[128];
                    snprintf(msg, sizeof(msg),
                             "índice de array fora dos limites (índice %d, tamanho %d)",
                             idx, node->left->data_type.array_size);
                    report_semantic_error(node->line, node->column, msg);
                }
            }

            node->data_type = node->left->data_type;
            node->data_type.is_array = false;
            node->data_type.array_size = 0;
            break;
        }

        case AST_BIN_OP: {
            check_node(node->left);
            check_node(node->right);

            DataType *lt = &node->left->data_type;
            DataType *rt = &node->right->data_type;

            if ((node->op == OP_DIV || node->op == OP_MOD) && is_literal_zero(node->right)) {
                report_semantic_error(node->right->line, node->right->column, "divisão por zero");
            }

            /* Operadores relacionais e lógicos sempre resultam em bool */
            if (node->op == OP_EQ || node->op == OP_NEQ ||
                node->op == OP_LT || node->op == OP_LE ||
                node->op == OP_GT || node->op == OP_GE ||
                node->op == OP_AND || node->op == OP_OR) {
                node->data_type = data_type_primitive(TYPE_BOOL);
            } else if (node->op == OP_MOD) {
                bool esq_int = lt->base == TYPE_INT && !lt->is_array;
                bool dir_int = rt->base == TYPE_INT && !rt->is_array;

                bool esq_invalido = lt->base != TYPE_UNKNOWN && !esq_int;
                bool dir_invalido = rt->base != TYPE_UNKNOWN && !dir_int;

                if (esq_invalido || dir_invalido) {
                    report_semantic_error(node->line, node->column, "operador '%' exige operandos do tipo int");
                } else if (esq_int && dir_int) {
                    node->data_type = data_type_primitive(TYPE_INT);
                } else {
                    node->data_type = data_type_primitive(TYPE_UNKNOWN);
                }
            } else {
                /* Operadores aritméticos (+, -, *, /) */
                if (lt->base == TYPE_FLOAT || rt->base == TYPE_FLOAT) {
                    node->data_type = data_type_primitive(TYPE_FLOAT);
                } else if (lt->base == TYPE_INT && rt->base == TYPE_INT) {
                    node->data_type = data_type_primitive(TYPE_INT);
                } else {
                    node->data_type = *lt;
                }
            }
            break;
        }

        case AST_UN_OP:
            check_node(node->left);
            if (node->op == OP_NOT) {
                node->data_type = data_type_primitive(TYPE_BOOL);
            } else {
                node->data_type = node->left->data_type;
            }
            break;

        case AST_ASSIGN: {
            /* Alteração #73: validação do destino (l-value). */
            if (node->left->type != AST_IDENTIFIER &&
                node->left->type != AST_ARRAY_ACCESS) {
                report_semantic_error(
                    node->line,
                    node->column,
                    "lado esquerdo da atribuição não é um l-value válido"
                );
            }

            check_node(node->left);
            check_node(node->right);

            if (node->op == OP_DIV_ASSIGN && is_literal_zero(node->right)) {
                report_semantic_error(node->right->line, node->right->column, "divisão por zero");
            }

            if (node->left->data_type.base != TYPE_UNKNOWN && node->right->data_type.base != TYPE_UNKNOWN) {
                if (!is_type_compatible(&node->left->data_type, &node->right->data_type)) {
                    report_semantic_error(node->line, node->column, "tipos incompatíveis na atribuição");
                }
            }
            node->data_type = node->left->data_type;
            break;
        }

        case AST_LIT_INT:
        case AST_LIT_FLOAT:
        case AST_LIT_CHAR:
        case AST_LIT_STRING:
        case AST_LIT_BOOL:
            /* Tipos já fixados nos construtores */
            break;

        default:
            break;
    }

    /* Processa os próximos nós irmãos na lista */
    check_node(node->next);
}

int semantic_analyze(ASTNode *root, Scope **out_global_scope) {
    semantic_errors_count = 0;
    loop_depth = 0;
    in_function = false;

    global_scope = symtab_create_global();
    current_scope = global_scope;

    /* Passo 1: Pré-coleta global */
    if (root && root->type == AST_PROGRAM) {
        collect_global_declarations(root->left);
    } else {
        collect_global_declarations(root);
    }

    /* Passo 2: Verificação completa de escopos e tipos */
    check_node(root);

    /* Passo 3: Validação do ponto de entrada (função main) */
    Symbol *main_sym = symtab_lookup_local(global_scope, "main");
    if (!main_sym) {
        int line = root ? (root->line > 0 ? root->line : 1) : 1;
        int col = root ? (root->column > 0 ? root->column : 1) : 1;
        report_semantic_error(line, col, "função 'main' não declarada no escopo global");
    } else if (main_sym->kind != SYM_FUNC) {
        report_semantic_error(main_sym->line, main_sym->column, "o identificador 'main' deve ser uma função");
    } else if (main_sym->return_type.is_array ||
               (main_sym->return_type.base != TYPE_INT && main_sym->return_type.base != TYPE_VOID)) {
        report_semantic_error(main_sym->line, main_sym->column, "tipo de retorno da função 'main' deve ser 'int' ou 'void'");
    }

    if (out_global_scope) {
        *out_global_scope = global_scope;
    } else {
        symtab_free(global_scope);
        global_scope = NULL;
    }

    return semantic_errors_count;
}
