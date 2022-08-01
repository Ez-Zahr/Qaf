#include "../include/compiler.h"

void init_rodata(sections_t* sections) {
    sections->rodata = (char*) calloc(28, sizeof(char));
    strcat(sections->rodata, ".section .rodata\n");
}

void init_text_prologue(sections_t* sections, context_t* context) {
    char* text = ".section .text\n.globl _start\n_start:\n\tcall entry\n\tmov %rax, %rdi\n\tmov $60, %rax\n\tsyscall"
    "\n.globl entry\nentry:\n\tpush %rbp\n\tmov %rsp, %rbp\n";
    sections->text = (char*) calloc(strlen(text) + 1, sizeof(char));
    strcat(sections->text, text);

    if (context->size) {
        char* alloc = (char*) calloc(24, sizeof(char));
        sprintf(alloc, "\tsub $%d, %%rsp\n", context->size * 8);
        sections->text = (char*) realloc(sections->text, (strlen(sections->text) + strlen(alloc) + 1) * sizeof(char));
        strcat(sections->text, alloc);
        free(alloc);
    }
}

void init_text_epilogue(sections_t* sections, context_t* context) {
    char* text = "\tleave\n\tret\n";
    sections->text = (char*) realloc(sections->text, (strlen(sections->text) + 16) * sizeof(char));
    strcat(sections->text, text);
}

void free_sections(sections_t* sections) {
    free(sections->rodata);
    free(sections->text);
}

void init_context(context_t* context) {
    context->cap = 8;
    context->size = 0;
    context->offsets = (wchar_t**) calloc(context->cap, sizeof(wchar_t*));
}

void add_offset(context_t* context, wchar_t* id) {
    context->offsets[context->size++] = id;
    if (context->size >= context->cap) {
        context->cap *= 2;
        context->offsets = (wchar_t**) realloc(context->offsets, context->cap * sizeof(wchar_t*));
    }
}

int get_offset(context_t* context, wchar_t* id) {
    for (int i = 0; i < context->size; i++) {
        if (!wcscmp(context->offsets[i], id)) {
            return (i + 1) * 8;
        }
    }
    return 0;
}

void free_context(context_t* context) {
    free(context->offsets);
}

char* _compile_instr(node_t* parseTree, context_t* context, char* format);

