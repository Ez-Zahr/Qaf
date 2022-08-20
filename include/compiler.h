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
    wchar_t** arr;
    bind_type_t* types;
    int cap;
    int size;
} _offsets_list_t;

typedef struct {
    _offsets_list_t* offsets;
    int labels;
} context_t;

typedef struct {
    wchar_t* rodata;
    wchar_t* bss;
    wchar_t* text;
    int labels;
} sections_t;

void compile(ast_t* root, int _s);

#endif