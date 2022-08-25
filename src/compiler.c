#include "../include/compiler.h"

extern ERROR_STATUS err_status;

wchar_t* init_prologue(int stack_alloc) {
    wchar_t* prologue;

    if (stack_alloc) {
        prologue = (wchar_t*) calloc(64, sizeof(wchar_t));
        swprintf(prologue, 64, L"\tpushq %%rbp\n\tmovq %%rsp, %%rbp\n\tsubq $%d, %%rsp\n", stack_alloc * 8);
    } else {
        prologue = (wchar_t*) calloc(32, sizeof(wchar_t));
        wcscat(prologue, L"\tpushq %rbp\n\tmovq %rsp, %rbp\n");
    }

    return prologue;
}

wchar_t* init_epilogue() {
    wchar_t* epilogue = (wchar_t*) calloc(32, sizeof(wchar_t));
    wcscat(epilogue, L"\tmovq $0, %rax\n\tleaveq\n\tretq\n");
    return epilogue;
}

wchar_t* get_label(sections_t* sections) {
    wchar_t* label = (wchar_t*) calloc(8, sizeof(wchar_t));
    swprintf(label, 7, L".L%d", sections->labels++);
    return label;
}

long extract_offset(wchar_t* offset) {
    wchar_t* start = wcschr(offset, L'-') + 1;
    wchar_t buf[32] = {};
    wcsncpy(buf, start, wcschr(start, L'(') - start);
    return wcstol(buf, 0, 10);
}

wchar_t* _compile_binary_instr(ast_t* ast, scope_t* scope, sections_t* sections, wchar_t* op);
wchar_t* _compile_unary_instr(ast_t* ast, scope_t* scope, sections_t* sections, wchar_t* op);

