#include "../include/parser.h"

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
            node_t* expr = (node_t*) calloc(1, sizeof(node_t));
            expr->tok = &lexer->tokens[lexer->pos++];
            expr->left = parse_primary_expr(lexer);
            return expr;
        }

        case TOK_PRINT: {
            node_t* expr = (node_t*) calloc(1, sizeof(node_t));
            expr->tok = &lexer->tokens[lexer->pos++];
            expr->left = _parse(lexer, 1);
            return expr;
        }

        default: {
            token_t tok = lexer->tokens[lexer->pos];
            wprintf(L"Error: Invalid token `%ls` of type `%d`\n", tok.data, tok_type_to_str(tok.type));
            exit(1);
        }
    }
}

node_t* _parse(lexer_t* lexer, int prior) {
    node_t* expr = (prior == PRI_MAX - 1)? parse_primary_expr(lexer) : _parse(lexer, prior + 1);
    
    while (get_op_priority(lexer->tokens[lexer->pos].type) == prior) {
        node_t* op_node = (node_t*) calloc(1, sizeof(node_t));
        op_node->tok = &lexer->tokens[lexer->pos++];
        op_node->left = expr;
        op_node->right = (prior == PRI_MAX - 1)? parse_primary_expr(lexer) : _parse(lexer, prior + 1);
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
        
        if (parser->size >= parser->cap) {
            parser->cap *= 2;
            parser->astList = (node_t**) realloc(parser->astList, parser->cap * sizeof(node_t*));
        }
    }
}

void print_tree(node_t* node, int indent) {
    if (node == NULL) {
        return;
    }

    wprintf(L"%d: ", indent);
    for (int i = 0; i < indent; i++) {
        wprintf(L" ");
    }
    
    wprintf(L"`%ls`\n", node->tok->data);
    print_tree(node->left, indent + 1);
    print_tree(node->right, indent + 1);
}

void print_parser(parser_t* parser) {
    for (int i = 0; i < parser->size; i++) {
        print_tree(parser->astList[i], 0);
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
    for (int i = 0; i < parser->size; i++) {
        free_parse_tree(parser->astList[i]);
    }
    free(parser->astList);
}