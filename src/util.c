#include "../include/util.h"

wchar_t* wcsrev(wchar_t* str) {
    wchar_t *p1, *p2;
    if (!str || !*str)
        return str;
    for (p1 = str, p2 = str + wcslen(str) - 1; p2 > p1; ++p1, --p2){
        *p1 ^= *p2;
        *p2 ^= *p1;
        *p1 ^= *p2;
    }
    return str;
}

int isaralpha(wchar_t c) {
    return (0x600 <= c && c <= 0x6ff);
}

int isop(wchar_t c) {
    return (c == L'=' || c == L'+' || c == L'-' || c == L'*' || c == L'/');
}

void init_src(src_t* src, char* filename) {
    src->filename = filename;
    src->cap = 1024;
    src->size = 0;
    src->pos = 0;
    
    src->buf = (wchar_t*) calloc(src->cap, sizeof(wchar_t));
    if (src->buf == NULL) {
        printf("Failed to initialize source buffer\n");
        exit(1);
    }
}

void read_src(char* filename, src_t* src) {
    FILE* input;

    if ((input = fopen(filename, "r")) == NULL) {
        printf("Could not open file %s\n", filename);
        exit(1);
    }

    wint_t c;
    while ((c = fgetwc(input)) != WEOF) {
        if (src->size >= src->cap) {
            src->cap *= 2;
            src->buf = (wchar_t*) realloc(src->buf, src->cap * sizeof(wchar_t));
            if (src->buf == NULL) {
                printf("Failed to resize source buffer\n");
                exit(1);
            }
        }
        src->buf[src->size++] = c;
    }
    src->buf[src->size] = L'\0';
    
    fclose(input);
}

void free_src(src_t* src) {
    free(src->buf);
}