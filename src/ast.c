#include "ast.h"
#include <string.h>

ASTNode *ast_root = NULL;

/*
 * Construtores e Utilitários de DataType
 */
DataType data_type_primitive(BaseType base) {
    DataType dt;
    dt.base = base;
    dt.struct_name = NULL;
    dt.is_array = false;
    dt.array_size = 0;
    return dt;
}

DataType data_type_struct(const char *struct_name) {
    DataType dt;
    dt.base = TYPE_STRUCT;
    dt.struct_name = struct_name ? strdup(struct_name) : NULL;
    dt.is_array = false;
    dt.array_size = 0;
    return dt;
}

DataType data_type_array(DataType elem_type, int size) {
    elem_type.is_array = true;
    elem_type.array_size = size;
    return elem_type;
}

const char *base_type_to_string(BaseType base) {
    switch (base) {
        case TYPE_VOID:   return "void";
        case TYPE_INT:    return "int";
        case TYPE_FLOAT:  return "float";
        case TYPE_CHAR:   return "char";
        case TYPE_BOOL:   return "bool";
        case TYPE_STRUCT: return "struct";
        default:          return "unknown";
    }
}

const char *data_type_to_string(const DataType *dt) {
    static char buffer[128];
    if (!dt) return "null";

    if (dt->base == TYPE_STRUCT) {
        if (dt->is_array) {
            if (dt->array_size > 0)
                snprintf(buffer, sizeof(buffer), "struct %s[%d]", dt->struct_name ? dt->struct_name : "?", dt->array_size);
            else
                snprintf(buffer, sizeof(buffer), "struct %s[]", dt->struct_name ? dt->struct_name : "?");
        } else {
            snprintf(buffer, sizeof(buffer), "struct %s", dt->struct_name ? dt->struct_name : "?");
        }
    } else {
        const char *base_str = base_type_to_string(dt->base);
        if (dt->is_array) {
            if (dt->array_size > 0)
                snprintf(buffer, sizeof(buffer), "%s[%d]", base_str, dt->array_size);
            else
                snprintf(buffer, sizeof(buffer), "%s[]", base_str);
        } else {
            snprintf(buffer, sizeof(buffer), "%s", base_str);
        }
    }
    return buffer;
}

bool data_type_equals(const DataType *a, const DataType *b) {
    if (!a || !b) return false;
    if (a->base != b->base) return false;
    if (a->is_array != b->is_array) return false;
    if (a->base == TYPE_STRUCT) {
        if (!a->struct_name || !b->struct_name) return false;
        if (strcmp(a->struct_name, b->struct_name) != 0) return false;
    }
    return true;
}

/*
 * Alocação base de nó da AST
 */
ASTNode *ast_create_node(ASTNodeType type, int line, int col) {
    ASTNode *node = (ASTNode *)calloc(1, sizeof(ASTNode));
    if (!node) {
        perror("calloc");
        exit(EXIT_FAILURE);
    }
    node->type = type;
    node->line = line;
    node->column = col;
    node->data_type = data_type_primitive(TYPE_UNKNOWN);
    return node;
}

/*
 * Construtores específicos de nós
 */
ASTNode *ast_create_program(ASTNode *declarations, int line, int col) {
    ASTNode *node = ast_create_node(AST_PROGRAM, line, col);
    node->left = declarations;
    return node;
}

ASTNode *ast_create_var_decl(DataType dt, const char *name, ASTNode *init_expr, int line, int col) {
    ASTNode *node = ast_create_node(AST_VAR_DECL, line, col);
    node->data_type = dt;
    node->name = name ? strdup(name) : NULL;
    node->left = init_expr;
    return node;
}

ASTNode *ast_create_func_def(DataType ret_type, const char *name, ASTNode *params, ASTNode *body, int line, int col) {
    ASTNode *node = ast_create_node(AST_FUNC_DEF, line, col);
    node->data_type = ret_type;
    node->name = name ? strdup(name) : NULL;
    node->left = params;
    node->right = body;
    return node;
}

ASTNode *ast_create_param(DataType dt, const char *name, int line, int col) {
    ASTNode *node = ast_create_node(AST_PARAM_DECL, line, col);
    node->data_type = dt;
    node->name = name ? strdup(name) : NULL;
    return node;
}

