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
    vars_t* vars;
} parser_t;

void print_tree(node_t* node, int indent);

int is_var(vars_t* vars, wchar_t* id);
void add_var(vars_t* vars, wchar_t* id, token_t* val);
void update_var(vars_t* vars, wchar_t* id, token_t* val);
token_t* get_var_val(vars_t* vars, wchar_t* id);

void init_parser(parser_t* parser);
void parse(lexer_t* lexer, parser_t* parser);
void free_parser(parser_t* parser);

#endif