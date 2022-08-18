#include "../include/parser.h"

extern ERROR_STATUS err_status;

void init_parser(parser_t* parser) {
    parser->cap = 32;
    parser->size = 0;
    parser->astList = (node_t**) calloc(parser->cap, sizeof(node_t*));
}

int get_op_priority(tok_type_t op) {
    switch (op) {
        case TOK_ASSIGN:
            return PRI_ASSIGN;
        
        case TOK_EQ:
        case TOK_NE:
        case TOK_LT:
        case TOK_LTE:
        case TOK_GT:
        case TOK_GTE:
            return PRI_CMP;
        
        case TOK_AND:
        case TOK_OR:
            return PRI_AND_OR;
        
        case TOK_PLUS:
        case TOK_MINUS:
            return PRI_ADD_SUB;
        
        case TOK_MUL:
        case TOK_DIV:
        case TOK_MOD:
            return PRI_MUL_DIV;
        
        default:
            return -1;
    }
}

node_t* _parse(lexer_t* lexer, int prior);

node_t* parse_primary_expr(lexer_t* lexer) {
    switch (lexer->tokens[lexer->pos].type) {
        case TOK_BOOL:
        case TOK_FLOAT:
        case TOK_INT:
        case TOK_CHAR:
        case TOK_STR:
        case TOK_ID: {
            node_t* prim_node = (node_t*) calloc(1, sizeof(node_t));
            prim_node->tok = &lexer->tokens[lexer->pos++];
            return prim_node;
        }

        case TOK_MINUS:
        case TOK_NOT: {
            node_t* expr = (node_t*) calloc(1, sizeof(node_t));
            expr->tok = &lexer->tokens[lexer->pos++];
            expr->left = parse_primary_expr(lexer);
            return expr;
        }

        case TOK_IF: 
        case TOK_WHILE: {
            node_t* expr = (node_t*) calloc(1, sizeof(node_t));
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
            node_t* left = parse_primary_expr(lexer);
            if (err_status != ERR_NONE) {
                return left;
            }
            lexer->pos++;
            node_t* right = parse_primary_expr(lexer);
            if (err_status != ERR_NONE) {
                left->right = right;
                return left;
            }
            node_t* expr = parse_primary_expr(lexer);
            expr->tok = tok;
            expr->left = left;
            expr->right = right;
            return expr;
        }

        case TOK_LPAREN: {
            lexer->pos++;
            node_t* expr = _parse(lexer, 1);
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
            node_t* exprList = (node_t*) calloc(1, sizeof(node_t));
            exprList->tok = &lexer->tokens[lexer->pos++];
            exprList->cap = 32;
            exprList->size = 0;
            exprList->astList = (node_t**) calloc(exprList->cap, sizeof(node_t*));
            while (lexer->tokens[lexer->pos].type != TOK_RBRACE) {
                if (lexer->tokens[lexer->pos].type == TOK_SEMI) {
                    lexer->pos++;
                    continue;
                } else if (lexer->tokens[lexer->pos].type == TOK_EOF) {
                    wprintf(L"Error: Missing closing brace\n");
                    err_status = ERR_PARSE;
                    return exprList;
                }
                exprList->astList[exprList->size++] = _parse(lexer, 0);
                if (err_status != ERR_NONE) {
                    return exprList;
                }
                if (exprList->size >= exprList->cap) {
                    exprList->cap *= 2;
                    exprList->astList = (node_t**) realloc(exprList->astList, exprList->cap * sizeof(node_t*));
                }
            }
            lexer->pos++;
            return exprList;
        }

        case TOK_LBRACK: {
            node_t* exprList = (node_t*) calloc(1, sizeof(node_t));
            exprList->tok = &lexer->tokens[lexer->pos++];
            exprList->cap = 8;
            exprList->size = 0;
            exprList->astList = (node_t**) calloc(exprList->cap, sizeof(node_t*));
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
                exprList->astList[exprList->size++] = parse_primary_expr(lexer);
                if (err_status != ERR_NONE) {
                    return exprList;
                }
                if (exprList->size >= exprList->cap) {
                    exprList->cap *= 2;
                    exprList->astList = (node_t**) realloc(exprList->astList, exprList->cap * sizeof(node_t*));
                }
            }
            lexer->pos++;
            return exprList;
        }

        case TOK_PRINT: {
            node_t* expr = (node_t*) calloc(1, sizeof(node_t));
            expr->tok = &lexer->tokens[lexer->pos++];
            expr->left = _parse(lexer, 1);
            return expr;
        }

        case TOK_READ: {
            node_t* expr = (node_t*) calloc(1, sizeof(node_t));
            expr->tok = &lexer->tokens[lexer->pos++];
            if (lexer->tokens[lexer->pos].type != TOK_ID) {
                wprintf(L"Error: Expected an ID token after a READ token\n");
                err_status = ERR_PARSE;
                return expr;
            }
            expr->left = (node_t*) calloc(1, sizeof(node_t));
            expr->left->tok = &lexer->tokens[lexer->pos++];
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

node_t* _parse(lexer_t* lexer, int prior) {
    node_t* expr = (prior == PRI_MAX - 1)? parse_primary_expr(lexer) : _parse(lexer, prior + 1);
    if (err_status != ERR_NONE) {
        return expr;
    }
    
    while (get_op_priority(lexer->tokens[lexer->pos].type) == prior) {
        node_t* op_node = (node_t*) calloc(1, sizeof(node_t));
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

void parse(lexer_t* lexer, parser_t* parser) {
    while (1) {
        if (lexer->tokens[lexer->pos].type == TOK_SEMI) {
            lexer->pos++;
            continue;
        } else if (lexer->tokens[lexer->pos].type == TOK_EOF) {
            break;
        }

        parser->astList[parser->size++] = _parse(lexer, 0);
        if (err_status != ERR_NONE) {
            return;
        }
        
        if (parser->size >= parser->cap) {
            parser->cap *= 2;
            parser->astList = (node_t**) realloc(parser->astList, parser->cap * sizeof(node_t*));
        }
    }
}

void _print_ast(node_t* node, int indent) {
    if (node == NULL) {
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
            _print_ast(node->astList[i], indent + 1);
        }
    }
}

void print_parser(parser_t* parser) {
    for (int i = 0; i < parser->size; i++) {
        _print_ast(parser->astList[i], 0);
    }
}

void _free_ast(node_t* node) {
    if (node == NULL) {
        return;
    }

    _free_ast(node->left);
    _free_ast(node->right);
    if (node->cap) {
        for (int i = 0; i < node->size; i++) {
            _free_ast(node->astList[i]);
        }
        free(node->astList);
    }
    free(node);
}

void free_parser(parser_t* parser) {
    for (int i = 0; i < parser->size; i++) {
        _free_ast(parser->astList[i]);
    }
    free(parser->astList);
}