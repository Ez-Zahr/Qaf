#ifndef parser_h
#define parser_h

#include "lexer.h"

typedef struct {
    token_t* tok;
    node_t* left;
    node_t* right;
} node_t;

typedef struct {
    node_t* parseTree;
} parser_t;

void init_parser(parser_t* parser);
void parse(lexer_t* lexer, parser_t* parser);
void parse_term(lexer_t* lexer, parser_t* parser);
void parse_factor(lexer_t* lexer, parser_t* parser);
void parse_primary_expr(lexer_t* lexer, parser_t* parser);

#endif