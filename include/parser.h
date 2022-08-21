#ifndef parser_h
#define parser_h

#include "lexer.h"

typedef enum {
    PRI_ASSIGN,
    PRI_LOGIC,
    PRI_CMP,
    PRI_TERM,
    PRI_FACTOR,
    PRI_MAX
} op_prior_t;

typedef struct ast_t {
    token_t* tok;
    struct ast_t* left;
    struct ast_t* right;
    struct ast_t** list;
    int cap;
    int size;
} ast_t;

void init_ast_list(ast_t* ast);
void parse(lexer_t* lexer, ast_t* root);
void print_ast_root(ast_t* root);
void free_ast(ast_t* ast);

#endif