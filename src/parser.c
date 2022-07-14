#include "../include/parser.h"

void init_parser(parser_t* parser) {
    parser->parseTree = NULL;
}

node_t* parse_primary_expr(lexer_t* lexer) {
    switch (lexer->tokens[lexer->pos].type) {
        case TOK_NUM: {
            node_t* prim_node = (node_t*) calloc(1, sizeof(node_t));
            prim_node->tok = &lexer->tokens[lexer->pos++];
            return prim_node;
        }
        
        case TOK_ID: {
            node_t* prim_node = (node_t*) calloc(1, sizeof(node_t));
            prim_node->tok = &lexer->tokens[lexer->pos++];
            return prim_node;
        }

        case TOK_KEYWORD: {
            break;
        }
        
        case TOK_NEWLINE:
            lexer->pos++;
            return NULL;

        default:
            wprintf(L"Error: Invalid token `%ls` of type `%d`\n", lexer->tokens[lexer->pos].data, lexer->tokens[lexer->pos].type);
            exit(1);
    }
}

node_t* parse_factor(lexer_t* lexer) {
    node_t* expr = parse_primary_expr(lexer);
    
    while (lexer->tokens[lexer->pos].type == TOK_MUL || lexer->tokens[lexer->pos].type == TOK_DIV) {
        node_t* op_node = (node_t*) calloc(1, sizeof(node_t));
        op_node->tok = &lexer->tokens[lexer->pos++];
        op_node->left = expr;
        op_node->right = parse_primary_expr(lexer);
        expr = op_node;
    }

    return expr;
}

node_t* parse_term(lexer_t* lexer) {
    node_t* expr = parse_factor(lexer);
    
    while (lexer->tokens[lexer->pos].type == TOK_PLUS || lexer->tokens[lexer->pos].type == TOK_MINUS) {
        node_t* op_node = (node_t*) calloc(1, sizeof(node_t));
        op_node->tok = &lexer->tokens[lexer->pos++];
        op_node->left = expr;
        op_node->right = parse_factor(lexer);
        expr = op_node;
    }

    return expr;
}

void parse(lexer_t* lexer, parser_t* parser) {
    if (lexer->tokens[lexer->pos].type == TOK_KEYWORD) {
        node_t* keyword_node = (node_t*) calloc(1, sizeof(node_t));
        keyword_node->tok = &lexer->tokens[lexer->pos++];
        keyword_node->left = parse_term(lexer);
        parser->parseTree = keyword_node;
        return;
    }

    node_t* expr = parse_term(lexer);
    
    if (lexer->tokens[lexer->pos].type == TOK_ASSIGN) {
        node_t* op_node = (node_t*) calloc(1, sizeof(node_t));
        op_node->tok = &lexer->tokens[lexer->pos++];
        op_node->left = expr;
        op_node->right = parse_term(lexer);
        expr = op_node;
    }

    parser->parseTree = expr;
}

void print_tree(node_t* node, int indent) {
    if (node == NULL) {
        return;
    }

    wprintf(L"%d: ", indent);
    int i;
    for (i = 0; i < indent; i++) {
        wprintf(L" ");
    }
    
    wprintf(L"`%ls`\n", node->tok->data);
    print_tree(node->left, indent + 1);
    print_tree(node->right, indent + 1);
}

void free_parse_tree(node_t* node) {
    if (node == NULL) {
        return;
    }

    free_parse_tree(node->left);
    free_parse_tree(node->right);
    free(node);
}

void free_parser(parser_t* parser) {
    free_parse_tree(parser->parseTree);
    parser->parseTree = NULL;
}