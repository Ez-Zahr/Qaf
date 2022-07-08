#ifndef lexer_h
#define lexer_h

#include "util.h"

typedef enum {
    TOK_ID,
    TOK_NUM,
    TOK_OP
} tok_type_t;

typedef struct {
    wchar_t* data;
    tok_type_t type;
    int len;
} token_t;

typedef struct {
    token_t* tokens;
    int cap;
    int size;
    int pos;
} lexer_t;

void init_lexer(lexer_t* lexer);
token_t* peek(lexer_t* lexer, int offset);
void add_tok(lexer_t* lexer, src_t* src, tok_type_t type);
void lex(src_t* src, lexer_t* lexer);
void free_lexer(lexer_t* lexer);

#endif