#include "../include/parser.h"

void print_tree(node_t* node, int indent) {
    if (node == NULL) {
        return;
    }

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
        printf("Failed to initialize variables list\n");
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
            printf("Failed to resize variables list\n");
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
}

void parse(lexer_t* lexer, parser_t* parser, vars_t* vars) {
    parser->parseTree = (node_t*) calloc(1, sizeof(node_t));
    parser->parseTree->left = parse_term(lexer, vars);
    
    if (wcscmp(lexer->tokens[lexer->pos].data, L"=") == 0) {
        parser->parseTree->tok = &lexer->tokens[lexer->pos++];
        parser->parseTree->right = parse_term(lexer, vars);
    }
}

node_t* parse_term(lexer_t* lexer, vars_t* vars) {
    node_t* left = parse_factor(lexer, vars);
    
    while (wcscmp(lexer->tokens[lexer->pos].data, L"+") == 0 || wcscmp(lexer->tokens[lexer->pos].data, L"-") == 0) {
        node_t* op_node = (node_t*) calloc(1, sizeof(node_t));
        op_node->tok = &lexer->tokens[lexer->pos++];
        node_t* right = parse_factor(lexer, vars);
        op_node->left = left;
        op_node->right = right;
        left = op_node;
    }

    return left;
}

node_t* parse_factor(lexer_t* lexer, vars_t* vars) {
    node_t* left = parse_primary_expr(lexer, vars);
    
    while (wcscmp(lexer->tokens[lexer->pos].data, L"*") == 0 || wcscmp(lexer->tokens[lexer->pos].data, L"/") == 0) {
        node_t* op_node = (node_t*) calloc(1, sizeof(node_t));
        op_node->tok = &lexer->tokens[lexer->pos++];
        node_t* right = parse_primary_expr(lexer, vars);
        op_node->left = left;
        op_node->right = right;
        left = op_node;
    }

    return left;
}

node_t* parse_primary_expr(lexer_t* lexer, vars_t* vars) {
    node_t* prim_node;
    token_t* next = peek(lexer, 1);
    switch (lexer->tokens[lexer->pos].type) {
        case TOK_NUM:
            prim_node = (node_t*) calloc(1, sizeof(node_t));
            prim_node->tok = &lexer->tokens[lexer->pos++];
            return prim_node;
        case TOK_ID:
            if (next != NULL) {
                if (wcscmp(next->data, L"=") == 0) {
                    prim_node = (node_t*) calloc(1, sizeof(node_t));
                    prim_node->tok = &lexer->tokens[lexer->pos++];
                    return prim_node;
                } else if (is_var(vars, lexer->tokens[lexer->pos].data)) {
                    prim_node = (node_t*) calloc(1, sizeof(node_t));
                    prim_node->tok = get_var_val(vars, lexer->tokens[lexer->pos++].data);
                    return prim_node;
                } else {
                    wprintf(L"Undefined variable `%ls`\n", lexer->tokens[lexer->pos].data);
                    exit(1);
                }
            } else {
                if (is_var(vars, lexer->tokens[lexer->pos].data)) {
                    prim_node = (node_t*) calloc(1, sizeof(node_t));
                    prim_node->tok = get_var_val(vars, lexer->tokens[lexer->pos++].data);
                    return prim_node;
                } else {
                    wprintf(L"Undefined variable `%ls`\n", lexer->tokens[lexer->pos].data);
                    exit(1);
                }
            }
            break;
        default:
            wprintf(L"Undefined token `%ls` of type `%d`\n", lexer->tokens[lexer->pos].data, lexer->tokens[lexer->pos].type);
            exit(1);
    }
}

void free_parser(parser_t* parser) {
    free_parse_tree(parser->parseTree);
}

void free_parse_tree(node_t* node) {
    if (node == NULL) {
        return;
    }

    free_parse_tree(node->left);
    free_parse_tree(node->right);
    free(node);
}