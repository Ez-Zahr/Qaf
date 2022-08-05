#ifndef compiler_h
#define compiler_h

#include "parser.h"

typedef enum {
    BIND_INT,
    BIND_BOOL
} bind_type_t;

typedef struct {
    wchar_t* id;
    bind_type_t type;
} id_t;

typedef struct {
    id_t* offsets;
    int cap;
    int size;
} context_t;

typedef struct {
    char* text;
    char* rodata;
} sections_t;

void compile(parser_t* parser);

#endif