ASTNode *ast_create_struct_decl(const char *name, ASTNode *members, int line, int col) {
    ASTNode *node = ast_create_node(AST_STRUCT_DECL, line, col);
    node->data_type = data_type_struct(name);
    node->name = name ? strdup(name) : NULL;
    node->left = members;
    return node;
}

ASTNode *ast_create_block(ASTNode *statements, int line, int col) {
    ASTNode *node = ast_create_node(AST_BLOCK, line, col);
    node->left = statements;
    return node;
}

ASTNode *ast_create_expr_stmt(ASTNode *expr, int line, int col) {
    ASTNode *node = ast_create_node(AST_EXPR_STMT, line, col);
    node->left = expr;
    return node;
}

ASTNode *ast_create_if(ASTNode *cond, ASTNode *then_branch, ASTNode *else_branch, int line, int col) {
    ASTNode *node = ast_create_node(AST_IF, line, col);
    node->left = cond;
    node->right = then_branch;
    node->extra = else_branch;
    return node;
}

ASTNode *ast_create_while(ASTNode *cond, ASTNode *body, int line, int col) {
    ASTNode *node = ast_create_node(AST_WHILE, line, col);
    node->left = cond;
    node->right = body;
    return node;
}

ASTNode *ast_create_for(ASTNode *init, ASTNode *cond, ASTNode *step, ASTNode *body, int line, int col) {
    ASTNode *node = ast_create_node(AST_FOR, line, col);
    node->left = init;
    node->right = cond;
    node->extra = step;
    /* Encadeia o corpo como proximo no extra ou usa um nó intermediário.
       Para simplicidade: next de extra armazena body */
    if (step) {
        step->next = body;
    } else {
        node->extra = body;
    }
    return node;
}

ASTNode *ast_create_switch(ASTNode *expr, ASTNode *body, int line, int col) {
    ASTNode *node = ast_create_node(AST_SWITCH, line, col);
    node->left = expr;
    node->right = body;
    return node;
}

ASTNode *ast_create_case(ASTNode *expr, ASTNode *stmt, int line, int col) {
    ASTNode *node = ast_create_node(AST_CASE, line, col);
    node->left = expr;
    node->right = stmt;
    return node;
}

ASTNode *ast_create_default(ASTNode *stmt, int line, int col) {
    ASTNode *node = ast_create_node(AST_DEFAULT, line, col);
    node->right = stmt;
    return node;
}

ASTNode *ast_create_break(int line, int col) {
    return ast_create_node(AST_BREAK, line, col);
}

ASTNode *ast_create_continue(int line, int col) {
    return ast_create_node(AST_CONTINUE, line, col);
}

ASTNode *ast_create_return(ASTNode *expr, int line, int col) {
    ASTNode *node = ast_create_node(AST_RETURN, line, col);
    node->left = expr;
    return node;
}

ASTNode *ast_create_bin_op(int op, ASTNode *left, ASTNode *right, int line, int col) {
    ASTNode *node = ast_create_node(AST_BIN_OP, line, col);
    node->op = op;
    node->left = left;
    node->right = right;
    return node;
}

ASTNode *ast_create_un_op(int op, ASTNode *operand, int line, int col) {
    ASTNode *node = ast_create_node(AST_UN_OP, line, col);
    node->op = op;
    node->left = operand;
    return node;
}

ASTNode *ast_create_assign(int op, ASTNode *target, ASTNode *expr, int line, int col) {
    ASTNode *node = ast_create_node(AST_ASSIGN, line, col);
    node->op = op;
    node->left = target;
    node->right = expr;
    return node;
}

ASTNode *ast_create_call(const char *callee_name, ASTNode *args, int line, int col) {
    ASTNode *node = ast_create_node(AST_CALL, line, col);
    node->name = callee_name ? strdup(callee_name) : NULL;
    node->left = args;
    return node;
}

ASTNode *ast_create_array_access(ASTNode *array, ASTNode *index, int line, int col) {
    ASTNode *node = ast_create_node(AST_ARRAY_ACCESS, line, col);
    node->left = array;
    node->right = index;
    return node;
}

