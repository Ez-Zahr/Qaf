#include "../include/evaluator.h"

node_t* get_primary_expr(node_t* node, vars_t* vars) {
    switch (node->tok->type) {
        case TOK_ID: {
            if (is_var(vars, node->tok->data)) {
                node->tok = get_var_val(vars, node->tok->data);
                return node;
            } else {
                wprintf(L"Error: Undefined variable `%ls`\n", node->tok->data);
                exit(1);
            }
        }

        case TOK_NUM:
            return node;
        
        default:
            wprintf(L"Error: Invalid primary expression\n");
            exit(1);
    }
}

node_t* _eval(node_t* node, vars_t* vars) {
    if (node == NULL) {
        return NULL;
    }

    switch (node->tok->type) {
        case TOK_ID: {
            return node;
        }

        case TOK_KEYWORD: {
            if (!wcscmp(node->tok->data, L"اطبع")) {
                node_t* left = get_primary_expr(_eval(node->left, vars), vars);
                wprintf(L"%d\n", left->tok->val);
            }
        }
        
        case TOK_NUM: {
            char buf[32];
            wcstombs(buf, node->tok->data, 32);
            node->tok->val = atoi(buf);
            return node;
        }

        case TOK_ASSIGN: {
            node_t* left = _eval(node->left, vars);
            node_t* right = get_primary_expr(_eval(node->right, vars), vars);
            if (left == NULL || right == NULL || left->tok->type != TOK_ID || right->tok->type != TOK_NUM) {
                wprintf(L"Error: Invalid operand to `+` operator\n");
                exit(1);
            }
            if (is_var(vars, left->tok->data)) {
                update_var(vars, left->tok->data, right->tok);
            } else {
                add_var(vars, left->tok->data, right->tok);
            }
            return right;
        }

        case TOK_PLUS: {
            node_t* left = get_primary_expr(_eval(node->left, vars), vars);
            node_t* right = get_primary_expr(_eval(node->right, vars), vars);
            if (left == NULL || right == NULL || left->tok->type != TOK_NUM || right->tok->type != TOK_NUM) {
                wprintf(L"Error: Invalid operand to `+` operator\n");
                exit(1);
            }
            node->tok->val = left->tok->val + right->tok->val;
            node->tok->type = TOK_NUM;
            return node;
        }

        case TOK_MINUS: {
            node_t* left = get_primary_expr(_eval(node->left, vars), vars);
            node_t* right = get_primary_expr(_eval(node->right, vars), vars);
            if (left == NULL || right == NULL || left->tok->type != TOK_NUM || right->tok->type != TOK_NUM) {
                wprintf(L"Error: Invalid operand to `-` operator\n");
                exit(1);
            }
            node->tok->val = left->tok->val - right->tok->val;
            node->tok->type = TOK_NUM;
            return node;
        }

        case TOK_MUL: {
            node_t* left = get_primary_expr(_eval(node->left, vars), vars);
            node_t* right = get_primary_expr(_eval(node->right, vars), vars);
            if (left == NULL || right == NULL || left->tok->type != TOK_NUM || right->tok->type != TOK_NUM) {
                wprintf(L"Error: Invalid operand to `*` operator\n");
                exit(1);
            }
            node->tok->val = left->tok->val * right->tok->val;
            node->tok->type = TOK_NUM;
            return node;
        }

        case TOK_DIV: {
            node_t* left = get_primary_expr(_eval(node->left, vars), vars);
            node_t* right = get_primary_expr(_eval(node->right, vars), vars);
            if (left == NULL || right == NULL || left->tok->type != TOK_NUM || right->tok->type != TOK_NUM) {
                wprintf(L"Error: Invalid operand to `/` operator\n");
                exit(1);
            }
            node->tok->val = left->tok->val / right->tok->val;
            node->tok->type = TOK_NUM;
            return node;
        }

        default:
            return NULL;
    }
}

void eval(parser_t* parser) {
    _eval(parser->parseTree, parser->vars);
}