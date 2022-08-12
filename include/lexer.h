#ifndef lexer_h
#define lexer_h

#include "util.h"

typedef enum {
    TOK_PRINT,
    TOK_ID,

    TOK_INT,
    TOK_FLOAT,
    TOK_BOOL,
    
    TOK_ASSIGN,
    TOK_PLUS,
    TOK_MINUS,
    TOK_MUL,
    TOK_DIV,
    TOK_MOD,

    TOK_AND,
    TOK_OR,
    TOK_NOT,

    TOK_EQ,
    TOK_NE,
    TOK_LT,
    TOK_LTE,
    TOK_GT,
    TOK_GTE,

    TOK_IF,
    TOK_FOR,
    TOK_WHILE,
    TOK_IN,

    TOK_LPAREN,
    TOK_RPAREN,
    TOK_LBRACE,
    TOK_RBRACE,
    TOK_LBRACK,
    TOK_RBRACK,
    TOK_COLON,
    TOK_SEMI,
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
wchar_t* tok_type_to_str(tok_type_t type);
void print_tokens(lexer_t* lexer);
void free_lexer(lexer_t* lexer);

#endif