ASTNode *ast_create_member_access(ASTNode *object, const char *member, int line, int col) {
    ASTNode *node = ast_create_node(AST_MEMBER_ACCESS, line, col);
    node->left = object;
    node->name = member ? strdup(member) : NULL;
    return node;
}

ASTNode *ast_create_identifier(const char *name, int line, int col) {
    ASTNode *node = ast_create_node(AST_IDENTIFIER, line, col);
    node->name = name ? strdup(name) : NULL;
    return node;
}

ASTNode *ast_create_lit_int(int val, int line, int col) {
    ASTNode *node = ast_create_node(AST_LIT_INT, line, col);
    node->int_val = val;
    node->data_type = data_type_primitive(TYPE_INT);
    return node;
}

ASTNode *ast_create_lit_float(float val, int line, int col) {
    ASTNode *node = ast_create_node(AST_LIT_FLOAT, line, col);
    node->float_val = val;
    node->data_type = data_type_primitive(TYPE_FLOAT);
    return node;
}

ASTNode *ast_create_lit_char(char val, int line, int col) {
    ASTNode *node = ast_create_node(AST_LIT_CHAR, line, col);
    node->char_val = val;
    node->data_type = data_type_primitive(TYPE_CHAR);
    return node;
}

ASTNode *ast_create_lit_string(const char *val, int line, int col) {
    ASTNode *node = ast_create_node(AST_LIT_STRING, line, col);
    node->str_val = val ? strdup(val) : NULL;
    node->data_type = data_type_array(data_type_primitive(TYPE_CHAR), 0);
    return node;
}

ASTNode *ast_create_lit_bool(bool val, int line, int col) {
    ASTNode *node = ast_create_node(AST_LIT_BOOL, line, col);
    node->int_val = val ? 1 : 0;
    node->data_type = data_type_primitive(TYPE_BOOL);
    return node;
}

ASTNode *ast_append_sibling(ASTNode *list, ASTNode *item) {
    if (!list) return item;
    if (!item) return list;
    ASTNode *curr = list;
    while (curr->next) {
        curr = curr->next;
    }
    curr->next = item;
    return list;
}

/*
 * Liberação recursiva da AST
 */
void ast_free(ASTNode *node) {
    while (node) {
        ASTNode *next = node->next;

        if (node->left) ast_free(node->left);
        if (node->right) ast_free(node->right);
        if (node->extra) ast_free(node->extra);

        if (node->name) free(node->name);
        if (node->str_val) free(node->str_val);
        if (node->data_type.struct_name) free(node->data_type.struct_name);

        free(node);
        node = next;
    }
}

/*
 * Visualização e impressão da AST (implementação preliminar/base)
 */
static void print_indent(int indent) {
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
}

