#include "../include/compiler.h"

void init_headers(sections_t* sections) {
    wchar_t* rodata = L".section .rodata\n";
    sections->rodata = (wchar_t*) calloc(wcslen(rodata) + 1, sizeof(wchar_t));
    wcscat(sections->rodata, rodata);

    wchar_t* text = L".section .text\n.globl _start\n_start:\n\tcallq _entry\n\tmovq %rax, %rdi\n\tmovq $60, %rax\n\tsyscall\n.globl _entry\n_entry:\n";
    sections->text = (wchar_t*) calloc(wcslen(text) + 1, sizeof(wchar_t));
    wcscat(sections->text, text);
}

void init_prologue(sections_t* sections, context_t* context) {
    sections->text = (wchar_t*) realloc(sections->text, (wcslen(sections->text) + 30) * sizeof(wchar_t));
    wcscat(sections->text, L"\tpushq %rbp\n\tmovq %rsp, %rbp\n");

    if (context->offsets->size) {
        wchar_t* alloc = (wchar_t*) calloc(24, sizeof(wchar_t));
        swprintf(alloc, 24, L"\tsubq $%d, %%rsp\n", context->offsets->size * 8);
        sections->text = (wchar_t*) realloc(sections->text, (wcslen(sections->text) + wcslen(alloc) + 1) * sizeof(wchar_t));
        wcscat(sections->text, alloc);
        free(alloc);
    }
}

void init_epilogue(sections_t* sections) {
    sections->text = (wchar_t*) realloc(sections->text, (wcslen(sections->text) + 32) * sizeof(wchar_t));
    wcscat(sections->text, L"\tmovq $0, %rax\n\tleaveq\n\tretq\n");
}

void free_sections(sections_t* sections) {
    free(sections->rodata);
    free(sections->text);
}

void init_context(context_t* context) {
    context->offsets = (_offsets_list_t*) calloc(1, sizeof(_offsets_list_t));
    context->offsets->cap = 8;
    context->offsets->size = 0;
    context->offsets->arr = (wchar_t**) calloc(context->offsets->cap, sizeof(wchar_t*));
    context->offsets->types = (bind_type_t*) calloc(context->offsets->cap, sizeof(bind_type_t));
    context->strings = (_strings_list_t*) calloc(1, sizeof(_strings_list_t));
    context->strings->cap = 8;
    context->strings->size = 0;
    context->strings->arr = (int*) calloc(context->strings->cap, sizeof(int));
}

int get_offset(context_t* context, wchar_t* id) {
    for (int i = 0; i < context->offsets->size; i++) {
        if (!wcscmp(context->offsets->arr[i], id)) {
            return (i + 1) * 8;
        }
    }

    context->offsets->arr[context->offsets->size] = id;
    context->offsets->types[context->offsets->size++] = -1;
    if (context->offsets->size >= context->offsets->cap) {
        context->offsets->cap *= 2;
        context->offsets->arr = (wchar_t**) realloc(context->offsets->arr, context->offsets->cap * sizeof(wchar_t*));
    }
    return context->offsets->size * 8;
}

void bind_string(context_t* context, int offset) {
    context->strings->arr[context->strings->size - 1] = offset;
    if (context->strings->size >= context->strings->cap) {
        context->strings->cap *= 2;
        context->strings->arr = (int*) realloc(context->strings->arr, context->strings->cap * sizeof(int));
    }
}

int get_string(context_t* context, int offset) {
    for (int i = 0; i < context->strings->size; i++) {
        if (context->strings->arr[i] == offset) {
            return i;
        }
    }
    return -1;
}

long extract_offset(wchar_t* offset) {
    wchar_t* start = wcschr(offset, L'-') + 1;
    wchar_t buf[32] = {};
    wcsncpy(buf, start, wcschr(start, L'(') - start);
    return wcstol(buf, 0, 10);
}

bind_type_t tok_to_bind_type(tok_type_t type) {
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
        case TOK_CHAR:
            return BIND_CHAR;
        case TOK_STR:
            return BIND_STR;
        default:
            return -1;
    }
}

