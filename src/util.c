#include "../include/util.h"

extern ERROR_STATUS err_status;

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

int endsWith(const char *str, const char *suffix) {
    if (!str || !suffix) {
        return 0;
    }

    size_t strLen = strlen(str);
    size_t sufLen = strlen(suffix);
    if (sufLen >= strLen) {
        return 0;
    }

    return strncmp(str + strLen - sufLen, suffix, sufLen) == 0;
}

src_t* init_src() {
    src_t* src = (src_t*) calloc(1, sizeof(src_t));
    src->buf = (wchar_t*) calloc(1, sizeof(wchar_t));
    src->size = 0;
    src->pos = 0;
    return src;
}

void read_src(char* filename, src_t* src) {
    FILE* input;

    if ((input = fopen(filename, "r")) == NULL) {
        wprintf(L"Could not open file %s\n", filename);
        err_status = ERR_SRC;
        return;
    }

    wint_t c;
    while ((c = fgetwc(input)) != WEOF) {
        src->buf[src->size++] = c;
        src->buf = (wchar_t*) realloc(src->buf, (src->size + 1) * sizeof(wchar_t));
    }
    src->buf[src->size] = L'\0';
    
    fclose(input);
}

void free_src(src_t* src) {
    free(src->buf);
    free(src);
}

sections_t* init_sections() {
    sections_t* sections = (sections_t*) calloc(1, sizeof(sections_t));

    wchar_t* rodata = L".section .rodata\n\tt: .string \"صح\"\n\tf: .string \"خطأ\"\n";
    sections->rodata = (wchar_t*) calloc(wcslen(rodata) + 1, sizeof(wchar_t));
    wcscat(sections->rodata, rodata);

    wchar_t* bss = L".section .bss\n";
    sections->bss = (wchar_t*) calloc(wcslen(bss) + 1, sizeof(wchar_t));
    wcscat(sections->bss, bss);

    wchar_t* text = L".section .text\n.globl _start\n_start:\n\tcallq _entry\n\tmovq %rax, %rdi\n\tmovq $60, %rax\n\tsyscall\n_entry:\n";
    sections->text = (wchar_t*) calloc(wcslen(text) + 1, sizeof(wchar_t));
    wcscat(sections->text, text);

    sections->funcs = (wchar_t*) calloc(1, sizeof(wchar_t));

    wchar_t* include = L".include \"./include/asm/lib.s\"\n";
    sections->include = (wchar_t*) calloc(wcslen(include) + 1, sizeof(wchar_t));
    wcscat(sections->include, include);

    return sections;
}

void write_asm(sections_t* sections, char* filename) {
    FILE* output;
    if ((output = fopen(filename, "w")) == NULL) {
        wprintf(L"Could not open file %s\n", filename);
        err_status = ERR_ASM;
        return;
    }
    fputws(sections->rodata, output);
    fputws(sections->bss, output);
    fputws(sections->text, output);
    fputws(sections->funcs, output);
    fputws(sections->include, output);
    fclose(output);
}

void free_sections(sections_t* sections) {
    free(sections->rodata);
    free(sections->bss);
    free(sections->text);
    free(sections->funcs);
    free(sections->include);
    free(sections);
}