#ifndef compiler_h
#define compiler_h

#include "parser.h"

typedef struct {
    char* text;
    char* rodata;
} sections_t;

typedef struct {
    wchar_t** offsets;
    int cap;
    int size;
} context_t;

void compile(parser_t* parser);

#endif