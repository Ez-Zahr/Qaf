#include "../include/parser.h"

void init_parser(parser_t* parser) {
    parser->cap = 32;
    parser->size = 0;
    parser->parseTrees = (node_t**) calloc(parser->cap, sizeof(node_t*));
}

int get_op_priority(tok_type_t op) {
    switch (op) {
        case TOK_ASSIGN:
            return 0;
        
        case TOK_PLUS:
        case TOK_MINUS:
        case TOK_AND:
        case TOK_OR:
            return 1;
        
        case TOK_MUL:
        case TOK_DIV:
            return 2;
        
        case TOK_NOT:
            return 3;
        
        default:
            return -1;
    }
}

node_t* parse_primary_expr(lexer_t* lexer) {
    switch (lexer->tokens[lexer->pos].type) {
        case TOK_TRUE:
        case TOK_FALSE:
        case TOK_FLOAT:
        case TOK_INT:
        case TOK_ID: {
            node_t* prim_node = (node_t*) calloc(1, sizeof(node_t));
            prim_node->tok = &lexer->tokens[lexer->pos++];
            return prim_node;
        }

        case TOK_NOT: {
            node_t* unary_expr = (node_t*) calloc(1, sizeof(node_t));
            unary_expr->tok = &lexer->tokens[lexer->pos++];
            unary_expr->left = parse_primary_expr(lexer);
            return unary_expr;
        }

        default: {
            wprintf(L"Error: Invalid token `%ls` of type `%d`\n", lexer->tokens[lexer->pos].data, lexer->tokens[lexer->pos].type);
            exit(1);
        }
    }
}

node_t* parse_factor(lexer_t* lexer) {
    node_t* expr = parse_primary_expr(lexer);
    
    while (get_op_priority(lexer->tokens[lexer->pos].type) == 2) {
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
    
    while (get_op_priority(lexer->tokens[lexer->pos].type) == 1) {
        node_t* op_node = (node_t*) calloc(1, sizeof(node_t));
        op_node->tok = &lexer->tokens[lexer->pos++];
        op_node->left = expr;
        op_node->right = parse_factor(lexer);
        expr = op_node;
    }

    return expr;
}

node_t* parse(lexer_t* lexer) {
    node_t* expr;

    if (lexer->tokens[lexer->pos].type == TOK_PRINT) {
        expr = (node_t*) calloc(1, sizeof(node_t));
        expr->tok = &lexer->tokens[lexer->pos++];
        expr->left = parse_term(lexer);
        return expr;
    }

    expr = parse_term(lexer);
    
    if (lexer->tokens[lexer->pos].type == TOK_ASSIGN) {
        node_t* op_node = (node_t*) calloc(1, sizeof(node_t));
        op_node->tok = &lexer->tokens[lexer->pos++];
        op_node->left = expr;
        op_node->right = parse_term(lexer);
        expr = op_node;
    }

    return expr;
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

void print_parser(parser_t* parser) {
    int i;
    for (i = 0; i < parser->size; i++) {
        print_tree(parser->parseTrees[i], 0);
    }
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
    int i;
    for (i = 0; i < parser->size; i++) {
        free_parse_tree(parser->parseTrees[i]);
    }
    free(parser->parseTrees);
}