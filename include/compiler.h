#ifndef compiler_h
#define compiler_h

#include "parser.h"

typedef enum {
    INS_OFFSET
} instr_type_t;

typedef enum {
    BIND_INT,
    BIND_BOOL
} bind_type_t;

typedef struct {
    char* data;
    instr_type_t type;
} instr_t;

typedef struct {
    wchar_t* id;
    bind_type_t type;
} id_t;

typedef struct {
    id_t* offsets;
    int cap;
    int size;
    int labels;
} context_t;

typedef struct {
    char* rodata;
    char* text;
    int labels;
} sections_t;

void compile(parser_t* parser, int _s);

#endif