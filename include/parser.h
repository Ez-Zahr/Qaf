#ifndef parser_h
#define parser_h

#include "lexer.h"

typedef struct node_t {
    token_t* tok;
    struct node_t* left;
    struct node_t* right;
} node_t;

typedef struct {
    node_t** parseTrees;
    int cap;
    int size;
} parser_t;

void init_parser(parser_t* parser);
node_t* parse(lexer_t* lexer);
void print_parser(parser_t* parser);
void free_parser(parser_t* parser);

#endif