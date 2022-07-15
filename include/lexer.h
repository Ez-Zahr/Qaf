#ifndef lexer_h
#define lexer_h

#include "util.h"

typedef enum {
    TOK_ID,

    TOK_PRINT,

    TOK_NUM,
    TOK_TRUE,
    TOK_FALSE,
    
    TOK_ASSIGN,
    TOK_PLUS,
    TOK_MINUS,
    TOK_MUL,
    TOK_DIV,

    TOK_OR,
    TOK_AND,
    TOK_NOT,

    TOK_NEWLINE,
    TOK_EOF
} tok_type_t;

typedef struct {
    wchar_t* data;
    int len;
    tok_type_t type;
} token_t;

typedef struct {
    token_t* tokens;
    int cap;
    int size;
    int pos;
} lexer_t;

void init_lexer(lexer_t* lexer);
void lex(src_t* src, lexer_t* lexer);
void print_tokens(lexer_t* lexer);
void free_lexer(lexer_t* lexer);

#endif