wchar_t* get_label(context_t* context) {
    wchar_t* label = (wchar_t*) calloc(8, sizeof(wchar_t));
    swprintf(label, 7, L".L%d", context->labels++);
    return label;
}

void free_context(context_t* context) {
    free(context->offsets->arr);
    free(context->offsets->types);
    free(context->offsets);
    free(context->strings->arr);
    free(context->strings);
}

void free_instr(instr_t* instr) {
    free(instr->data);
    free(instr);
}

instr_t* _compile_binary_instr(node_t* ast, context_t* context, sections_t* sections, instr_type_t type, wchar_t* op);
instr_t* _compile_unary_instr(node_t* ast, context_t* context, sections_t* sections, instr_type_t type, wchar_t* op);

instr_t* _compile(node_t* ast, context_t* context, sections_t* sections) {
    switch (ast->tok->type) {
        case TOK_PRINT: {
            instr_t* left = _compile(ast->left, context, sections);
            instr_t* instr = (instr_t*) calloc(1, sizeof(instr_t));
            instr->type = -1;
            switch (context->offsets->types[extract_offset(left->data) / 8 - 1]) {
                case BIND_INT: {
                    size_t len = wcslen(left->data) + 36;
                    instr->data = (wchar_t*) calloc(len, sizeof(wchar_t));
                    swprintf(instr->data, len, L"\tmovq %ls, %%r8\n\tcallq print_int\n", left->data);
                    break;
                }
                case BIND_CHAR: {
                    size_t len = wcslen(left->data) + 36;
                    instr->data = (wchar_t*) calloc(len, sizeof(wchar_t));
                    swprintf(instr->data, len, L"\tmovq %ls, %%r8\n\tcallq print_char\n", left->data);
                    break;
                }
                case BIND_STR: {
                    instr->data = (wchar_t*) calloc(64, sizeof(wchar_t));
                    int string = get_string(context, extract_offset(left->data));
                    swprintf(instr->data, 64, L"\tmovq $str%d, %%r8\n\tmovq $len%d, %%r9\n\tcallq print\n", string, string);
                    break;
                }
                default: {
                    wprintf(L"Error: Undefined variable `%ls`\n", ast->left->tok->data);
                    exit(1);
                }
            }
            free_instr(left);
            return instr;
        }

        case TOK_ID: {
            int offset = get_offset(context, ast->tok->data);
            instr_t* instr = (instr_t*) calloc(1, sizeof(instr_t));
            instr->type = INS_OFFSET;
            instr->data = (wchar_t*) calloc(16, sizeof(wchar_t));
            swprintf(instr->data, 16, L"\tpushq -%d(%%rbp)\n", offset);
            return instr;
        }

        case TOK_INT: {
            instr_t* instr = (instr_t*) calloc(1, sizeof(instr_t));
            instr->type = -1;
            int len = ast->tok->len + 16;
            instr->data = (wchar_t*) calloc(len, sizeof(wchar_t));
            swprintf(instr->data, len, L"\tpushq $%ls\n", ast->tok->data);
            return instr;
        }

        case TOK_BOOL: {
            instr_t* instr = (instr_t*) calloc(1, sizeof(instr_t));
            instr->type = -1;
            instr->data = (wchar_t*) calloc(12, sizeof(wchar_t));
            swprintf(instr->data, 12, L"\tpushq $%d\n", !wcscmp(ast->tok->data, L"صح"));
            return instr;
        }

        case TOK_CHAR: {
            instr_t* instr = (instr_t*) calloc(1, sizeof(instr_t));
            instr->type = -1;
            instr->data = (wchar_t*) calloc(16, sizeof(wchar_t));
            swprintf(instr->data, 16, L"\tpushq $%d\n", wctob(ast->tok->data[0]));
            return instr;
        }

        case TOK_STR: {
            int strnum = context->strings->size++;
            size_t len = wcslen(ast->tok->data) + 50;
            wchar_t buf[len];
            swprintf(buf, len, L"\tstr%d: .string \"%ls\"\n\t.equ len%d, .-str%d\n", strnum, ast->tok->data, strnum, strnum);
            sections->rodata = (wchar_t*) realloc(sections->rodata, (wcslen(sections->rodata) + wcslen(buf) + 1) * sizeof(wchar_t));
            wcscat(sections->rodata, buf);
            instr_t* instr = (instr_t*) calloc(1, sizeof(instr_t));
            instr->type = INS_STRING;
            instr->data = (wchar_t*) calloc(16, sizeof(wchar_t));
            swprintf(instr->data, 16, L"\tpushq $str%d\n", strnum);
            return instr;
        }

        case TOK_ASSIGN: {
            instr_t* left = _compile(ast->left, context, sections);
            instr_t* right = _compile(ast->right, context, sections);
            if (right->type == INS_STRING) {
                bind_string(context, extract_offset(left->data));
            }
            context->offsets->types[extract_offset(left->data) / 8 - 1] = tok_to_bind_type(ast->right->tok->type);
            instr_t* instr = (instr_t*) calloc(1, sizeof(instr_t));
            instr->type = -1;
            size_t len = wcslen(left->data) + wcslen(right->data) + 16;
            instr->data = (wchar_t*) calloc(len, sizeof(wchar_t));
            swprintf(instr->data, len, L"%ls\tpopq %ls\n", right->data, wcschr(left->data, L' ') + 1);
            free_instr(left);
            free_instr(right);
            return instr;
        }

        case TOK_PLUS: {
            return _compile_binary_instr(ast, context, sections, -1, L"\tpopq %rbx\n\tpopq %rax\n\taddq %rbx, %rax\n\tpushq %rax\n");
        }

        case TOK_MINUS: {
            if (!ast->right) {
                return _compile_unary_instr(ast, context, sections, -1, L"\tpopq %rax\n\tnegq %rax\n\tpushq %rax\n");
            }
            return _compile_binary_instr(ast, context, sections, -1, L"\tpopq %rbx\n\tpopq %rax\n\tsubq %rbx, %rax\n\tpushq %rax\n");
        }

        case TOK_MUL: {
            return _compile_binary_instr(ast, context, sections, -1, L"\tpopq %rbx\n\tpopq %rax\n\timulq %rbx, %rax\n\tpushq %rax\n");
        }

        case TOK_DIV: {
            return _compile_binary_instr(ast, context, sections, -1, L"\tpopq %rbx\n\tpopq %rax\n\tmovq $0, %rdx\n\tidivq %rbx\n\tpushq %rax\n");
        }

        case TOK_MOD: {
            return _compile_binary_instr(ast, context, sections, -1, L"\tpopq %rbx\n\tpopq %rax\n\tmovq $0, %rdx\n\tidivq %rbx\n\tpushq %rdx\n");
        }

        case TOK_AND: {
            return _compile_binary_instr(ast, context, sections, -1, L"\tpopq %rbx\n\tpopq %rax\n\tandq %rbx, %rax\n\tpushq %rax\n");
        }

        case TOK_OR: {
            return _compile_binary_instr(ast, context, sections, -1, L"\tpopq %rbx\n\tpopq %rax\n\torq %rbx, %rax\n\tpushq %rax\n");
        }

        case TOK_NOT: {
            return _compile_unary_instr(ast, context, sections, -1, L"\tpopq %rax\n\tcmpq $0, %rax\n\tsete %al\n\tmovzbq %al, %rax\n\tpushq %rax\n");
        }

        case TOK_EQ: {
            return _compile_binary_instr(ast, context, sections, -1, L"\tpopq %rbx\n\tpopq %rax\n\tcmpq %rbx, %rax\n\tsete %al\n\tmovzbq %al, %rax\n\tpushq %rax\n");
        }

        case TOK_NE: {
            return _compile_binary_instr(ast, context, sections, -1, L"\tpopq %rbx\n\tpopq %rax\n\tcmpq %rbx, %rax\n\tsetne %al\n\tmovzbq %al, %rax\n\tpushq %rax\n");
        }

        case TOK_LT: {
            return _compile_binary_instr(ast, context, sections, -1, L"\tpopq %rbx\n\tpopq %rax\n\tcmpq %rbx, %rax\n\tsetl %al\n\tmovzbq %al, %rax\n\tpushq %rax\n");
        }

        case TOK_LTE: {
            return _compile_binary_instr(ast, context, sections, -1, L"\tpopq %rbx\n\tpopq %rax\n\tcmpq %rbx, %rax\n\tsetle %al\n\tmovzbq %al, %rax\n\tpushq %rax\n");
        }

        case TOK_GT: {
            return _compile_binary_instr(ast, context, sections, -1, L"\tpopq %rbx\n\tpopq %rax\n\tcmpq %rbx, %rax\n\tsetg %al\n\tmovzbq %al, %rax\n\tpushq %rax\n");
        }

        case TOK_GTE: {
            return _compile_binary_instr(ast, context, sections, -1, L"\tpopq %rbx\n\tpopq %rax\n\tcmpq %rbx, %rax\n\tsetge %al\n\tmovzbq %al, %rax\n\tpushq %rax\n");
        }

        case TOK_IF: {
            instr_t* left = _compile(ast->left, context, sections);
            wchar_t* buf = (wchar_t*) calloc(1, sizeof(wchar_t));
            for (int i = 0; i < ast->right->size; i++) {
                instr_t* ins = _compile(ast->right->astList[i], context, sections);
                buf = (wchar_t*) realloc(buf, (wcslen(buf) + wcslen(ins->data) + 1) * sizeof(wchar_t));
                wcscat(buf, ins->data);
                free_instr(ins);
            }

            instr_t* instr = (instr_t*) calloc(1, sizeof(instr_t));
            instr->type = -1;

            wchar_t* format = L"%ls\tpopq %%rax\n\tcmpq $1, %%rax\n\tjne %ls\n%ls%ls:\n";
            wchar_t* label = get_label(context);
            size_t len = wcslen(left->data) + wcslen(buf) + wcslen(label) * 2 + wcslen(format);
            instr->data = (wchar_t*) calloc(len + 1, sizeof(wchar_t));
            swprintf(instr->data, len, format, left->data, label, buf, label);

            free_instr(left);
            free(buf);
            free(label);
            return instr;
        }

        case TOK_WHILE: {
            instr_t* left = _compile(ast->left, context, sections);
            wchar_t* buf = (wchar_t*) calloc(1, sizeof(wchar_t));
            for (int i = 0; i < ast->right->size; i++) {
                instr_t* ins = _compile(ast->right->astList[i], context, sections);
                buf = (wchar_t*) realloc(buf, (wcslen(buf) + wcslen(ins->data) + 1) * sizeof(wchar_t));
                wcscat(buf, ins->data);
                free_instr(ins);
            }

            instr_t* instr = (instr_t*) calloc(1, sizeof(instr_t));
            instr->type = -1;

            wchar_t* format = L"%ls:\n%ls\tpopq %%rax\n\tcmpq $1, %%rax\n\tjne %ls\n%ls\tjmp %ls\n%ls:\n";
            wchar_t* l1 = get_label(context);
            wchar_t* l2 = get_label(context);
            size_t len = wcslen(left->data) + wcslen(buf) + wcslen(l1) * 2 + wcslen(l2) * 2 + wcslen(format);
            instr->data = (wchar_t*) calloc(len + 1, sizeof(wchar_t));
            swprintf(instr->data, len, format, l1, left->data, l2, buf, l1, l2);

            free_instr(left);
            free(buf);
            free(l1);
            free(l2);
            return instr;
        }

        case TOK_FOR: {
            instr_t* left = _compile(ast->left, context, sections);
            context->offsets->types[extract_offset(left->data) / 8 - 1] = BIND_INT;

            instr_t* start = _compile(ast->right->astList[0], context, sections);
            instr_t* end = _compile(ast->right->astList[1], context, sections);
            instr_t* step = _compile(ast->right->astList[2], context, sections);

            wchar_t* buf = (wchar_t*) calloc(1, sizeof(wchar_t));
            for (int i = 0; i < ast->size; i++) {
                instr_t* ins = _compile(ast->astList[i], context, sections);
                buf = (wchar_t*) realloc(buf, (wcslen(buf) + wcslen(ins->data) + 1) * sizeof(wchar_t));
                wcscat(buf, ins->data);
                free_instr(ins);
            }

            instr_t* instr = (instr_t*) calloc(1, sizeof(instr_t));
            instr->type = -1;

            wchar_t* format = L"%ls\tpopq %ls\n%ls:\n%ls\tpopq %%rax\n\tcmpq %%rax, %ls\n\t%ls %ls\n%ls%ls\tpopq %%rax\n\taddq %%rax, %ls\n\tjmp %ls\n%ls:\n";
            wchar_t* jmp = (ast->right->astList[2]->tok->type == TOK_MINUS)? L"jl" : L"jg";
            wchar_t* l1 = get_label(context);
            wchar_t* l2 = get_label(context);
            size_t len = wcslen(left->data) * 3 + wcslen(buf) + wcslen(start->data) + wcslen(end->data) + wcslen(step->data) + wcslen(l1) * 2 + wcslen(l2) * 2 + wcslen(format) + 2;
            instr->data = (wchar_t*) calloc(len + 1, sizeof(wchar_t));
            swprintf(instr->data, len, format, start->data, left->data, l1, end->data, left->data, jmp, l2, buf, step->data, left->data, l1, l2);

            free_instr(left);
            free_instr(start);
            free_instr(end);
            free_instr(step);
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

instr_t* _compile_binary_instr(node_t* ast, context_t* context, sections_t* sections, instr_type_t type, wchar_t* op) {
    instr_t* left = _compile(ast->left, context, sections);
    instr_t* right = _compile(ast->right, context, sections);
    instr_t* instr = (instr_t*) calloc(1, sizeof(instr_t));
    instr->type = type;
    instr->data = (wchar_t*) calloc(wcslen(left->data) + wcslen(right->data) + wcslen(op) + 1, sizeof(wchar_t));
    wcscat(instr->data, left->data);
    wcscat(instr->data, right->data);
    wcscat(instr->data, op);
    free_instr(left);
    free_instr(right);
    return instr;
}

instr_t* _compile_unary_instr(node_t* ast, context_t* context, sections_t* sections, instr_type_t type, wchar_t* op) {
    instr_t* left = _compile(ast->left, context, sections);
    instr_t* instr = (instr_t*) calloc(1, sizeof(instr_t));
    instr->type = type;
    instr->data = (wchar_t*) calloc(wcslen(left->data) + wcslen(op) + 1, sizeof(wchar_t));
    wcscat(instr->data, left->data);
    wcscat(instr->data, op);
    free_instr(left);
    return instr;
}

void compile(parser_t* parser, int _s) {
    sections_t sections;
    init_headers(&sections);
    context_t context;
    init_context(&context);
    
    wchar_t* buf = (wchar_t*) calloc(1, sizeof(wchar_t));
    for (int i = 0; i < parser->size; i++) {
        instr_t* instr = _compile(parser->astList[i], &context, &sections);
        buf = (wchar_t*) realloc(buf, (wcslen(buf) + wcslen(instr->data) + 1) * sizeof(wchar_t));
        wcscat(buf, instr->data);
        free_instr(instr);
    }
    
    init_prologue(&sections, &context);
    sections.text = (wchar_t*) realloc(sections.text, (wcslen(sections.text) + wcslen(buf) + 1) * sizeof(wchar_t));
    wcscat(sections.text, buf);
    free(buf);
    init_epilogue(&sections);

    FILE* output;
    if ((output = fopen("./a.s", "w")) == NULL) {
        wprintf(L"Could not open file ./a.s\n");
        exit(1);
    }
    fputws(sections.rodata, output);
    fputws(sections.text, output);
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