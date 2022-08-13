#ifndef compiler_h
#define compiler_h

#include "parser.h"

typedef enum {
    INS_OFFSET
} instr_type_t;

typedef enum {
    BIND_INT,
    BIND_BOOL,
    BIND_CHAR,
    BIND_STR
} bind_type_t;

typedef struct {
    wchar_t* data;
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
    int strings;
} context_t;

typedef struct {
    wchar_t* rodata;
    wchar_t* text;
    int labels;
} sections_t;

void compile(parser_t* parser, int _s);

#endif