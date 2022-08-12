#include "../include/compiler.h"

void init_headers(sections_t* sections) {
    char* rodata = ".section .rodata\n";
    sections->rodata = (char*) calloc(strlen(rodata) + 1, sizeof(char));
    strcat(sections->rodata, rodata);

    char* text = ".section .text\n.globl _start\n_start:\n\tcallq _entry\n\tmovq %rax, %rdi\n\tmovq $60, %rax\n\tsyscall\n.globl _entry\n_entry:\n";
    sections->text = (char*) calloc(strlen(text) + 1, sizeof(char));
    strcat(sections->text, text);
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
    sections->text = (char*) realloc(sections->text, (strlen(sections->text) + 32) * sizeof(char));
    strcat(sections->text, "\tmovq $0, %rax\n\tleaveq\n\tretq\n");
}

void free_sections(sections_t* sections) {
    free(sections->rodata);
    free(sections->text);
}

void init_context(context_t* context) {
    context->labels = 0;
    context->cap = 8;
    context->size = 0;
    context->offsets = (id_t*) calloc(context->cap, sizeof(id_t));
}

void add_offset(context_t* context, wchar_t* id) {
    context->offsets[context->size].id = id;
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

int extract_offset(char* offset) {
    int len = strchr(offset, '(') - offset;
    char sub[len];
    strncpy(sub, &offset[1], len - 1);
    return atoi(sub);
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
        case TOK_BOOL:
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

char* get_label(context_t* context) {
    char* label = (char*) calloc(8, sizeof(char));
    sprintf(label, ".L%d", context->labels++);
    return label;
}

void free_context(context_t* context) {
    free(context->offsets);
}

void free_instr(instr_t* instr) {
    free(instr->data);
    free(instr);
}

instr_t* _process_offset(context_t* context, instr_t* instr) {
    switch (instr->type) {
        case INS_OFFSET: {
            id_t id = context->offsets[extract_offset(instr->data) / 8 - 1];
            if (id.type == -1) {
                wprintf(L"Error: Undefined variable `%ls`\n", id.id);
                exit(1);
            }
            instr_t* new_instr = (instr_t*) calloc(1, sizeof(instr_t));
            new_instr->type = -1;
            new_instr->data = (char*) calloc(strlen(instr->data) + 12, sizeof(char));
            sprintf(new_instr->data, "\tpushq %s\n", instr->data);
            free_instr(instr);
            return new_instr;
        }

        default: return instr;
    }
}

char* wcstocs(wchar_t* str) {
    int len = wcslen(str);
    char* buf = (char*) calloc(len + 1, sizeof(char));
    wcstombs(buf, str, len);
    return buf;
}

instr_t* _compile_binary_instr(node_t* ast, context_t* context, instr_type_t type, char* op);
instr_t* _compile_unary_instr(node_t* ast, context_t* context, instr_type_t type, char* op);

instr_t* _compile(node_t* ast, context_t* context) {
    switch (ast->tok->type) {
        case TOK_PRINT: {
            return _compile_unary_instr(ast, context, -1, "\tpopq %r8\n\tcallq print_int\n");
        }

        case TOK_ID: {
            int offset;
            if (!(offset = get_offset(context, ast->tok->data))) {
                add_offset(context, ast->tok->data);
                offset = get_offset(context, ast->tok->data);
            }
            instr_t* instr = (instr_t*) calloc(1, sizeof(instr_t));
            instr->type = INS_OFFSET;
            instr->data = (char*) calloc(16, sizeof(char));
            sprintf(instr->data, "-%d(%%rbp)", offset);
            return instr;
        }

        case TOK_INT: {
            char buf[32];
            wcstombs(buf, ast->tok->data, 32);
            instr_t* instr = (instr_t*) calloc(1, sizeof(instr_t));
            instr->type = -1;
            instr->data = (char*) calloc(strlen(buf) + 12, sizeof(char));
            sprintf(instr->data, "\tpushq $%s\n", buf);
            return instr;
        }

        case TOK_BOOL: {
            instr_t* instr = (instr_t*) calloc(1, sizeof(instr_t));
            instr->type = -1;
            instr->data = (char*) calloc(12, sizeof(char));
            sprintf(instr->data, "\tpushq $%d\n", !wcscmp(ast->tok->data, L"صح"));
            return instr;
        }

        case TOK_ASSIGN: {
            instr_t* left = _compile(ast->left, context);
            instr_t* right = _process_offset(context, _compile(ast->right, context));
            context->offsets[extract_offset(left->data) / 8 - 1].type = tok_to_bind_type(ast->right->tok->type);
            instr_t* instr = (instr_t*) calloc(1, sizeof(instr_t));
            instr->type = -1;
            instr->data = (char*) calloc(strlen(left->data) + strlen(right->data) + 32, sizeof(char));
            sprintf(instr->data, "%s\tpopq %%rax\n\tmovq %%rax, %s\n", right->data, left->data);
            free_instr(left);
            free_instr(right);
            return instr;
        }

        case TOK_PLUS: {
            return _compile_binary_instr(ast, context, -1, "\tpopq %rbx\n\tpopq %rax\n\taddq %rbx, %rax\n\tpushq %rax\n");
        }

        case TOK_MINUS: {
            if (!ast->right) {
                char buf[32];
                wcstombs(buf, ast->left->tok->data, 32);
                instr_t* instr = (instr_t*) calloc(1, sizeof(instr_t));
                instr->type = -1;
                instr->data = (char*) calloc(strlen(buf) + 12, sizeof(char));
                sprintf(instr->data, "\tpushq $-%s\n", buf);
                return instr;
            }
            return _compile_binary_instr(ast, context, -1, "\tpopq %rbx\n\tpopq %rax\n\tsubq %rbx, %rax\n\tpushq %rax\n");
        }

        case TOK_MUL: {
            return _compile_binary_instr(ast, context, -1, "\tpopq %rbx\n\tpopq %rax\n\timulq %rbx, %rax\n\tpushq %rax\n");
        }

        case TOK_DIV: {
            return _compile_binary_instr(ast, context, -1, "\tpopq %rbx\n\tpopq %rax\n\tmovq $0, %rdx\n\tidivq %rbx\n\tpushq %rax\n");
        }

        case TOK_MOD: {
            return _compile_binary_instr(ast, context, -1, "\tpopq %rbx\n\tpopq %rax\n\tmovq $0, %rdx\n\tidivq %rbx\n\tpushq %rdx\n");
        }

        case TOK_AND: {
            return _compile_binary_instr(ast, context, -1, "\tpopq %rbx\n\tpopq %rax\n\tandq %rbx, %rax\n\tpushq %rax\n");
        }

        case TOK_OR: {
            return _compile_binary_instr(ast, context, -1, "\tpopq %rbx\n\tpopq %rax\n\torq %rbx, %rax\n\tpushq %rax\n");
        }

        case TOK_NOT: {
            return _compile_unary_instr(ast, context, -1, "\tpopq %rax\n\tcmpq $0, %rax\n\tsete %al\n\tmovzbq %al, %rax\n\tpushq %rax\n");
        }

        case TOK_EQ: {
            return _compile_binary_instr(ast, context, -1, "\tpopq %rbx\n\tpopq %rax\n\tcmpq %rbx, %rax\n\tsete %al\n\tmovzbq %al, %rax\n\tpushq %rax\n");
        }

        case TOK_NE: {
            return _compile_binary_instr(ast, context, -1, "\tpopq %rbx\n\tpopq %rax\n\tcmpq %rbx, %rax\n\tsetne %al\n\tmovzbq %al, %rax\n\tpushq %rax\n");
        }

        case TOK_LT: {
            return _compile_binary_instr(ast, context, -1, "\tpopq %rbx\n\tpopq %rax\n\tcmpq %rbx, %rax\n\tsetl %al\n\tmovzbq %al, %rax\n\tpushq %rax\n");
        }

        case TOK_LTE: {
            return _compile_binary_instr(ast, context, -1, "\tpopq %rbx\n\tpopq %rax\n\tcmpq %rbx, %rax\n\tsetle %al\n\tmovzbq %al, %rax\n\tpushq %rax\n");
        }

        case TOK_GT: {
            return _compile_binary_instr(ast, context, -1, "\tpopq %rbx\n\tpopq %rax\n\tcmpq %rbx, %rax\n\tsetg %al\n\tmovzbq %al, %rax\n\tpushq %rax\n");
        }

        case TOK_GTE: {
            return _compile_binary_instr(ast, context, -1, "\tpopq %rbx\n\tpopq %rax\n\tcmpq %rbx, %rax\n\tsetge %al\n\tmovzbq %al, %rax\n\tpushq %rax\n");
        }

        case TOK_IF: {
            instr_t* left = _process_offset(context, _compile(ast->left, context));
            char* buf = (char*) calloc(1, sizeof(char));
            for (int i = 0; i < ast->right->size; i++) {
                instr_t* ins = _compile(ast->right->astList[i], context);
                buf = (char*) realloc(buf, (strlen(buf) + strlen(ins->data) + 1) * sizeof(char));
                strcat(buf, ins->data);
                free_instr(ins);
            }

            instr_t* instr = (instr_t*) calloc(1, sizeof(instr_t));
            instr->type = -1;

            char* format = "%s\tpopq %%rax\n\tcmpq $1, %%rax\n\tjne %s\n%s%s:\n";
            char* label = get_label(context);
            instr->data = (char*) calloc(strlen(left->data) + strlen(buf) + strlen(label) * 2 + strlen(format) + 1, sizeof(char));
            sprintf(instr->data, format, left->data, label, buf, label);

            free_instr(left);
            free(buf);
            free(label);
            return instr;
        }

        case TOK_WHILE: {
            instr_t* left = _process_offset(context, _compile(ast->left, context));
            char* buf = (char*) calloc(1, sizeof(char));
            for (int i = 0; i < ast->right->size; i++) {
                instr_t* ins = _compile(ast->right->astList[i], context);
                buf = (char*) realloc(buf, (strlen(buf) + strlen(ins->data) + 1) * sizeof(char));
                strcat(buf, ins->data);
                free_instr(ins);
            }

            instr_t* instr = (instr_t*) calloc(1, sizeof(instr_t));
            instr->type = -1;

            char* format = "%s:\n%s\tpopq %%rax\n\tcmpq $1, %%rax\n\tjne %s\n%s\tjmp %s\n%s:\n";
            char* l1 = get_label(context);
            char* l2 = get_label(context);
            instr->data = (char*) calloc(strlen(left->data) + strlen(buf) + strlen(l1) * 2 + strlen(l2) * 2 + strlen(format) + 1, sizeof(char));
            sprintf(instr->data, format, l1, left->data, l2, buf, l1, l2);

            free_instr(left);
            free(buf);
            free(l1);
            free(l2);
            return instr;
        }

        case TOK_FOR: {
            instr_t* left = _compile(ast->left, context);
            context->offsets[extract_offset(left->data) / 8 - 1].type = BIND_INT;
            char* start = wcstocs(ast->right->astList[0]->tok->data);
            char* end = wcstocs(ast->right->astList[1]->tok->data);
            char* step = wcstocs(ast->right->astList[2]->tok->data);
            char* buf = (char*) calloc(1, sizeof(char));
            for (int i = 0; i < ast->size; i++) {
                instr_t* ins = _compile(ast->astList[i], context);
                buf = (char*) realloc(buf, (strlen(buf) + strlen(ins->data) + 1) * sizeof(char));
                strcat(buf, ins->data);
                free_instr(ins);
            }

            instr_t* instr = (instr_t*) calloc(1, sizeof(instr_t));
            instr->type = -1;

            char* format = "\tmovq $%s, %s\n%s:\n\tcmpq $%s, %s\n\t%s %s\n%s\taddq $%s, %s\n\tjmp %s\n%s:\n";
            char* cond = (step[0] == '-')? "jl" : "jg";
            char* l1 = get_label(context);
            char* l2 = get_label(context);
            instr->data = (char*) calloc(strlen(left->data) * 3 + strlen(buf) + strlen(start) + strlen(end) + strlen(step) + strlen(l1) * 2 + strlen(l2) * 2 + strlen(format) + 3, sizeof(char));
            sprintf(instr->data, format, start, left->data, l1, end, left->data, cond, l2, buf, step, left->data, l1, l2);

            free_instr(left);
            free(start);
            free(end);
            free(step);
            free(buf);
            free(l1);
            free(l2);
            return instr;
        }

        default: {
            wprintf(L"Error: Undefined compilation for `%ls`\n", tok_type_to_str(ast->tok->type));
            exit(1);
        }
    }
}

instr_t* _compile_binary_instr(node_t* ast, context_t* context, instr_type_t type, char* op) {
    instr_t* left = _process_offset(context, _compile(ast->left, context));
    instr_t* right = _process_offset(context, _compile(ast->right, context));
    instr_t* instr = (instr_t*) calloc(1, sizeof(instr_t));
    instr->type = type;
    instr->data = (char*) calloc(strlen(left->data) + strlen(right->data) + strlen(op) + 1, sizeof(char));
    strcat(instr->data, left->data);
    strcat(instr->data, right->data);
    strcat(instr->data, op);
    free_instr(left);
    free_instr(right);
    return instr;
}

instr_t* _compile_unary_instr(node_t* ast, context_t* context, instr_type_t type, char* op) {
    instr_t* left = _process_offset(context, _compile(ast->left, context));
    instr_t* instr = (instr_t*) calloc(1, sizeof(instr_t));
    instr->type = type;
    instr->data = (char*) calloc(strlen(left->data) + strlen(op) + 1, sizeof(char));
    strcat(instr->data, left->data);
    strcat(instr->data, op);
    free_instr(left);
    return instr;
}

void compile(parser_t* parser, int _s) {
    sections_t sections;
    init_headers(&sections);
    context_t context;
    init_context(&context);

    char* buf = (char*) calloc(1, sizeof(char));
    for (int i = 0; i < parser->size; i++) {
        instr_t* instr = _compile(parser->astList[i], &context);
        buf = (char*) realloc(buf, (strlen(buf) + strlen(instr->data) + 1) * sizeof(char));
        strcat(buf, instr->data);
        free_instr(instr);
    }
    
    init_prologue(&sections, &context);
    sections.text = (char*) realloc(sections.text, (strlen(sections.text) + strlen(buf) + 1) * sizeof(char));
    strcat(sections.text, buf);
    init_epilogue(&sections, &context);

    FILE* output;
    if ((output = fopen("./a.s", "w")) == NULL) {
        wprintf(L"Could not open file ./a.s\n");
        exit(1);
    }
    fputs(sections.rodata, output);
    fputs(sections.text, output);
    fclose(output);
    
    if (!_s) {
        system("as a.s include/asm/* -o a.o");
        system("ld a.o -o a.out");
        system("rm a.o");
        system("rm a.s");
    }
    
    free_context(&context);
    free_sections(&sections);
}