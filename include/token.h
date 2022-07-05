#ifndef token_h
#define token_h

#include "util.h"

typedef enum {
    TOK_ID,
    TOK_NUM,
    TOK_OP
} TOK_TYPE;

typedef struct {
    wchar_t* data;
    TOK_TYPE type;
} token_t;

#endif