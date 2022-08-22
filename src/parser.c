#include "../include/parser.h"

extern ERROR_STATUS err_status;

void init_ast_list(ast_t* ast) {
    ast->cap = 8;
    ast->size = 0;
    ast->list = (ast_t**) calloc(ast->cap, sizeof(ast_t*));
}

int get_op_priority(tok_type_t op) {
    switch (op) {
        case TOK_ASSIGN:
            return PRI_ASSIGN;
        case TOK_AND:
        case TOK_OR:
            return PRI_LOGIC;
        case TOK_EQ:
        case TOK_NE:
        case TOK_LT:
        case TOK_LTE:
        case TOK_GT:
        case TOK_GTE:
            return PRI_CMP;
        case TOK_PLUS:
        case TOK_MINUS:
            return PRI_TERM;
        case TOK_MUL:
        case TOK_DIV:
        case TOK_MOD:
            return PRI_FACTOR;
        default:
            return -1;
    }
}

ast_t* _parse(lexer_t* lexer, int prior);

ast_t* parse_primary_expr(lexer_t* lexer) {
    switch (lexer->tokens[lexer->pos].type) {
        case TOK_BOOL:
        case TOK_FLOAT:
        case TOK_INT:
        case TOK_CHAR:
        case TOK_STR:
        case TOK_READ:
        case TOK_ID: {
            ast_t* prim_node = (ast_t*) calloc(1, sizeof(ast_t));
            prim_node->tok = &lexer->tokens[lexer->pos++];
            return prim_node;
        }

        case TOK_MINUS:
        case TOK_NOT: {
            ast_t* expr = (ast_t*) calloc(1, sizeof(ast_t));
            expr->tok = &lexer->tokens[lexer->pos++];
            expr->left = parse_primary_expr(lexer);
            return expr;
        }

        case TOK_IF: 
        case TOK_WHILE: {
            ast_t* expr = (ast_t*) calloc(1, sizeof(ast_t));
            expr->tok = &lexer->tokens[lexer->pos++];
            expr->left = parse_primary_expr(lexer);
            if (err_status != ERR_NONE) {
                return expr;
            }
            expr->right = parse_primary_expr(lexer);
            return expr;
        }

        case TOK_FOR: {
            token_t* tok = &lexer->tokens[lexer->pos++];
            ast_t* left = parse_primary_expr(lexer);
            if (err_status != ERR_NONE) {
                return left;
            }
            lexer->pos++;
            ast_t* right = parse_primary_expr(lexer);
            if (err_status != ERR_NONE) {
                left->right = right;
                return left;
            }
            ast_t* expr = parse_primary_expr(lexer);
            expr->tok = tok;
            expr->left = left;
            expr->right = right;
            return expr;
        }

        case TOK_LPAREN: {
            lexer->pos++;
            ast_t* expr = _parse(lexer, 1);
            if (err_status != ERR_NONE) {
                return expr;
            }
            if (lexer->tokens[lexer->pos].type != TOK_RPAREN) {
                wprintf(L"Error: Missing closing parenthesis\n");
                err_status = ERR_PARSE;
                return expr;
            }
            lexer->pos++;
            return expr;
        }

        case TOK_LBRACE: {
            ast_t* exprList = (ast_t*) calloc(1, sizeof(ast_t));
            exprList->tok = &lexer->tokens[lexer->pos++];
            init_ast_list(exprList);
            while (lexer->tokens[lexer->pos].type != TOK_RBRACE) {
                if (lexer->tokens[lexer->pos].type == TOK_SEMI) {
                    lexer->pos++;
                    continue;
                } else if (lexer->tokens[lexer->pos].type == TOK_EOF) {
                    wprintf(L"Error: Missing closing brace\n");
                    err_status = ERR_PARSE;
                    return exprList;
                }
                exprList->list[exprList->size++] = _parse(lexer, 0);
                if (err_status != ERR_NONE) {
                    return exprList;
                }
                if (exprList->size >= exprList->cap) {
                    exprList->cap *= 2;
                    exprList->list = (ast_t**) realloc(exprList->list, exprList->cap * sizeof(ast_t*));
                }
            }
            lexer->pos++;
            return exprList;
        }

        case TOK_LBRACK: {
            ast_t* exprList = (ast_t*) calloc(1, sizeof(ast_t));
            exprList->tok = &lexer->tokens[lexer->pos++];
            init_ast_list(exprList);
            while (lexer->tokens[lexer->pos].type != TOK_RBRACK) {
                if (lexer->tokens[lexer->pos].type == TOK_COLON) {
                    lexer->pos++;
                    continue;
                } else if (lexer->tokens[lexer->pos].type == TOK_SEMI) {
                    wprintf(L"Error: Invalid semicolon\n");
                    err_status = ERR_PARSE;
                    return exprList;
                } else if (lexer->tokens[lexer->pos].type == TOK_EOF) {
                    wprintf(L"Error: Missing closing bracket\n");
                    err_status = ERR_PARSE;
                    return exprList;
                }
                exprList->list[exprList->size++] = parse_primary_expr(lexer);
                if (err_status != ERR_NONE) {
                    return exprList;
                }
                if (exprList->size >= exprList->cap) {
                    exprList->cap *= 2;
                    exprList->list = (ast_t**) realloc(exprList->list, exprList->cap * sizeof(ast_t*));
                }
            }
            lexer->pos++;
            return exprList;
        }

        case TOK_PRINT: {
            ast_t* expr = (ast_t*) calloc(1, sizeof(ast_t));
            expr->tok = &lexer->tokens[lexer->pos++];
            expr->left = _parse(lexer, 1);
            return expr;
        }

        default: {
            token_t tok = lexer->tokens[lexer->pos];
            wprintf(L"Error: Invalid token `%ls` of type `%ls`\n", tok.data, tok_type_to_str(tok.type));
            err_status = ERR_PARSE;
            return 0;
        }
    }
}

