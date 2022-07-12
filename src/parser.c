#include "../include/parser.h"

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

void init_vars(vars_t* vars) {
    vars->cap = 64;
    vars->size = 0;

    vars->list = (var_t*) calloc(vars->cap, sizeof(var_t));
    if (vars->list == NULL) {
        wprintf(L"Failed to initialize variables list\n");
        exit(1);
    }
}

int is_var(vars_t* vars, wchar_t* id) {
    int i;
    for (i = 0; i < vars->size; i++) {
        if (wcscmp(vars->list[i].id, id) == 0) {
            return 1;
        }
    }

    return 0;
}

void add_var(vars_t* vars, wchar_t* id, token_t* val) {
    if (vars->size >= vars->cap) {
        vars->cap *= 2;
        vars->list = (var_t*) realloc(vars->list, vars->cap * sizeof(var_t));
        if (vars->list == NULL) {
            wprintf(L"Failed to resize variables list\n");
            exit(1);
        }
    }

    vars->list[vars->size].id = id;
    vars->list[vars->size++].val = val;
}

void update_var(vars_t* vars, wchar_t* id, token_t* val) {
    int i;
    for (i = 0; i < vars->size; i++) {
        if (wcscmp(vars->list[i].id, id) == 0) {
            vars->list[i].val = val;
        }
    }
}

token_t* get_var_val(vars_t* vars, wchar_t* id) {
    int i;
    for (i = 0; i < vars->size; i++) {
        if (wcscmp(vars->list[i].id, id) == 0) {
            return vars->list[i].val;
        }
    }

    return NULL;
}

void free_vars(vars_t* vars) {
    free(vars->list);
}

void init_parser(parser_t* parser) {
    parser->parseTree = NULL;
    parser->vars = (vars_t*) calloc(1, sizeof(vars_t));
    init_vars(parser->vars);
}

node_t* parse_primary_expr(lexer_t* lexer, vars_t* vars) {
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

node_t* parse_factor(lexer_t* lexer, vars_t* vars) {
    node_t* expr = parse_primary_expr(lexer, vars);
    
    while (lexer->tokens[lexer->pos].type == TOK_MUL || lexer->tokens[lexer->pos].type == TOK_DIV) {
        node_t* op_node = (node_t*) calloc(1, sizeof(node_t));
        op_node->tok = &lexer->tokens[lexer->pos++];
        op_node->left = expr;
        op_node->right = parse_primary_expr(lexer, vars);
        expr = op_node;
    }

    return expr;
}

node_t* parse_term(lexer_t* lexer, vars_t* vars) {
    node_t* expr = parse_factor(lexer, vars);
    
    while (lexer->tokens[lexer->pos].type == TOK_PLUS || lexer->tokens[lexer->pos].type == TOK_MINUS) {
        node_t* op_node = (node_t*) calloc(1, sizeof(node_t));
        op_node->tok = &lexer->tokens[lexer->pos++];
        op_node->left = expr;
        op_node->right = parse_factor(lexer, vars);
        expr = op_node;
    }

    return expr;
}

void parse(lexer_t* lexer, parser_t* parser) {
    if (lexer->tokens[lexer->pos].type == TOK_KEYWORD) {
        node_t* keyword_node = (node_t*) calloc(1, sizeof(node_t));
        keyword_node->tok = &lexer->tokens[lexer->pos++];
        keyword_node->left = parse_term(lexer, parser->vars);
        parser->parseTree = keyword_node;
        return;
    }

    node_t* expr = parse_term(lexer, parser->vars);
    
    if (lexer->tokens[lexer->pos].type == TOK_ASSIGN) {
        node_t* op_node = (node_t*) calloc(1, sizeof(node_t));
        op_node->tok = &lexer->tokens[lexer->pos++];
        op_node->left = expr;
        op_node->right = parse_term(lexer, parser->vars);
        expr = op_node;
    }

    parser->parseTree = expr;
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
    free_vars(parser->vars);
    free(parser->vars);
}