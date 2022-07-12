#ifndef lexer_h
#define lexer_h

#include "util.h"

typedef enum {
    TOK_ID,
    TOK_KEYWORD,

    TOK_NUM,
    
    TOK_ASSIGN,
    TOK_PLUS,
    TOK_MINUS,
    TOK_MUL,
    TOK_DIV,

    TOK_NEWLINE,
    TOK_EOF
} tok_type_t;

typedef struct {
    wchar_t* data;
    int len;
    tok_type_t type;
    int val;
} token_t;

typedef struct {
    token_t* tokens;
    int cap;
    int size;
    int pos;
} lexer_t;

void init_lexer(lexer_t* lexer);
token_t* peek(lexer_t* lexer, int offset);
void lex(src_t* src, lexer_t* lexer);
void free_lexer(lexer_t* lexer);

#endif