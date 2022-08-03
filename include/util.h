#ifndef util_h
#define util_h

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>

typedef struct {
    char* filename;
    wchar_t* buf;
    int cap;
    int size;
    int pos;
} src_t;

wchar_t* wcsrev(wchar_t* str);
int endsWith(const char *str, const char *suffix);

void init_src(src_t* src);
void read_src(char* filename, src_t* src);
void free_src(src_t* src);

#endif