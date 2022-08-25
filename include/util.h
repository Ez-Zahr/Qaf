#ifndef util_h
#define util_h

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>

typedef enum ERROR_STATUS {
    ERR_NONE,
    ERR_SRC,
    ERR_LEX,
    ERR_PARSE,
    ERR_COMPILE,
    ERR_ASM,
    ERR_EVAL
} ERROR_STATUS;

ERROR_STATUS err_status;

typedef struct src_t {
    char* filename;
    wchar_t* buf;
    int size;
    int pos;
} src_t;

typedef struct sections_t {
    wchar_t* rodata;
    wchar_t* bss;
    wchar_t* text;
    wchar_t* funcs;
    wchar_t* include;
    int labels;
} sections_t;

wchar_t* wcsrev(wchar_t* str);
int endsWith(const char *str, const char *suffix);

src_t* init_src();
void read_src(char* filename, src_t* src);
void free_src(src_t* src);

sections_t* init_sections();
void write_asm(sections_t* sections, char* filename);
void free_sections(sections_t* sections);

#endif