#ifndef util_h
#define util_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>

typedef struct {
    wchar_t* buf;
    int cap;
    int size;
    int pos;
} src_t;

wchar_t* wcsrev(wchar_t* str);
void init_src(src_t* src);
void read_src(char* filename, src_t* src);

#endif