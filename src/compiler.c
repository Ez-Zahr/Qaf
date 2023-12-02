#include "../include/compiler.h"

wchar_t* init_prologue() {
    return L"\tpushq %rbp\n\tmovq %rsp, %rbp\n";
}

wchar_t* init_epilogue() {
    return L"\tleaveq\n\tretq\n";
}

wchar_t* get_label(sections_t* sections) {
    wchar_t* label = (wchar_t*) smart_alloc(8, sizeof(wchar_t));
    swprintf(label, 7, L".L%d", sections->labels++);
    return label;
}

long extract_offset(wchar_t* offset) {
    wchar_t* start = wcschr(offset, L'-') + 1;
    wchar_t* end = wcschr(start, L'(');
    return wcstol(start, &end, 10);
}

wchar_t* _compile_binary_instr(ast_t* ast, scope_t* scope, sections_t* sections, wchar_t* op);
wchar_t* _compile_unary_instr(ast_t* ast, scope_t* scope, sections_t* sections, wchar_t* op);

wchar_t* _compile_ast(ast_t* ast, scope_t* scope, sections_t* sections) {
    switch (ast->tok->type) {
        case TOK_PRINT: {
            wchar_t* left = _compile_ast(ast->left, scope, sections);
            size_t len = wcslen(left) + 36;
            wchar_t* instr = (wchar_t*) smart_alloc(len, sizeof(wchar_t));
            switch (get_var_type(scope, extract_offset(left) / 8 - 1)) {
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
                    smart_exit(ERR_COMPILE);
                }
            }
            return instr;
        }

        case TOK_READ: {
            int bufnum = sections->labels++;
            wchar_t buf[24];
            swprintf(buf, 24, L"\tbuf%d: .skip 80\n", bufnum);
            sections->bss = (wchar_t*) smart_realloc(sections->bss, wcslen(sections->bss) + wcslen(buf) + 1, sizeof(wchar_t));
            wcscat(sections->bss, buf);
            wchar_t* instr = (wchar_t*) smart_alloc(64, sizeof(wchar_t));
            swprintf(instr, 64, L"\tmovq $buf%d, %%r8\n\tcallq read_line\n\tpushq $buf%d\n", bufnum, bufnum);
            return instr;
        }

        case TOK_LET: {
            int offset = add_var(scope, ast->left->tok->data, scope->depth);
            if (offset == -1) {
                wprintf(L"Error: Duplicate definition for `%ls`\n", ast->left->tok->data);
                smart_exit(ERR_COMPILE);
            }
            wchar_t* instr = (wchar_t*) smart_alloc(24, sizeof(wchar_t));
            swprintf(instr, 24, L"\tpushq -%d(%%rbp)\n", (offset + 1) * 8);
            return instr;
        }

        case TOK_ID: {
            if (ast->left) {
                int offset = get_func_offset(scope, ast->tok->data);
                if (offset == -1 || ast->left->size != get_func_argc(scope, offset)) {
                    wprintf(L"Error: Invalid function call `%ls`\n", ast->tok->data);
                    smart_exit(ERR_COMPILE);
                }
                wchar_t* args = _compile_ast(ast->left, scope, sections);
                size_t len = wcslen(args) + 48;
                wchar_t* instr = (wchar_t*) smart_alloc(len, sizeof(wchar_t));
                swprintf(instr, len, L"%ls\tcallq func%d\n\taddq $%d, %%rsp\n", args, offset, ast->left->size * 8);
                return instr;
            } else {
                int offset = get_var_offset(scope, ast->tok->data);
                if (offset == -1) {
                    wprintf(L"Error: Undefined reference `%ls`\n", ast->tok->data);
                    smart_exit(ERR_COMPILE);
                }
                wchar_t* instr = (wchar_t*) smart_alloc(24, sizeof(wchar_t));
                swprintf(instr, 24, L"\tpushq -%d(%%rbp)\n", (offset + 1) * 8);
                return instr;
            }
        }

        case TOK_INT: {
            int len = ast->tok->len + 16;
            wchar_t* instr = (wchar_t*) smart_alloc(len, sizeof(wchar_t));
            swprintf(instr, len, L"\tpushq $%ls\n", ast->tok->data);
            return instr;
        }

        case TOK_BOOL: {
            wchar_t* instr = (wchar_t*) smart_alloc(16, sizeof(wchar_t));
            swprintf(instr, 16, L"\tpushq $%d\n", !wcscmp(ast->tok->data, L"صح"));
            return instr;
        }

        case TOK_CHAR: {
            wchar_t* instr = (wchar_t*) smart_alloc(24, sizeof(wchar_t));
            swprintf(instr, 24, L"\tpushq $0x%x\n", ast->tok->data[0]);
            return instr;
        }

        case TOK_STR: {
            int strnum = sections->labels++;
            size_t len = wcslen(ast->tok->data) + 32;
            wchar_t buf[len];
            swprintf(buf, len, L"\tstr%d: .string \"%ls\"\n", strnum, ast->tok->data);
            sections->rodata = (wchar_t*) smart_realloc(sections->rodata, wcslen(sections->rodata) + wcslen(buf) + 1, sizeof(wchar_t));
            wcscat(sections->rodata, buf);
            wchar_t* instr = (wchar_t*) smart_alloc(20, sizeof(wchar_t));
            swprintf(instr, 20, L"\tpushq $str%d\n", strnum);
            return instr;
        }

        case TOK_ASSIGN: {
            wchar_t* left = _compile_ast(ast->left, scope, sections);
            wchar_t* right = _compile_ast(ast->right, scope, sections);
            set_var_type(scope, extract_offset(left) / 8 - 1, VAR_INT);
            size_t len = wcslen(left) + wcslen(right) + 16;
            wchar_t* instr = (wchar_t*) smart_alloc(len, sizeof(wchar_t));
            swprintf(instr, len, L"%ls\tpopq %ls", right, wcschr(left, L'-'));
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
            wchar_t* left = _compile_ast(ast->left->list[0], scope, sections);
            wchar_t* block = _compile_ast(ast->right, scope, sections);
            wchar_t* format = L"%ls\tpopq %%rax\n\tcmpq $1, %%rax\n\tjne %ls\n%ls%ls:\n";
            wchar_t* label = get_label(sections);
            size_t len = wcslen(left) + wcslen(block) + wcslen(label) * 2 + wcslen(format);
            wchar_t* instr = (wchar_t*) smart_alloc(len + 1, sizeof(wchar_t));
            swprintf(instr, len, format, left, label, block, label);
            return instr;
        }

        case TOK_WHILE: {
            wchar_t* left = _compile_ast(ast->left->list[0], scope, sections);
            wchar_t* block = _compile_ast(ast->right, scope, sections);
            wchar_t* format = L"%ls:\n%ls\tpopq %%rax\n\tcmpq $1, %%rax\n\tjne %ls\n%ls\tjmp %ls\n%ls:\n";
            wchar_t* l1 = get_label(sections);
            wchar_t* l2 = get_label(sections);
            size_t len = wcslen(format) + wcslen(left) + wcslen(block) + wcslen(l1) * 2 + wcslen(l2) * 2;
            wchar_t* instr = (wchar_t*) smart_alloc(len + 1, sizeof(wchar_t));
            swprintf(instr, len, format, l1, left, l2, block, l1, l2);
            return instr;
        }

        case TOK_FOR: {
            int offset = add_var(scope, ast->list[0]->tok->data, scope->depth + 1);
            set_var_type(scope, offset, VAR_INT);
            offset = (offset + 1) * 8;
            wchar_t* start = _compile_ast(ast->list[1]->list[0], scope, sections);
            wchar_t* end = _compile_ast(ast->list[1]->list[1], scope, sections);
            wchar_t* step = _compile_ast(ast->list[1]->list[2], scope, sections);
            wchar_t* block = _compile_ast(ast->list[2], scope, sections);
            wchar_t* format = L"%ls\tpopq -%d(%%rbp)\n%ls:\n%ls\tpopq %%rax\n\tcmpq %%rax, -%d(%%rbp)\n\t%ls %ls\n%ls%ls\tpopq %%rax\n\taddq %%rax, -%d(%%rbp)\n\tjmp %ls\n%ls:\n";
            wchar_t* jmp = (ast->list[1]->list[2]->tok->type == TOK_MINUS)? L"jle" : L"jge";
            wchar_t* l1 = get_label(sections);
            wchar_t* l2 = get_label(sections);
            size_t len = wcslen(format) + wcslen(block) + wcslen(start) + wcslen(end) + wcslen(step) + wcslen(l1) * 2 + wcslen(l2) * 2 + 16;
            wchar_t* instr = (wchar_t*) smart_alloc(len + 1, sizeof(wchar_t));
            swprintf(instr, len, format, start, offset, l1, end, offset, jmp, l2, block, step, offset, l1, l2);
            return instr;
        }

        case TOK_FUNC: {
            int offset = add_func(scope, ast->list[0]->tok->data);
            if (offset == -1) {
                wprintf(L"Error: Duplicate definition for `%ls`\n", ast->list[0]->tok->data);
                smart_exit(ERR_COMPILE);
            }
            ast_t* args = ast->list[1];
            set_func_argc(scope, offset, args->size);
            ast_t* block = ast->list[2];
            block->scope = init_scope(scope);
            wchar_t* func_args = (wchar_t*) smart_alloc(1, sizeof(wchar_t));
            for (int i = 0; i < args->size; i++) {
                if (args->list[i]->tok->type != TOK_ID || add_arg(block->scope, args->list[i]->tok->data) == -1) {
                    wprintf(L"Error: Invalid function definition for `%ls`\n", ast->list[0]->tok->data);
                    smart_exit(ERR_COMPILE);
                }
                wchar_t arg_init[64];
                swprintf(arg_init, 64, L"\tpushq %d(%%rbp)\n", i * 8 + 16);
                func_args = (wchar_t*) smart_realloc(func_args, wcslen(func_args) + 64, sizeof(wchar_t));
                wcscat(func_args, arg_init);
            }
            wchar_t* body = (wchar_t*) smart_alloc(1, sizeof(wchar_t));
            for (int i = 0; i < block->size; i++) {
                wchar_t* instr = _compile_ast(block->list[i], block->scope, sections);
                body = (wchar_t*) smart_realloc(body, wcslen(body) + wcslen(instr) + 1, sizeof(wchar_t));
                wcscat(body, instr);
            }
            wchar_t* pro = init_prologue();
            wchar_t* epi = init_epilogue();
            size_t len = wcslen(pro) + wcslen(func_args) + wcslen(body) + wcslen(epi) + 48;
            wchar_t* func = (wchar_t*) smart_alloc(len, sizeof(wchar_t));
            swprintf(func, len, L"func%d:\n%ls%ls\tsubq $%d, %%rsp\n%ls%ls", offset, pro, func_args, block->scope->vars_size * 8, body, epi);
            sections->funcs = (wchar_t*) smart_realloc(sections->funcs, wcslen(sections->funcs) + wcslen(func) + 1, sizeof(wchar_t));
            wcscat(sections->funcs, func);
            return (wchar_t*) smart_alloc(1, sizeof(wchar_t));
        }

        case TOK_LBRACE: {
            push_depth(scope);
            wchar_t* block = (wchar_t*) smart_alloc(1, sizeof(wchar_t));
            for (int i = 0; i < ast->size; i++) {
                wchar_t* instr = _compile_ast(ast->list[i], scope, sections);
                block = (wchar_t*) smart_realloc(block, wcslen(block) + wcslen(instr) + 1, sizeof(wchar_t));
                wcscat(block, instr);
            }
            pop_depth(scope);
            return block;
        }

        case TOK_LPAREN: {
            wchar_t* args = (wchar_t*) smart_alloc(1, sizeof(wchar_t));
            for (int i = ast->size - 1; i >= 0; i--) {
                wchar_t* arg = _compile_ast(ast->list[i], scope, sections);
                args = (wchar_t*) smart_realloc(args, wcslen(args) + wcslen(arg) + 1, sizeof(wchar_t));
                wcscat(args, arg);
            }
            return args;
        }

        default: {
            wprintf(L"Error: Undefined compilation for `%ls`\n", tok_type_to_str(ast->tok->type));
            smart_exit(ERR_COMPILE);
        }
    }

    return NULL;
}

