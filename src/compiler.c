#include "../include/compiler.h"

void init_headers(sections_t* sections) {
    sections->rodata = (char*) calloc(20, sizeof(char));
    strcat(sections->rodata, ".section .rodata\n");
    sections->text = (char*) calloc(120, sizeof(char));
    strcat(sections->text, ".section .text\n.globl _start\n_start:\n\tcallq _entry\n\tmovq %rax, %rdi\n\tmovq $60, %rax\n\tsyscall\n.globl _entry\n_entry:\n");
}

void init_prologue(sections_t* sections, context_t* context) {
    sections->text = (char*) realloc(sections->text, (strlen(sections->text) + 30) * sizeof(char));
    strcat(sections->text, "\tpushq %rbp\n\tmovq %rsp, %rbp\n");

    if (context->size) {
        char* alloc = (char*) calloc(24, sizeof(char));
        sprintf(alloc, "\tsubq $%d, %%rsp\n", context->size * 8);
        sections->text = (char*) realloc(sections->text, (strlen(sections->text) + strlen(alloc) + 1) * sizeof(char));
        strcat(sections->text, alloc);
        free(alloc);
    }
}

void init_epilogue(sections_t* sections, context_t* context) {
    sections->text = (char*) realloc(sections->text, (strlen(sections->text) + 16) * sizeof(char));
    strcat(sections->text, "\tleaveq\n\tretq\n");
}

void free_sections(sections_t* sections) {
    free(sections->rodata);
    free(sections->text);
}

void init_context(context_t* context) {
    context->cap = 8;
    context->size = 0;
    context->offsets = (id_t*) calloc(context->cap, sizeof(id_t));
}

void add_offset(context_t* context, wchar_t* id) {
    context->offsets[context->size++].id = id;
    context->offsets[context->size++].type = -1;
    if (context->size >= context->cap) {
        context->cap *= 2;
        context->offsets = (id_t*) realloc(context->offsets, context->cap * sizeof(id_t));
    }
}

int get_offset(context_t* context, wchar_t* id) {
    for (int i = 0; i < context->size; i++) {
        if (!wcscmp(context->offsets[i].id, id)) {
            return (i + 1) * 8;
        }
    }
    return 0;
}

void set_bind_type(context_t* context, int offset, bind_type_t type) {
    context->offsets[offset / 8 - 1].type = type;
}

int tok_to_bind_type(tok_type_t type) {
    switch (type) {
        case TOK_INT:
        case TOK_PLUS:
        case TOK_MINUS:
        case TOK_MUL:
        case TOK_DIV:
        case TOK_MOD:
            return BIND_INT;
        case TOK_TRUE:
        case TOK_FALSE:
        case TOK_AND:
        case TOK_OR:
        case TOK_NOT:
        case TOK_EQ:
        case TOK_NE:
        case TOK_LT:
        case TOK_LTE:
        case TOK_GT:
        case TOK_GTE:
            return BIND_BOOL;
        default:
            return -1;
    }
}

void free_context(context_t* context) {
    free(context->offsets);
}

char* _compile_instr(node_t* ast, context_t* context, char* format);

