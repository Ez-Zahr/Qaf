#ifndef parser_h
#define parser_h

#include "lexer.h"

typedef enum {
    PRI_ASSIGN,
    PRI_AND_OR,
    PRI_CMP,
    PRI_ADD_SUB,
    PRI_MUL_DIV,
    PRI_MAX
} op_prior_t;

typedef struct node_t {
    token_t* tok;
    struct node_t* left;
    struct node_t* right;
    struct node_t** astList;
    int cap;
    int size;
} node_t;

typedef struct {
    node_t** astList;
    int cap;
    int size;
} parser_t;

void init_parser(parser_t* parser);
void parse(lexer_t* lexer, parser_t* parser);
void print_parser(parser_t* parser);
void free_parser(parser_t* parser);

#endif