wchar_t* _compile_binary_instr(ast_t* ast, scope_t* scope, sections_t* sections, wchar_t* op) {
    wchar_t* left = _compile_ast(ast->left, scope, sections);
    wchar_t* right = _compile_ast(ast->right, scope, sections);
    wchar_t* instr = (wchar_t*) smart_alloc(wcslen(left) + wcslen(right) + wcslen(op) + 1, sizeof(wchar_t));
    wcscat(instr, left);
    wcscat(instr, right);
    wcscat(instr, op);
    return instr;
}

wchar_t* _compile_unary_instr(ast_t* ast, scope_t* scope, sections_t* sections, wchar_t* op) {
    wchar_t* left = _compile_ast(ast->left, scope, sections);
    wchar_t* instr = (wchar_t*) smart_alloc(wcslen(left) + wcslen(op) + 1, sizeof(wchar_t));
    wcscat(instr, left);
    wcscat(instr, op);
    return instr;
}

void compile(ast_t* root, sections_t* sections) {
    root->scope = init_scope(0);
    wchar_t* body = (wchar_t*) smart_alloc(1, sizeof(wchar_t));
    for (int i = 0; i < root->size; i++) {
        wchar_t* instr = _compile_ast(root->list[i], root->scope, sections);
        body = (wchar_t*) smart_realloc(body, wcslen(body) + wcslen(instr) + 1, sizeof(wchar_t));
        wcscat(body, instr);
    }
    wchar_t* pro = init_prologue();
    wchar_t* epi = init_epilogue();
    size_t len = wcslen(pro) + wcslen(body) + wcslen(epi) + 32;
    wchar_t buf[len];
    swprintf(buf, len, L"%ls\tsubq $%d, %%rsp\n%ls%ls", pro, root->scope->vars_size * 8, body, epi);
    sections->text = (wchar_t*) smart_realloc(sections->text, wcslen(sections->text) + wcslen(buf) + 1, sizeof(wchar_t));
    wcscat(sections->text, buf);
}