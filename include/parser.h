#ifndef parser_h
#define parser_h

#include "lexer.h"

typedef struct node_t {
    token_t* tok;
    struct node_t* left;
    struct node_t* right;
} node_t;

typedef struct {
    node_t* parseTree;
} parser_t;

void init_parser(parser_t* parser);
void parse(lexer_t* lexer, parser_t* parser);
void print_tree(node_t* node, int indent);
void free_parser(parser_t* parser);

#endif