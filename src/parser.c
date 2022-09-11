#include "../include/parser.h"

ast_t* init_ast_list() {
    ast_t* ast = (ast_t*) smart_alloc(1, sizeof(ast_t));
    ast->list = (ast_t**) smart_alloc(1, sizeof(ast_t*));
    ast->size = 0;
    return ast;
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
        case TOK_READ: {
            ast_t* prim_node = (ast_t*) smart_alloc(1, sizeof(ast_t));
            prim_node->tok = &lexer->tokens[lexer->pos++];
            return prim_node;
        }

        case TOK_ID: {
            ast_t* prim_node = (ast_t*) smart_alloc(1, sizeof(ast_t));
            prim_node->tok = &lexer->tokens[lexer->pos++];
            if (lexer->tokens[lexer->pos].type == TOK_LPAREN) {
                prim_node->left = parse_primary_expr(lexer);
            }
            return prim_node;
        }

        case TOK_LET:
        case TOK_MINUS:
        case TOK_NOT: {
            ast_t* expr = (ast_t*) smart_alloc(1, sizeof(ast_t));
            expr->tok = &lexer->tokens[lexer->pos++];
            expr->left = parse_primary_expr(lexer);
            return expr;
        }

        case TOK_FUNC: {
            ast_t* expr = (ast_t*) smart_alloc(1, sizeof(ast_t));
            expr->list = (ast_t**) smart_alloc(3, sizeof(ast_t*));
            expr->size = 3;
            expr->tok = &lexer->tokens[lexer->pos++];
            expr->list[0] = (ast_t*) smart_alloc(1, sizeof(ast_t));
            expr->list[0]->tok = &lexer->tokens[lexer->pos++];
            expr->list[1] = parse_primary_expr(lexer);
            expr->list[2] = parse_primary_expr(lexer);
            return expr;
        }

        case TOK_IF: 
        case TOK_WHILE: {
            ast_t* expr = (ast_t*) smart_alloc(1, sizeof(ast_t));
            expr->tok = &lexer->tokens[lexer->pos++];
            expr->left = parse_primary_expr(lexer);
            if (expr->left->size != 1) {
                wprintf(L"Error: Invalid number of arguments inside the condition parentheses\n");
                smart_exit(ERR_PARSE);
            }
            expr->right = parse_primary_expr(lexer);
            return expr;
        }

        case TOK_FOR: {
            ast_t* expr = (ast_t*) smart_alloc(1, sizeof(ast_t));
            expr->list = (ast_t**) smart_alloc(3, sizeof(ast_t*));
            expr->size = 3;
            expr->tok = &lexer->tokens[lexer->pos++];
            expr->list[0] = parse_primary_expr(lexer);
            lexer->pos++;
            expr->list[1] = parse_primary_expr(lexer);
            if (expr->list[1]->size != 3) {
                wprintf(L"Error: Invalid number of arguments inside the for-loop brackets\n");
                smart_exit(ERR_PARSE);
            }
            expr->list[2] = parse_primary_expr(lexer);
            return expr;
        }

        case TOK_LPAREN: {
            ast_t* exprList = init_ast_list();
            exprList->tok = &lexer->tokens[lexer->pos++];
            while (lexer->tokens[lexer->pos].type != TOK_RPAREN) {
                if (lexer->tokens[lexer->pos].type == TOK_COMMA) {
                    lexer->pos++;
                    continue;
                }
                exprList->list[exprList->size++] = _parse(lexer, PRI_ASSIGN + 1);
                exprList->list = (ast_t**) smart_realloc(exprList->list, exprList->size + 1, sizeof(ast_t*));
            }
            lexer->pos++;
            return exprList;
        }

        case TOK_LBRACE: {
            ast_t* exprList = init_ast_list();
            exprList->tok = &lexer->tokens[lexer->pos++];
            while (lexer->tokens[lexer->pos].type != TOK_RBRACE) {
                if (lexer->tokens[lexer->pos].type == TOK_SEMI) {
                    lexer->pos++;
                    continue;
                }
                exprList->list[exprList->size++] = _parse(lexer, PRI_ASSIGN);
                exprList->list = (ast_t**) smart_realloc(exprList->list, exprList->size + 1, sizeof(ast_t*));
            }
            lexer->pos++;
            return exprList;
        }

        case TOK_LBRACK: {
            ast_t* exprList = init_ast_list();
            exprList->tok = &lexer->tokens[lexer->pos++];
            while (lexer->tokens[lexer->pos].type != TOK_RBRACK) {
                if (lexer->tokens[lexer->pos].type == TOK_COLON) {
                    lexer->pos++;
                    continue;
                }
                exprList->list[exprList->size++] = parse_primary_expr(lexer);
                exprList->list = (ast_t**) smart_realloc(exprList->list, exprList->size + 1, sizeof(ast_t*));
            }
            lexer->pos++;
            return exprList;
        }

        case TOK_PRINT: {
            ast_t* expr = (ast_t*) smart_alloc(1, sizeof(ast_t));
            expr->tok = &lexer->tokens[lexer->pos++];
            expr->left = _parse(lexer, PRI_ASSIGN + 1);
            return expr;
        }

        default: {
            token_t tok = lexer->tokens[lexer->pos];
            wprintf(L"Error: Invalid token `%ls` of type `%ls`\n", tok.data, tok_type_to_str(tok.type));
            smart_exit(ERR_PARSE);
        }
    }
}

ast_t* _parse(lexer_t* lexer, int prior) {
    ast_t* expr = (prior == PRI_MAX - 1)? parse_primary_expr(lexer) : _parse(lexer, prior + 1);
    
    while (get_op_priority(lexer->tokens[lexer->pos].type) == prior) {
        ast_t* op_node = (ast_t*) smart_alloc(1, sizeof(ast_t));
        op_node->tok = &lexer->tokens[lexer->pos++];
        op_node->left = expr;
        op_node->right = (prior == PRI_MAX - 1)? parse_primary_expr(lexer) : _parse(lexer, prior + 1);
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

        root->list[root->size++] = _parse(lexer, PRI_ASSIGN);
        
        root->list = (ast_t**) smart_realloc(root->list, root->size + 1, sizeof(ast_t*));
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

    for (int i = 0; i < node->size; i++) {
        _print_ast(node->list[i], indent + 1);
    }
}

void print_ast_list(ast_t* ast) {
    wprintf(L"----------<AST>----------\n");
    for (int i = 0; i < ast->size; i++) {
        _print_ast(ast->list[i], 0);
    }
    wprintf(L"-------------------------\n");
}