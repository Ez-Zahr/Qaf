#include "../include/evaluator.h"

node_t* _eval(node_t* node, vars_t* vars) {
    if (node == NULL) {
        return NULL;
    }

    switch (node->tok->type) {
        case TOK_ID: {
            return node;
        }
        
        case TOK_NUM: {
            char buf[32];
            wcstombs(buf, node->tok->data, 32);
            node->tok->val = atoi(buf);
            return node;
        }

        case TOK_ASSIGN: {
            node_t* left = _eval(node->left, vars);
            node_t* right = _eval(node->right, vars);
            if (left == NULL || right == NULL || left->tok->type != TOK_ID || right->tok->type != TOK_NUM) {
                printf("Error: Invalid operand to `+` operator\n");
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
            node_t* left = _eval(node->left, vars);
            node_t* right = _eval(node->right, vars);
            if (left == NULL || right == NULL || left->tok->type != TOK_NUM || right->tok->type != TOK_NUM) {
                printf("Error: Invalid operand to `+` operator\n");
                exit(1);
            }
            node->tok->val = left->tok->val + right->tok->val;
            node->tok->type = TOK_NUM;
            return node;
        }

        case TOK_MINUS: {
            node_t* left = _eval(node->left, vars);
            node_t* right = _eval(node->right, vars);
            if (left == NULL || right == NULL || left->tok->type != TOK_NUM || right->tok->type != TOK_NUM) {
                printf("Error: Invalid operand to `-` operator\n");
                exit(1);
            }
            node->tok->val = left->tok->val - right->tok->val;
            node->tok->type = TOK_NUM;
            return node;
        }

        case TOK_MUL: {
            node_t* left = _eval(node->left, vars);
            node_t* right = _eval(node->right, vars);
            if (left == NULL || right == NULL || left->tok->type != TOK_NUM || right->tok->type != TOK_NUM) {
                printf("Error: Invalid operand to `*` operator\n");
                exit(1);
            }
            node->tok->val = left->tok->val * right->tok->val;
            node->tok->type = TOK_NUM;
            return node;
        }

        case TOK_DIV: {
            node_t* left = _eval(node->left, vars);
            node_t* right = _eval(node->right, vars);
            if (left == NULL || right == NULL || left->tok->type != TOK_NUM || right->tok->type != TOK_NUM) {
                printf("Error: Invalid operand to `/` operator\n");
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