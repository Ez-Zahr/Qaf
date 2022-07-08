#ifndef parser_h
#define parser_h

#include "lexer.h"

typedef struct {
    wchar_t* id;
    token_t* val;
} var_t;

typedef struct {
    var_t* list;
    int cap;
    int size;
} vars_t;

typedef struct node_t {
    token_t* tok;
    struct node_t* left;
    struct node_t* right;
} node_t;

typedef struct {
    node_t* parseTree;
} parser_t;

void print_tree(node_t* node, int indent);

void init_vars(vars_t* vars);
int is_var(vars_t* vars, wchar_t* id);
void add_var(vars_t* vars, wchar_t* id, token_t* val);
void update_var(vars_t* vars, wchar_t* id, token_t* val);
token_t* get_var_val(vars_t* vars, wchar_t* id);
void free_vars(vars_t* vars);

void init_parser(parser_t* parser);
void parse(lexer_t* lexer, parser_t* parser, vars_t* vars);
node_t* parse_term(lexer_t* lexer, vars_t* vars);
node_t* parse_factor(lexer_t* lexer, vars_t* vars);
node_t* parse_primary_expr(lexer_t* lexer, vars_t* vars);
void free_parser(parser_t* parser);
void free_parse_tree(node_t* node);

#endif