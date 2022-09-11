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
    ERR_MAIN,
    ERR_SRC,
    ERR_LEX,
    ERR_PARSE,
    ERR_COMPILE,
    ERR_ASM,
    ERR_EVAL
} ERROR_STATUS;

typedef struct allocs_t {
    void** list;
    int size;
} allocs_t;

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

void init_allocs();
void* smart_alloc(int n, int size);
void* smart_realloc(void* ptr, int n, int size);
void free_allocs();
void smart_exit(ERROR_STATUS status);

src_t* init_src();
void read_src(char* filename, src_t* src);

sections_t* init_sections();
void write_asm(sections_t* sections, char* filename);

#endif