char* _compile(node_t* parseTree, context_t* context) {
    switch (parseTree->tok->type) {
        case TOK_PRINT: {
            char* left = _compile(parseTree->left, context);
            // char* format = "\tmov $1, %%rax\n\tmov $1, %%rdi\n\tmov %s, %%rsi\n\tmov %s, %%rdx\n\tsyscall\n";
            char* instr = (char*) calloc(strlen(left) + 18, sizeof(char));
            sprintf(instr, "\tmov %s, %%rax\n", left);
            return instr;
        }

        case TOK_ID: {
            int offset;
            if (!(offset = get_offset(context, parseTree->tok->data))) {
                add_offset(context, parseTree->tok->data);
                offset = get_offset(context, parseTree->tok->data);
            }
            char* expr = (char*) calloc(16, sizeof(char));
            sprintf(expr, "-%d(%%rbp)", offset);
            return expr;
        }

        case TOK_INT: {
            char* expr = (char*) calloc(parseTree->tok->len + 2, sizeof(char));
            strcat(expr, "$");
            wcstombs(&expr[1], parseTree->tok->data, parseTree->tok->len);
            return expr;
        }

        case TOK_TRUE: {
            char* expr = (char*) calloc(3, sizeof(char));
            strcpy(expr, "$1");
            return expr;
        }

        case TOK_FALSE: {
            char* expr = (char*) calloc(3, sizeof(char));
            strcpy(expr, "$0");
            return expr;
        }

        case TOK_ASSIGN: {
            char* left = _compile(parseTree->left, context);
            char* right = _compile(parseTree->right, context);
            char* instr = (char*) calloc(strlen(left) + strlen(right) + 18, sizeof(char));
            sprintf(instr, "%s\tmov %%rax, %s\n", right, left);
            free(left);
            free(right);
            return instr;
        }

        case TOK_PLUS: {
            return _compile_instr(parseTree, context, "\tmov %s, %%rax\n\tadd %s, %%rax\n");
        }

        case TOK_MINUS: {
            return _compile_instr(parseTree, context, "\tmov %s, %%rax\n\tsub %s, %%rax\n");
        }

        case TOK_MUL: {
            return _compile_instr(parseTree, context, "\tmov %s, %%rax\n\timul %s, %%rax\n");
        }

        case TOK_DIV: {
            break;
        }

        case TOK_AND: {
            return _compile_instr(parseTree, context, "\tmov %s, %%rax\n\tand %s, %%rax\n");
        }

        case TOK_OR: {
            return _compile_instr(parseTree, context, "\tmov %s, %%rax\n\tor %s, %%rax\n");
        }

        case TOK_NOT: {
            char* left = _compile(parseTree->left, context);
            char* instr = (char*) calloc(strlen(left) + 28, sizeof(char));
            sprintf(instr, "\tmov %s, %%rax\n\tneg %%rax\n", left);
            free(left);
            return instr;
        }

        case TOK_EQ: {
            return _compile_instr(parseTree, context, "\tmov %s, %%rax\n\tcmp %s, %%rax\n\tsete %%al\n\tmovzbq %%al, %%rax\n");
        }

        case TOK_NE: {
            return _compile_instr(parseTree, context, "\tmov %s, %%rax\n\tcmp %s, %%rax\n\tsetne %%al\n\tmovzbq %%al, %%rax\n");
        }

        case TOK_LT: {
            return _compile_instr(parseTree, context, "\tmov %s, %%rax\n\tcmp %s, %%rax\n\tsetl %%al\n\tmovzbq %%al, %%rax\n");
        }

        case TOK_LTE: {
            return _compile_instr(parseTree, context, "\tmov %s, %%rax\n\tcmp %s, %%rax\n\tsetle %%al\n\tmovzbq %%al, %%rax\n");
        }

        case TOK_GT: {
            return _compile_instr(parseTree, context, "\tmov %s, %%rax\n\tcmp %s, %%rax\n\tsetg %%al\n\tmovzbq %%al, %%rax\n");
        }

        case TOK_GTE: {
            return _compile_instr(parseTree, context, "\tmov %s, %%rax\n\tcmp %s, %%rax\n\tsetge %%al\n\tmovzbq %%al, %%rax\n");
        }

        default: {
            wprintf(L"Error: Undefined compilation for `%s`\n", tok_type_to_str(parseTree->tok->type));
            exit(1);
        }
    }
}

char* _compile_instr(node_t* parseTree, context_t* context, char* format) {
    char* left = _compile(parseTree->left, context);
    char* right = _compile(parseTree->right, context);
    char* instr = (char*) calloc(strlen(left) + strlen(right) + strlen(format) + 1, sizeof(char));
    sprintf(instr, format, left, right);
    free(left);
    free(right);
    return instr;
}

void write_asm(char* filename, sections_t* sections) {
    FILE* output;

    if ((output = fopen(filename, "w")) == NULL) {
        wprintf(L"Could not open file %s\n", filename);
        exit(1);
    }

    fputs(sections->rodata, output);
    fputs(sections->text, output);
    fclose(output);
}

void compile(parser_t* parser) {
    sections_t sections;
    init_rodata(&sections);
    context_t context;
    init_context(&context);

    char* buf = (char*) calloc(1, sizeof(char));
    for (int i = 0; i < parser->size; i++) {
        char* instr = _compile(parser->parseTrees[i], &context);
        buf = (char*) realloc(buf, (strlen(buf) + strlen(instr) + 1) * sizeof(char));
        strcat(buf, instr);
    }
    
    init_text_prologue(&sections, &context);

    sections.text = (char*) realloc(sections.text, (strlen(sections.text) + strlen(buf) + 1) * sizeof(char));
    strcat(sections.text, buf);
    
    init_text_epilogue(&sections, &context);

    write_asm("./a.s", &sections);
    
    system("as a.s -o a.o");
    system("ld a.o -o a.out");
    system("rm a.o");
    
    free_context(&context);
    free_sections(&sections);
}