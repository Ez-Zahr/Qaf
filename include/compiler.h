#ifndef compiler_h
#define compiler_h

#include "parser.h"

typedef enum {
    BIND_INT,
    BIND_BOOL
} bind_type_t;

typedef enum {
    INS_OFFSET,
    INS_COMP
} instr_type_t;

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
    char* rodata;
    char* text;
} sections_t;

typedef struct {
    char* instr;
    instr_type_t type;
} instr_t;

void compile(parser_t* parser);

#endif