void ast_print(const ASTNode *node, int indent) {
    while (node) {
        print_indent(indent);
        switch (node->type) {
            case AST_PROGRAM:
                printf("Program\n");
                if (node->left) ast_print(node->left, indent + 1);
                break;
            case AST_VAR_DECL:
                printf("VarDecl: %s (Type: %s)\n", node->name ? node->name : "?", data_type_to_string(&node->data_type));
                if (node->left) {
                    print_indent(indent + 1);
                    printf("Init:\n");
                    ast_print(node->left, indent + 2);
                }
                break;
            case AST_FUNC_DEF:
                printf("FuncDef: %s (Return: %s)\n", node->name ? node->name : "?", data_type_to_string(&node->data_type));
                if (node->left) {
                    print_indent(indent + 1);
                    printf("Params:\n");
                    ast_print(node->left, indent + 2);
                }
                if (node->right) {
                    print_indent(indent + 1);
                    printf("Body:\n");
                    ast_print(node->right, indent + 2);
                }
                break;
            case AST_PARAM_DECL:
                printf("Param: %s (Type: %s)\n", node->name ? node->name : "?", data_type_to_string(&node->data_type));
                break;
            case AST_STRUCT_DECL:
                printf("StructDecl: %s\n", node->name ? node->name : "?");
                if (node->left) {
                    print_indent(indent + 1);
                    printf("Members:\n");
                    ast_print(node->left, indent + 2);
                }
                break;
            case AST_BLOCK:
                printf("Block\n");
                if (node->left) ast_print(node->left, indent + 1);
                break;
            case AST_EXPR_STMT:
                printf("ExprStmt\n");
                if (node->left) ast_print(node->left, indent + 1);
                break;
            case AST_IF:
                printf("If\n");
                print_indent(indent + 1);
                printf("Cond:\n");
                ast_print(node->left, indent + 2);
                print_indent(indent + 1);
                printf("Then:\n");
                ast_print(node->right, indent + 2);
                if (node->extra) {
                    print_indent(indent + 1);
                    printf("Else:\n");
                    ast_print(node->extra, indent + 2);
                }
                break;
            case AST_WHILE:
                printf("While\n");
                print_indent(indent + 1);
                printf("Cond:\n");
                ast_print(node->left, indent + 2);
                print_indent(indent + 1);
                printf("Body:\n");
                ast_print(node->right, indent + 2);
                break;
            case AST_FOR:
                printf("For\n");
                if (node->left) {
                    print_indent(indent + 1);
                    printf("Init:\n");
                    ast_print(node->left, indent + 2);
                }
                if (node->right) {
                    print_indent(indent + 1);
                    printf("Cond:\n");
                    ast_print(node->right, indent + 2);
                }
                if (node->extra) {
                    print_indent(indent + 1);
                    printf("Step / Body:\n");
                    ast_print(node->extra, indent + 2);
                }
                break;
            case AST_SWITCH:
                printf("Switch\n");
                if (node->left) ast_print(node->left, indent + 1);
                if (node->right) ast_print(node->right, indent + 1);
                break;
            case AST_CASE:
                printf("Case\n");
                if (node->left) ast_print(node->left, indent + 1);
                if (node->right) ast_print(node->right, indent + 1);
                break;
            case AST_DEFAULT:
                printf("Default\n");
                if (node->right) ast_print(node->right, indent + 1);
                break;
            case AST_BREAK:
                printf("Break\n");
                break;
            case AST_CONTINUE:
                printf("Continue\n");
                break;
            case AST_RETURN:
                printf("Return\n");
                if (node->left) ast_print(node->left, indent + 1);
                break;
            case AST_BIN_OP:
                printf("BinOp (op: %d)\n", node->op);
                if (node->left) ast_print(node->left, indent + 1);
                if (node->right) ast_print(node->right, indent + 1);
                break;
            case AST_UN_OP:
                printf("UnOp (op: %d)\n", node->op);
                if (node->left) ast_print(node->left, indent + 1);
                break;
            case AST_ASSIGN:
                printf("Assign (op: %d)\n", node->op);
                if (node->left) ast_print(node->left, indent + 1);
                if (node->right) ast_print(node->right, indent + 1);
                break;
            case AST_CALL:
                printf("Call: %s\n", node->name ? node->name : "?");
                if (node->left) {
                    print_indent(indent + 1);
                    printf("Args:\n");
                    ast_print(node->left, indent + 2);
                }
                break;
            case AST_ARRAY_ACCESS:
                printf("ArrayAccess\n");
                print_indent(indent + 1);
                printf("Base:\n");
                ast_print(node->left, indent + 2);
                print_indent(indent + 1);
                printf("Index:\n");
                ast_print(node->right, indent + 2);
                break;
            case AST_MEMBER_ACCESS:
                printf("MemberAccess (.%s)\n", node->name ? node->name : "?");
                if (node->left) ast_print(node->left, indent + 1);
                break;
            case AST_IDENTIFIER:
                printf("Identifier: %s\n", node->name ? node->name : "?");
                break;
            case AST_LIT_INT:
                printf("LiteralInt: %d\n", node->int_val);
                break;
            case AST_LIT_FLOAT:
                printf("LiteralFloat: %f\n", node->float_val);
                break;
            case AST_LIT_CHAR:
                printf("LiteralChar: '%c'\n", node->char_val);
                break;
            case AST_LIT_STRING:
                printf("LiteralString: \"%s\"\n", node->str_val ? node->str_val : "");
                break;
            case AST_LIT_BOOL:
                printf("LiteralBool: %s\n", node->int_val ? "true" : "false");
                break;
            default:
                printf("UnknownNode (%d)\n", node->type);
                break;
        }
        node = node->next;
    }
}
