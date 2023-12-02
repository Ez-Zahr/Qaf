#include "../include/util.h"

#if defined(_WIN32) || defined(_WIN64)
    const char* OS = "Windows";
#elif defined(__linux)
    const char* OS = "Linux";
#else
    const char* OS = "Unknown";
#endif

static allocs_t* allocs = 0;

void set_locale() {
    char* locale = NULL;

    if (!strcmp(OS, "Windows")) {
        locale = setlocale(LC_ALL, "Arabic_Saudi Arabia.1256");
    } else if (!strcmp(OS, "Linux")) {
        locale = setlocale(LC_ALL, "ar_SA.utf8");
    }

    if (locale == NULL) {
        wprintf(L"Failed to set locale\n");
        smart_exit(ERR_LOCALE);
    }
}

args_t* init_args() {
    args_t* args = (args_t*) smart_alloc(1, sizeof(args_t));
    args->filename = 0;
    args->_t = 0;
    args->_a = 0;
    args->_s = 0;
    return args;
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

void read_args(int argc, char* argv[], args_t* args) {
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
                case 't': args->_t = 1; break;
                case 'a': args->_a = 1; break;
                case 's': args->_s = 1; break;
                default: wprintf(L"Error: Unknown option `%s`\n", argv[i]); smart_exit(ERR_ARGS);
            }
        } else if (endsWith(argv[i], ".qaf") && !args->filename) {
            args->filename = argv[i];
        } else {
            wprintf(L"Error: Unknown argument '%s'\n", argv[i]);
            smart_exit(ERR_ARGS);
        }
    }

    if (!args->filename) {
        wprintf(L"Please specify a .qaf input file (./qaf <filename>.qaf)\n");
        smart_exit(ERR_ARGS);
    }
}

void init_allocs() {
    if (allocs) return;
    allocs = (allocs_t*) calloc(1, sizeof(allocs_t));
    allocs->list = (void**) calloc(1, sizeof(void*));
    allocs->size = 0;
}

void* smart_alloc(int n, int size) {
    allocs->list[allocs->size++] = (void*) calloc(n, size);
    allocs->list = (void**) realloc(allocs->list, (allocs->size + 1) * sizeof(void*));
    return allocs->list[allocs->size - 1];
}

void* smart_realloc(void* ptr, int n, int size) {
    for (int i = 0; i < allocs->size; i++) {
        if (ptr == allocs->list[i]) {
            allocs->list[i] = (void*) realloc(ptr, n * size);
            return allocs->list[i];
        }
    }
    return 0;
}

void free_allocs() {
    for (int i = 0; i < allocs->size; i++) {
        free(allocs->list[i]);
    }
    free(allocs->list);
    free(allocs);
}

void smart_exit(ERROR_STATUS status) {
    free_allocs();
    exit(status);
}

src_t* init_src() {
    src_t* src = (src_t*) smart_alloc(1, sizeof(src_t));
    src->buf = NULL;
    src->size = 0;
    src->pos = 0;
    return src;
}

void read_src(char* filename, src_t* src) {
    FILE* input;
    
    if ((input = fopen(filename, "r")) == NULL) {
        wprintf(L"Could not open file %s\n", filename);
        smart_exit(ERR_SRC);
    }

    fseek(input, 0L, SEEK_END);
    long sz = ftell(input);
    rewind(input);
    char bf[sz];
    fread(bf, sizeof(char), sz, input);
    fclose(input);

    for (int i = sz - 1; i >= 0; i--) {
        if ((bf[i] == 0xffffff9b || bf[i] == 0x7d) && (i + 1) < sz) {
            bf[i + 1] = '\0';
            break;
        }
    }

    src->size = MultiByteToWideChar(CP_UTF8, 0, bf, -1, NULL, 0);
    src->buf = (wchar_t*) smart_alloc(src->size + 1, sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, bf, -1, src->buf, src->size);
}

sections_t* init_sections() {
    sections_t* sections = (sections_t*) smart_alloc(1, sizeof(sections_t));

    wchar_t* rodata = L".section .rodata\n\tt: .string \"صح\"\n\tf: .string \"خطأ\"\n";
    sections->rodata = (wchar_t*) smart_alloc(wcslen(rodata) + 1, sizeof(wchar_t));
    wcscat(sections->rodata, rodata);

    wchar_t* bss = L".section .bss\n";
    sections->bss = (wchar_t*) smart_alloc(wcslen(bss) + 1, sizeof(wchar_t));
    wcscat(sections->bss, bss);

    wchar_t* text = L".section .text\n.globl _start\n_start:\n\tcallq _entry\n\tmovq $0, %rdi\n\tmovq $60, %rax\n\tsyscall\n_entry:\n";
    sections->text = (wchar_t*) smart_alloc(wcslen(text) + 1, sizeof(wchar_t));
    wcscat(sections->text, text);

    sections->funcs = (wchar_t*) smart_alloc(1, sizeof(wchar_t));

    wchar_t* include = L".include \"./include/asm/lib.s\"\n";
    sections->include = (wchar_t*) smart_alloc(wcslen(include) + 1, sizeof(wchar_t));
    wcscat(sections->include, include);

    return sections;
}

void write_asm(sections_t* sections, char* filename) {
    FILE* output;
    if ((output = fopen(filename, "w")) == NULL) {
        wprintf(L"Could not open file %s\n", filename);
        smart_exit(ERR_ASM);
    }
    fputws(sections->rodata, output);
    fputws(sections->bss, output);
    fputws(sections->text, output);
    fputws(sections->funcs, output);
    fputws(sections->include, output);
    fclose(output);
}