char* _compile(node_t* ast, context_t* context) {
    switch (ast->tok->type) {
        case TOK_PRINT: {
            char* left = _compile(ast->left, context);
            char* instr = (char*) calloc(strlen(left) + 18, sizeof(char));
            sprintf(instr, "\tmovq %s, %%rax\n", left);
            return instr;
        }

        case TOK_ID: {
            int offset;
            if (!(offset = get_offset(context, ast->tok->data))) {
                add_offset(context, ast->tok->data);
                offset = get_offset(context, ast->tok->data);
            }
            char* expr = (char*) calloc(16, sizeof(char));
            sprintf(expr, "-%d(%%rbp)", offset);
            return expr;
        }

        case TOK_INT: {
            char* expr = (char*) calloc(ast->tok->len + 2, sizeof(char));
            strcat(expr, "$");
            wcstombs(&expr[1], ast->tok->data, ast->tok->len);
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
            char* left = _compile(ast->left, context);
            char* right = _compile(ast->right, context);
            char* instr = (char*) calloc(strlen(left) + strlen(right) + 20, sizeof(char));
            set_bind_type(context, atoi(left), tok_to_bind_type(ast->right->tok->type));
            sprintf(instr, "%s\tmovq %%rax, -%s(%%rbp)\n", right, left);
            free(left);
            free(right);
            return instr;
        }

        case TOK_PLUS: {
            return _compile_instr(ast, context, "\tmovq %s, %%rax\n\taddq %s, %%rax\n");
        }

        case TOK_MINUS: {
            return _compile_instr(ast, context, "\tmovq %s, %%rax\n\tsubq %s, %%rax\n");
        }

        case TOK_MUL: {
            return _compile_instr(ast, context, "\tmovq %s, %%rax\n\timulq %s, %%rax\n");
        }

        case TOK_DIV: {
            return _compile_instr(ast, context, "\tmovq %s, %%rax\n\tmovq %s, %%rbx\n\tmovq $0, %%rdx\n\tidivq %%rbx\n");
        }

        case TOK_MOD: {
            return _compile_instr(ast, context, "\tmovq %s, %%rax\n\tmovq %s, %%rbx\n\tmovq $0, %%rdx\n\tidivq %%rbx\n\tmovq %%rdx, %%rax\n");
        }

        case TOK_AND: {
            return _compile_instr(ast, context, "\tmovq %s, %%rax\n\tandq %s, %%rax\n");
        }

        case TOK_OR: {
            return _compile_instr(ast, context, "\tmovq %s, %%rax\n\torq %s, %%rax\n");
        }

        case TOK_NOT: {
            char* left = _compile(ast->left, context);
            char* instr = (char*) calloc(strlen(left) + 28, sizeof(char));
            sprintf(instr, "\tmovq %s, %%rax\n\tnegq %%rax\n", left);
            free(left);
            return instr;
        }

        case TOK_EQ: {
            return _compile_instr(ast, context, "\tmovq %s, %%rax\n\tcmpq %s, %%rax\n\tsete %%al\n\tmovzbq %%al, %%rax\n");
        }

        case TOK_NE: {
            return _compile_instr(ast, context, "\tmovq %s, %%rax\n\tcmpq %s, %%rax\n\tsetne %%al\n\tmovzbq %%al, %%rax\n");
        }

        case TOK_LT: {
            return _compile_instr(ast, context, "\tmovq %s, %%rax\n\tcmpq %s, %%rax\n\tsetl %%al\n\tmovzbq %%al, %%rax\n");
        }

        case TOK_LTE: {
            return _compile_instr(ast, context, "\tmovq %s, %%rax\n\tcmpq %s, %%rax\n\tsetle %%al\n\tmovzbq %%al, %%rax\n");
        }

        case TOK_GT: {
            return _compile_instr(ast, context, "\tmovq %s, %%rax\n\tcmpq %s, %%rax\n\tsetg %%al\n\tmovzbq %%al, %%rax\n");
        }

        case TOK_GTE: {
            return _compile_instr(ast, context, "\tmovq %s, %%rax\n\tcmpq %s, %%rax\n\tsetge %%al\n\tmovzbq %%al, %%rax\n");
        }

        default: {
            wprintf(L"Error: Undefined compilation for `%s`\n", tok_type_to_str(ast->tok->type));
            exit(1);
        }
    }
}

char* _compile_instr(node_t* ast, context_t* context, char* format) {
    char* left = _compile(ast->left, context);
    char* right = _compile(ast->right, context);
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
    init_headers(&sections);
    context_t context;
    init_context(&context);

    char* buf = (char*) calloc(1, sizeof(char));
    for (int i = 0; i < parser->size; i++) {
        char* instr = _compile(parser->astList[i], &context);
        buf = (char*) realloc(buf, (strlen(buf) + strlen(instr) + 1) * sizeof(char));
        strcat(buf, instr);
    }
    
    init_prologue(&sections, &context);

    sections.text = (char*) realloc(sections.text, (strlen(sections.text) + strlen(buf) + 1) * sizeof(char));
    strcat(sections.text, buf);
    
    init_epilogue(&sections, &context);

    write_asm("./a.s", &sections);
    
    system("as a.s -o a.o");
    system("ld a.o -o a.out");
    system("rm a.o");
    
    free_context(&context);
    free_sections(&sections);
}