wchar_t* _compile(ast_t* ast, scope_t* scope, sections_t* sections) {
    switch (ast->tok->type) {
        case TOK_PRINT: {
            wchar_t* left = _compile(ast->left, scope, sections);
            if (err_status != ERR_NONE) {
                return 0;
            }
            size_t len = wcslen(left) + 36;
            wchar_t* instr = (wchar_t*) calloc(len, sizeof(wchar_t));
            switch (scope->types[extract_offset(left) / 8 - 1]) {
                case VAR_INT: {
                    swprintf(instr, len, L"%ls\tpopq %%r8\n\tcallq print_int\n", left);
                    break;
                }
                case VAR_BOOL: {
                    swprintf(instr, len, L"%ls\tpopq %%r8\n\tcallq print_bool\n", left);
                    break;
                }
                case VAR_STR: {
                    swprintf(instr, len, L"%ls\tpopq %%r8\n\tcallq print_str\n", left);
                    break;
                }
                default: {
                    wprintf(L"Error: Undefined type for variable `%ls`\n", ast->left->tok->data);
                    err_status = ERR_COMPILE;
                    free(left);
                    free(instr);
                    return 0;
                }
            }
            free(left);
            return instr;
        }

        case TOK_READ: {
            int bufnum = sections->labels++;
            wchar_t buf[20];
            swprintf(buf, 20, L"\tbuf%d: .skip 80\n", bufnum);
            sections->bss = (wchar_t*) realloc(sections->bss, (wcslen(sections->bss) + wcslen(buf) + 1) * sizeof(wchar_t));
            wcscat(sections->bss, buf);
            wchar_t* instr = (wchar_t*) calloc(64, sizeof(wchar_t));
            swprintf(instr, 64, L"\tmovq $buf%d, %%r8\n\tcallq read_line\n\tpushq $buf%d\n", bufnum, bufnum);
            return instr;
        }

        case TOK_LET: {
            int offset = add_var(scope, ast->left->tok->data);
            if (offset == -1) {
                wprintf(L"Error: Duplicate definition for `%ls`\n", ast->left->tok->data);
                err_status = ERR_COMPILE;
                return 0;
            }
            wchar_t* instr = (wchar_t*) calloc(24, sizeof(wchar_t));
            swprintf(instr, 24, L"\tpushq -%d(%%rbp)\n", (offset + 1) * 8);
            return instr;
        }

        case TOK_ID: {
            int offset = get_var(scope, ast->tok->data);
            if (offset == -1) {
                wprintf(L"Error: Undefined variable `%ls`\n", ast->tok->data);
                err_status = ERR_COMPILE;
                return 0;
            }
            wchar_t* instr = (wchar_t*) calloc(24, sizeof(wchar_t));
            swprintf(instr, 24, L"\tpushq -%d(%%rbp)\n", (offset + 1) * 8);
            return instr;
        }

        case TOK_INT: {
            int len = ast->tok->len + 16;
            wchar_t* instr = (wchar_t*) calloc(len, sizeof(wchar_t));
            swprintf(instr, len, L"\tpushq $%ls\n", ast->tok->data);
            return instr;
        }

        case TOK_BOOL: {
            wchar_t* instr = (wchar_t*) calloc(16, sizeof(wchar_t));
            swprintf(instr, 16, L"\tpushq $%d\n", !wcscmp(ast->tok->data, L"صح"));
            return instr;
        }

        case TOK_CHAR: {
            wchar_t* instr = (wchar_t*) calloc(24, sizeof(wchar_t));
            swprintf(instr, 24, L"\tpushq $0x%x\n", ast->tok->data[0]);
            return instr;
        }

        case TOK_STR: {
            int strnum = sections->labels++;
            size_t len = wcslen(ast->tok->data) + 32;
            wchar_t buf[len];
            swprintf(buf, len, L"\tstr%d: .string \"%ls\"\n", strnum, ast->tok->data);
            sections->rodata = (wchar_t*) realloc(sections->rodata, (wcslen(sections->rodata) + wcslen(buf) + 1) * sizeof(wchar_t));
            wcscat(sections->rodata, buf);
            wchar_t* instr = (wchar_t*) calloc(20, sizeof(wchar_t));
            swprintf(instr, 20, L"\tpushq $str%d\n", strnum);
            return instr;
        }

        case TOK_ASSIGN: {
            wchar_t* left = _compile(ast->left, scope, sections);
            if (err_status != ERR_NONE) {
                return 0;
            }
            wchar_t* right = _compile(ast->right, scope, sections);
            if (err_status != ERR_NONE) {
                free(left);
                return 0;
            }
            scope->types[extract_offset(left) / 8 - 1] = VAR_INT;
            size_t len = wcslen(left) + wcslen(right) + 16;
            wchar_t* instr = (wchar_t*) calloc(len, sizeof(wchar_t));
            swprintf(instr, len, L"%ls\tpopq %ls", right, wcschr(left, L'-'));
            free(left);
            free(right);
            return instr;
        }

        case TOK_PLUS: {
            return _compile_binary_instr(ast, scope, sections, L"\tpopq %rbx\n\tpopq %rax\n\taddq %rbx, %rax\n\tpushq %rax\n");
        }

        case TOK_MINUS: {
            if (!ast->right) {
                return _compile_unary_instr(ast, scope, sections, L"\tpopq %rax\n\tnegq %rax\n\tpushq %rax\n");
            }
            return _compile_binary_instr(ast, scope, sections, L"\tpopq %rbx\n\tpopq %rax\n\tsubq %rbx, %rax\n\tpushq %rax\n");
        }

        case TOK_MUL: {
            return _compile_binary_instr(ast, scope, sections, L"\tpopq %rbx\n\tpopq %rax\n\timulq %rbx, %rax\n\tpushq %rax\n");
        }

        case TOK_DIV: {
            return _compile_binary_instr(ast, scope, sections, L"\tpopq %rbx\n\tpopq %rax\n\tmovq $0, %rdx\n\tidivq %rbx\n\tpushq %rax\n");
        }

        case TOK_MOD: {
            return _compile_binary_instr(ast, scope, sections, L"\tpopq %rbx\n\tpopq %rax\n\tmovq $0, %rdx\n\tidivq %rbx\n\tpushq %rdx\n");
        }

        case TOK_AND: {
            return _compile_binary_instr(ast, scope, sections, L"\tpopq %rbx\n\tpopq %rax\n\tandq %rbx, %rax\n\tpushq %rax\n");
        }

        case TOK_OR: {
            return _compile_binary_instr(ast, scope, sections, L"\tpopq %rbx\n\tpopq %rax\n\torq %rbx, %rax\n\tpushq %rax\n");
        }

        case TOK_NOT: {
            return _compile_unary_instr(ast, scope, sections, L"\tpopq %rax\n\tcmpq $0, %rax\n\tsete %al\n\tmovzbq %al, %rax\n\tpushq %rax\n");
        }

        case TOK_EQ: {
            return _compile_binary_instr(ast, scope, sections, L"\tpopq %rbx\n\tpopq %rax\n\tcmpq %rbx, %rax\n\tsete %al\n\tmovzbq %al, %rax\n\tpushq %rax\n");
        }

        case TOK_NE: {
            return _compile_binary_instr(ast, scope, sections, L"\tpopq %rbx\n\tpopq %rax\n\tcmpq %rbx, %rax\n\tsetne %al\n\tmovzbq %al, %rax\n\tpushq %rax\n");
        }

        case TOK_LT: {
            return _compile_binary_instr(ast, scope, sections, L"\tpopq %rbx\n\tpopq %rax\n\tcmpq %rbx, %rax\n\tsetl %al\n\tmovzbq %al, %rax\n\tpushq %rax\n");
        }

        case TOK_LTE: {
            return _compile_binary_instr(ast, scope, sections, L"\tpopq %rbx\n\tpopq %rax\n\tcmpq %rbx, %rax\n\tsetle %al\n\tmovzbq %al, %rax\n\tpushq %rax\n");
        }

        case TOK_GT: {
            return _compile_binary_instr(ast, scope, sections, L"\tpopq %rbx\n\tpopq %rax\n\tcmpq %rbx, %rax\n\tsetg %al\n\tmovzbq %al, %rax\n\tpushq %rax\n");
        }

        case TOK_GTE: {
            return _compile_binary_instr(ast, scope, sections, L"\tpopq %rbx\n\tpopq %rax\n\tcmpq %rbx, %rax\n\tsetge %al\n\tmovzbq %al, %rax\n\tpushq %rax\n");
        }

        case TOK_IF: {
            wchar_t* left = _compile(ast->left->list[0], scope, sections);
            if (err_status != ERR_NONE) {
                return 0;
            }
            wchar_t* block = _compile(ast->right, scope, sections);
            if (err_status != ERR_NONE) {
                free(left);
                return 0;
            }
            wchar_t* format = L"%ls\tpopq %%rax\n\tcmpq $1, %%rax\n\tjne %ls\n%ls%ls:\n";
            wchar_t* label = get_label(sections);
            size_t len = wcslen(left) + wcslen(block) + wcslen(label) * 2 + wcslen(format);
            wchar_t* instr = (wchar_t*) calloc(len + 1, sizeof(wchar_t));
            swprintf(instr, len, format, left, label, block, label);
            free(left);
            free(block);
            free(label);
            return instr;
        }

        case TOK_WHILE: {
            wchar_t* left = _compile(ast->left->list[0], scope, sections);
            if (err_status != ERR_NONE) {
                return 0;
            }
            wchar_t* block = _compile(ast->right, scope, sections);
            if (err_status != ERR_NONE) {
                free(left);
                return 0;
            }
            wchar_t* format = L"%ls:\n%ls\tpopq %%rax\n\tcmpq $1, %%rax\n\tjne %ls\n%ls\tjmp %ls\n%ls:\n";
            wchar_t* l1 = get_label(sections);
            wchar_t* l2 = get_label(sections);
            size_t len = wcslen(format) + wcslen(left) + wcslen(block) + wcslen(l1) * 2 + wcslen(l2) * 2;
            wchar_t* instr = (wchar_t*) calloc(len + 1, sizeof(wchar_t));
            swprintf(instr, len, format, l1, left, l2, block, l1, l2);
            free(left);
            free(block);
            free(l1);
            free(l2);
            return instr;
        }

        case TOK_FOR: {
            wchar_t* start = _compile(ast->list[1]->list[0], scope, sections);
            if (err_status != ERR_NONE) {
                return 0;
            }
            wchar_t* end = _compile(ast->list[1]->list[1], scope, sections);
            if (err_status != ERR_NONE) {
                free(start);
                return 0;
            }
            wchar_t* step = _compile(ast->list[1]->list[2], scope, sections);
            if (err_status != ERR_NONE) {
                free(start);
                free(end);
                return 0;
            }

            wchar_t* block = _compile(ast->list[2], scope, sections);
            if (err_status != ERR_NONE) {
                free(start);
                free(end);
                free(step);
                return 0;
            }
            
            wchar_t* left = _compile(ast->list[0], scope, sections);
            if (err_status != ERR_NONE) {
                free(start);
                free(end);
                free(step);
                free(block);
                return 0;
            }
            scope->types[extract_offset(left) / 8 - 1] = VAR_INT;

            wchar_t* format = L"%ls\tpopq %ls\n%ls:\n%ls\tpopq %%rax\n\tcmpq %%rax, %ls\n\t%ls %ls\n%ls%ls\tpopq %%rax\n\taddq %%rax, %ls\n\tjmp %ls\n%ls:\n";
            wchar_t* jmp = (ast->list[1]->list[2]->tok->type == TOK_MINUS)? L"jl" : L"jg";
            wchar_t* l1 = get_label(sections);
            wchar_t* l2 = get_label(sections);
            size_t len = wcslen(format) + wcslen(left) * 3 + wcslen(block) + wcslen(start) + wcslen(end) + wcslen(step) + wcslen(l1) * 2 + wcslen(l2) * 2 + 2;
            wchar_t* instr = (wchar_t*) calloc(len + 1, sizeof(wchar_t));
            swprintf(instr, len, format, start, left, l1, end, left, jmp, l2, block, step, left, l1, l2);

            free(left);
            free(start);
            free(end);
            free(step);
            free(block);
            free(l1);
            free(l2);
            return instr;
        }

        case TOK_FUNC: {
            // wprintf(L"%ls %d\n", tok_type_to_str(ast->tok->type), ast->size);
            // for (int i = 0; i < ast->size; i++) {
            //     wprintf(L"%ls\n", tok_type_to_str(ast->list[i]->tok->type));
            // }
            int offset = add_var(scope, ast->list[0]->tok->data);
            if (offset == -1) {
                wprintf(L"Error: Duplicate definition for `%ls`\n", ast->list[0]->tok->data);
                err_status = ERR_COMPILE;
                return 0;
            }
            scope_t* new_scope = init_scope(scope);
            ast_t* args = ast->list[1];
            for (int i = 0; i < args->size; i++) {
                if (args->list[i]->tok->type != TOK_ID || add_var(new_scope, args->list[i]->tok->data) == -1) {
                    wprintf(L"Error: Invalid function definition for `%ls`\n", ast->list[0]->tok->data);
                    err_status = ERR_COMPILE;
                    return 0;
                }
            }
            new_scope->cur_depth--;
            wchar_t* body = _compile(ast->list[2], new_scope, sections);
            if (err_status != ERR_NONE) {
                return 0;
            }
            new_scope->cur_depth++;
            wchar_t* pro = init_prologue(new_scope->size);
            wchar_t* epi = init_epilogue();
            size_t len = wcslen(pro) + wcslen(body) + wcslen(epi) + 24;
            wchar_t* func = (wchar_t*) calloc(len, sizeof(wchar_t));
            swprintf(func, len, L"func%d:\n%ls%ls%ls", offset, pro, body, epi);
            sections->funcs = (wchar_t*) realloc(sections->funcs, (wcslen(sections->funcs) + wcslen(func) + 1) * sizeof(wchar_t));
            wcscat(sections->funcs, func);
            free(pro);
            free(body);
            free(epi);
            free(func);
            return (wchar_t*) calloc(1, sizeof(wchar_t));
        }

        case TOK_LBRACE: {
            scope->cur_depth++;
            wchar_t* buf = (wchar_t*) calloc(1, sizeof(wchar_t));
            for (int i = 0; i < ast->size; i++) {
                wchar_t* instr = _compile(ast->list[i], scope, sections);
                if (err_status != ERR_NONE) {
                    free(buf);
                    return 0;
                }
                buf = (wchar_t*) realloc(buf, (wcslen(buf) + wcslen(instr) + 1) * sizeof(wchar_t));
                wcscat(buf, instr);
                free(instr);
            }
            scope->cur_depth--;
            return buf;
        }

        default: {
            wprintf(L"Error: Undefined compilation for `%ls`\n", tok_type_to_str(ast->tok->type));
            err_status = ERR_COMPILE;
            return 0;
        }
    }
}

