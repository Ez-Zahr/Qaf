#ifndef parser_h
#define parser_h

#include "lexer.h"
#include "scope.h"

typedef enum op_prior_t {
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
    int size;
    scope_t* scope;
} ast_t;

ast_t* init_ast_list();
void parse(lexer_t* lexer, ast_t* root);
void print_ast_list(ast_t* ast);
void free_ast_list(ast_t* ast);

#endif