#include "../include/evaluator.h"

void _eval(node_t* node, vars_t* vars, obj_t* ret) {
    if (node == NULL) {
        return;
    }

    switch (node->tok->type) {
        case TOK_ID:
            ret->str_val = node->tok->data;
            ret->type = OBJ_STR;
            break;
        
        case TOK_NUM: {
            char buf[32];
            wcstombs(buf, node->tok->data, 32);
            ret->int_val = atoi(buf);
            ret->type = OBJ_INT;
            break;
        }

        case TOK_OP: {
            if (node->left == NULL || node->right == NULL) {
                printf("Missing operand for operator `%ls`\n", node->tok->data);
                exit(1);
            }

            if (wcscmp(node->tok->data, L"=") == 0) {
                _eval(node->left, vars, ret);
                if (ret->type != OBJ_STR) {
                    printf("Error: expecting a variable for assignment\n");
                    exit(1);
                }
                wchar_t* id = ret->str_val;
                _eval(node->right, vars, ret);
            } else if (wcscmp(node->tok->data, L"+") == 0) {
                _eval(node->left, vars, ret);
                if (ret->type != OBJ_INT) {
                    printf("Error: invalid operand for operator `%ls`\n", node->tok->data);
                    exit(1);
                }
                int left = ret->int_val;
                _eval(node->right, vars, ret);
                if (ret->type != OBJ_INT) {
                    printf("Error: invalid operand for operator `%ls`\n", node->tok->data);
                    exit(1);
                }
                int right = ret->int_val;
                ret->int_val = left + right;
            } else if (wcscmp(node->tok->data, L"-") == 0) {
                _eval(node->left, vars, ret);
                if (ret->type != OBJ_INT) {
                    printf("Error: invalid operand for operator `%ls`\n", node->tok->data);
                    exit(1);
                }
                int left = ret->int_val;
                _eval(node->right, vars, ret);
                if (ret->type != OBJ_INT) {
                    printf("Error: invalid operand for operator `%ls`\n", node->tok->data);
                    exit(1);
                }
                int right = ret->int_val;
                ret->int_val = left - right;
            } else if (wcscmp(node->tok->data, L"*") == 0) {
                _eval(node->left, vars, ret);
                if (ret->type != OBJ_INT) {
                    printf("Error: invalid operand for operator `%ls`\n", node->tok->data);
                    exit(1);
                }
                int left = ret->int_val;
                _eval(node->right, vars, ret);
                if (ret->type != OBJ_INT) {
                    printf("Error: invalid operand for operator `%ls`\n", node->tok->data);
                    exit(1);
                }
                int right = ret->int_val;
                ret->int_val = left * right;
            } else if (wcscmp(node->tok->data, L"/") == 0) {
                _eval(node->left, vars, ret);
                if (ret->type != OBJ_INT) {
                    printf("Error: invalid operand for operator `%ls`\n", node->tok->data);
                    exit(1);
                }
                int left = ret->int_val;
                _eval(node->right, vars, ret);
                if (ret->type != OBJ_INT) {
                    printf("Error: invalid operand for operator `%ls`\n", node->tok->data);
                    exit(1);
                }
                int right = ret->int_val;
                ret->int_val = left / right;
            } else {

            }
            break;
        }

        default:
            break;
    }
}

obj_t* eval(parser_t* parser) {
    obj_t ret;
    _eval(parser->parseTree, parser->vars, &ret);
    printf("%d\n", ret.int_val);
    return NULL;
}