wchar_t* _compile_binary_instr(ast_t* ast, scope_t* scope, sections_t* sections, wchar_t* op) {
    wchar_t* left = _compile(ast->left, scope, sections);
    if (err_status != ERR_NONE) {
        return 0;
    }
    wchar_t* right = _compile(ast->right, scope, sections);
    if (err_status != ERR_NONE) {
        free(left);
        return 0;
    }
    wchar_t* instr = (wchar_t*) calloc(wcslen(left) + wcslen(right) + wcslen(op) + 1, sizeof(wchar_t));
    wcscat(instr, left);
    wcscat(instr, right);
    wcscat(instr, op);
    free(left);
    free(right);
    return instr;
}

wchar_t* _compile_unary_instr(ast_t* ast, scope_t* scope, sections_t* sections, wchar_t* op) {
    wchar_t* left = _compile(ast->left, scope, sections);
    if (err_status != ERR_NONE) {
        return 0;
    }
    wchar_t* instr = (wchar_t*) calloc(wcslen(left) + wcslen(op) + 1, sizeof(wchar_t));
    wcscat(instr, left);
    wcscat(instr, op);
    free(left);
    return instr;
}

void compile(ast_t* root, sections_t* sections) {
    root->scope = init_scope(0);

    wchar_t* body = (wchar_t*) calloc(1, sizeof(wchar_t));
    for (int i = 0; i < root->size; i++) {
        wchar_t* instr = _compile(root->list[i], root->scope, sections);
        if (err_status != ERR_NONE) {
            free(body);
            return;
        }
        body = (wchar_t*) realloc(body, (wcslen(body) + wcslen(instr) + 1) * sizeof(wchar_t));
        wcscat(body, instr);
        free(instr);
    }
    
    wchar_t* pro = init_prologue(root->scope->size);
    wchar_t* epi = init_epilogue();
    sections->text = (wchar_t*) realloc(sections->text, (wcslen(sections->text) + wcslen(pro) + wcslen(body) + wcslen(epi) + 1) * sizeof(wchar_t));
    wcscat(sections->text, pro);
    wcscat(sections->text, body);
    wcscat(sections->text, epi);
    free(pro);
    free(body);
    free(epi);
}