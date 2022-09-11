#include "../include/evaluator.h"

int eval(ast_t* ast) {
    switch (ast->tok->type) {
        case TOK_INT: {
            char buf[32];
            wcstombs(buf, ast->tok->data, 32);
            return atoi(buf);
        }

        case TOK_FLOAT: {
            char buf[32];
            wcstombs(buf, ast->tok->data, 32);
            return atof(buf);
        }
        
        case TOK_BOOL: {
            return !wcscmp(ast->tok->data, L"صح");
        }
        
        case TOK_PLUS: {
            int left = eval(ast->left);
            int right = eval(ast->right);
            return left + right;
        }
        
        case TOK_MINUS: {
            int left = eval(ast->left);
            int right = eval(ast->right);
            return left - right;
        }
        
        case TOK_MUL: {
            int left = eval(ast->left);
            int right = eval(ast->right);
            return left * right;
        }
        
        case TOK_DIV: {
            int left = eval(ast->left);
            int right = eval(ast->right);
            return left / right;
        }
        
        case TOK_MOD: {
            int left = eval(ast->left);
            int right = eval(ast->right);
            return left % right;
        }
        
        case TOK_AND: {
            int left = eval(ast->left);
            int right = eval(ast->right);
            return left && right;
        }
        
        case TOK_OR: {
            int left = eval(ast->left);
            int right = eval(ast->right);
            return left || right;
        }
        
        case TOK_NOT: {
            int left = eval(ast->left);
            return !left;
        }

        case TOK_EQ: {
            int left = eval(ast->left);
            int right = eval(ast->right);
            return left == right;
        }
        
        case TOK_NE: {
            int left = eval(ast->left);
            int right = eval(ast->right);
            return left != right;
        }
        
        case TOK_LT: {
            int left = eval(ast->left);
            int right = eval(ast->right);
            return left < right;
        }
        
        case TOK_LTE: {
            int left = eval(ast->left);
            int right = eval(ast->right);
            return left <= right;
        }
        
        case TOK_GT: {
            int left = eval(ast->left);
            int right = eval(ast->right);
            return left > right;
        }
        
        case TOK_GTE: {
            int left = eval(ast->left);
            int right = eval(ast->right);
            return left >= right;
        }

        default: {
            wprintf(L"Error: Cannot evaluate token of type %ls\n", tok_type_to_str(ast->tok->type));
            smart_exit(ERR_EVAL);
        }
    }
}