ast_t* _parse(lexer_t* lexer, int prior) {
    ast_t* expr = (prior == PRI_MAX - 1)? parse_primary_expr(lexer) : _parse(lexer, prior + 1);
    if (err_status != ERR_NONE) {
        return expr;
    }
    
    while (get_op_priority(lexer->tokens[lexer->pos].type) == prior) {
        ast_t* op_node = (ast_t*) calloc(1, sizeof(ast_t));
        op_node->tok = &lexer->tokens[lexer->pos++];
        op_node->left = expr;
        op_node->right = (prior == PRI_MAX - 1)? parse_primary_expr(lexer) : _parse(lexer, prior + 1);
        if (err_status != ERR_NONE) {
            return op_node;
        }
        expr = op_node;
    }

    return expr;
}

void parse(lexer_t* lexer, ast_t* root) {
    while (1) {
        if (lexer->tokens[lexer->pos].type == TOK_SEMI) {
            lexer->pos++;
            continue;
        } else if (lexer->tokens[lexer->pos].type == TOK_EOF) {
            break;
        }

        root->list[root->size++] = _parse(lexer, 0);
        if (err_status != ERR_NONE) {
            return;
        }
        
        if (root->size >= root->cap) {
            root->cap *= 2;
            root->list = (ast_t**) realloc(root->list, root->cap * sizeof(ast_t*));
        }
    }
}

void _print_ast(ast_t* node, int indent) {
    if (!node) {
        return;
    }

    wprintf(L"%d: ", indent);
    for (int i = 0; i < indent; i++) {
        wprintf(L" ");
    }
    wprintf(L"`%ls`\n", node->tok->data);
    
    _print_ast(node->left, indent + 1);
    _print_ast(node->right, indent + 1);

    if (node->cap > 0) {
        for (int i = 0; i < node->size; i++) {
            _print_ast(node->list[i], indent + 1);
        }
    }
}

void print_ast_root(ast_t* root) {
    for (int i = 0; i < root->size; i++) {
        _print_ast(root->list[i], 0);
    }
}

void _free_ast(ast_t* ast) {
    if (!ast) {
        return;
    }

    _free_ast(ast->left);
    _free_ast(ast->right);
    if (ast->list) {
        for (int i = 0; i < ast->size; i++) {
            _free_ast(ast->list[i]);
        }
        free(ast->list);
    }
    free(ast);
}

void free_ast(ast_t* ast) {
    for (int i = 0; i < ast->size; i++) {
        _free_ast(ast->list[i]);
    }
    free(ast->list);
}