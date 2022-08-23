#include "../include/lexer.h"

extern ERROR_STATUS err_status;

lexer_t* init_lexer() {
    lexer_t* lexer = (lexer_t*) calloc(1, sizeof(lexer_t));
    lexer->tokens = (token_t*) calloc(1, sizeof(token_t));
    lexer->size = 0;
    lexer->pos = 0;
    return lexer;
}

int isaralpha(wchar_t c) {
    return (0x600 <= c && c <= 0x6ff && c != L'؛');
}

tok_type_t get_keyword_type(wchar_t* keyword) {
    if (!wcscmp(keyword, L"اطبع")) {
        return TOK_PRINT;
    } else if (!wcscmp(keyword, L"اقرأ_سطر")) {
        return TOK_READ;
    } else if (!wcscmp(keyword, L"صح") || !wcscmp(keyword, L"خطأ")) {
        return TOK_BOOL;
    } else if (!wcscmp(keyword, L"أو")) {
        return TOK_OR;
    } else if (!wcscmp(keyword, L"و")) {
        return TOK_AND;
    } else if (!wcscmp(keyword, L"ليس")) {
        return TOK_NOT;
    } else if (!wcscmp(keyword, L"دع")) {
        return TOK_LET;
    } else if (!wcscmp(keyword, L"إذا")) {
        return TOK_IF;
    } else if (!wcscmp(keyword, L"لكل")) {
        return TOK_FOR;
    } else if (!wcscmp(keyword, L"طالما")) {
        return TOK_WHILE;
    } else if (!wcscmp(keyword, L"في")) {
        return TOK_IN;
    }
    return TOK_ID;
}

int isop(wchar_t c) {
    switch (c) {
        case L'=': case L'+': case L'-': case L'*': case L'/': case L'%': case L'<': case L'>': case L'!': return 1; default: return 0;
    }
}

tok_type_t get_op_type(wchar_t* op) {
    if (!wcscmp(op, L"=")) {
        return TOK_ASSIGN;
    } else if (!wcscmp(op, L"+")) {
        return TOK_PLUS;
    } else if (!wcscmp(op, L"-")) {
        return TOK_MINUS;
    } else if (!wcscmp(op, L"*")) {
        return TOK_MUL;
    } else if (!wcscmp(op, L"/")) {
        return TOK_DIV;
    } else if (!wcscmp(op, L"%")) {
        return TOK_MOD;
    } else if (!wcscmp(op, L"==")) {
        return TOK_EQ;
    } else if (!wcscmp(op, L"!=")) {
        return TOK_NE;
    } else if (!wcscmp(op, L"<")) {
        return TOK_LT;
    } else if (!wcscmp(op, L"<=")) {
        return TOK_LTE;
    } else if (!wcscmp(op, L">")) {
        return TOK_GT;
    } else if (!wcscmp(op, L">=")) {
        return TOK_GTE;
    } else {
        wprintf(L"Error: Undefined operator `%ls`\n", op);
        err_status = ERR_LEX;
        return -1;
    }
}

int issymbol(wchar_t c) {
    switch (c) {
        case L'؛': case L'(': case L')': case L'{': case L'}': case L'[': case L']': case L':': return 1; default: return 0;
    }
}

tok_type_t get_symbol_type(wchar_t c) {
    switch (c) {
        case L'؛': return TOK_SEMI;
        case L'(': return TOK_LPAREN;
        case L')': return TOK_RPAREN;
        case L'{': return TOK_LBRACE;
        case L'}': return TOK_RBRACE;
        case L'[': return TOK_LBRACK;
        case L']': return TOK_RBRACK;
        case L':': return TOK_COLON;
        default: return -1;
    }
}

void skip_whitespace(src_t* src) {
    while (isspace(src->buf[src->pos]) && src->buf[src->pos] != L'\n') {
        src->pos++;
    }
}

void lex(src_t* src, lexer_t* lexer) {
    int line = 1;
    while (1) {
        skip_whitespace(src);

        if (src->buf[src->pos] == L'\0') {
            break;
        }

        if (isaralpha(src->buf[src->pos]) || src->buf[src->pos] == L'_') {
            token_t* tok = &lexer->tokens[lexer->size++];
            tok->len = 0;
            tok->data = (wchar_t*) calloc(1, sizeof(wchar_t));
            do {
                tok->data[tok->len++] = src->buf[src->pos++];
                tok->data = (wchar_t*) realloc(tok->data, (tok->len + 1) * sizeof(wchar_t));
            } while (isaralpha(src->buf[src->pos]) || src->buf[src->pos] == L'_' || isdigit(src->buf[src->pos]));
            tok->data[tok->len] = L'\0';
            tok->type = get_keyword_type(tok->data);

        } else if (isdigit(src->buf[src->pos])) {
            token_t* tok = &lexer->tokens[lexer->size++];
            tok->len = 0;
            tok->data = (wchar_t*) calloc(1, sizeof(wchar_t));
            tok->type = TOK_INT;
            int period = 0;
            do {
                if (src->buf[src->pos] == L'.') {
                    if (period) {
                        wprintf(L"Error: Invalid number format\n");
                        err_status = ERR_LEX;
                        return;
                    } else {
                        tok->type = TOK_FLOAT;
                        period = 1;
                    }
                }
                tok->data[tok->len++] = src->buf[src->pos++];
                tok->data = (wchar_t*) realloc(tok->data, (tok->len + 1) * sizeof(wchar_t));
            } while (isdigit(src->buf[src->pos]) || src->buf[src->pos] == L'.');
            tok->data[tok->len] = L'\0';

        } else if (isop(src->buf[src->pos])) {
            token_t* tok = &lexer->tokens[lexer->size++];
            tok->len = 0;
            tok->data = (wchar_t*) calloc(1, sizeof(wchar_t));
            do {
                tok->data[tok->len++] = src->buf[src->pos++];
                tok->data = (wchar_t*) realloc(tok->data, (tok->len + 1) * sizeof(wchar_t));
            } while (isop(src->buf[src->pos]));
            tok->data[tok->len] = L'\0';
            tok->type = get_op_type(tok->data);

        } else if (issymbol(src->buf[src->pos])) {
            token_t* tok = &lexer->tokens[lexer->size++];
            tok->type = get_symbol_type(src->buf[src->pos]);
            tok->len = 1;
            tok->data = (wchar_t*) calloc(2, sizeof(wchar_t));
            tok->data[0] = src->buf[src->pos];
            tok->data[1] = L'\0';
            src->pos++;

        } else if (src->buf[src->pos] == L'‘') {
            token_t* tok = &lexer->tokens[lexer->size++];
            tok->type = TOK_CHAR;
            src->pos++;
            tok->data = (wchar_t*) calloc(1, sizeof(wchar_t));
            tok->len = 0;
            if (src->buf[src->pos] != L'’' && src->buf[src->pos] != L'\0') {
                tok->data[tok->len++] = src->buf[src->pos++];
                tok->data = (wchar_t*) realloc(tok->data, (tok->len + 1) * sizeof(wchar_t));
            }
            tok->data[tok->len] = L'\0';
            if (src->buf[src->pos] != L'’') {
                wprintf(L"Error: Invalid character format\n");
                err_status = ERR_LEX;
                return;
            }
            src->pos++;

        } else if (src->buf[src->pos] == L'"') {
            token_t* tok = &lexer->tokens[lexer->size++];
            tok->type = TOK_STR;
            src->pos++;
            tok->data = (wchar_t*) calloc(1, sizeof(wchar_t));
            tok->len = 0;
            while (src->buf[src->pos] != L'"' && src->buf[src->pos] != L'\0') {
                tok->data[tok->len++] = src->buf[src->pos++];
                tok->data = (wchar_t*) realloc(tok->data, (tok->len + 1) * sizeof(wchar_t));
            }
            tok->data[tok->len] = L'\0';
            if (src->buf[src->pos] != L'"') {
                wprintf(L"Error: Missing closing double quote\n");
                err_status = ERR_LEX;
                return;
            }
            src->pos++;

        } else if (src->buf[src->pos] == L'#') {
            while (src->buf[src->pos] != L'\n' && src->buf[src->pos] != L'\0') {
                src->pos++;
            }
            continue;

        } else if (src->buf[src->pos] == L'\n') {
            line++;
            src->pos++;
            continue;

        } else {
            wprintf(L"Error: Undefined symbol `%lc` at %d:%d\n", src->buf[src->pos], line, src->pos + 1);
            err_status = ERR_LEX;
            return;
        }

        lexer->tokens = (token_t*) realloc(lexer->tokens, (lexer->size + 1) * sizeof(token_t));
    }

    token_t* tok = &lexer->tokens[lexer->size++];
    tok->data = (wchar_t*) calloc(1, sizeof(wchar_t));
    tok->type = TOK_EOF;
}

wchar_t* tok_type_to_str(tok_type_t type) {
    switch (type) {
        case TOK_PRINT: return L"TOK_PRINT";
        case TOK_READ: return L"TOK_READ";
        case TOK_LET: return L"TOK_LET";
        case TOK_ID: return L"TOK_ID";
        case TOK_INT: return L"TOK_INT";
        case TOK_FLOAT: return L"TOK_FLOAT";
        case TOK_BOOL: return L"TOK_BOOL";
        case TOK_CHAR: return L"TOK_CHAR";
        case TOK_STR: return L"TOK_STR";
        case TOK_ASSIGN: return L"TOK_ASSIGN";
        case TOK_PLUS: return L"TOK_PLUS";
        case TOK_MINUS: return L"TOK_MINUS";
        case TOK_MUL: return L"TOK_MUL";
        case TOK_DIV: return L"TOK_DIV";
        case TOK_MOD: return L"TOK_MOD";
        case TOK_AND: return L"TOK_AND";
        case TOK_OR: return L"TOK_OR";
        case TOK_NOT: return L"TOK_NOT";
        case TOK_EQ: return L"TOK_EQ";
        case TOK_NE: return L"TOK_NE";
        case TOK_LT: return L"TOK_LT";
        case TOK_LTE: return L"TOK_LTE";
        case TOK_GT: return L"TOK_GT";
        case TOK_GTE: return L"TOK_GTE";
        case TOK_IF: return L"TOK_IF";
        case TOK_FOR: return L"TOK_FOR";
        case TOK_WHILE: return L"TOK_WHILE";
        case TOK_IN: return L"TOK_IN";
        case TOK_LPAREN: return L"TOK_LPAREN";
        case TOK_RPAREN: return L"TOK_RPAREN";
        case TOK_LBRACE: return L"TOK_LBRACE";
        case TOK_RBRACE: return L"TOK_RBRACE";
        case TOK_LBRACK: return L"TOK_LBRACK";
        case TOK_RBRACK: return L"TOK_RBRACK";
        case TOK_COLON: return L"TOK_COLON";
        case TOK_SEMI: return L"TOK_SEMI";
        case TOK_EOF: return L"TOK_EOF";
        default: return L"Undefined type string";
    }
}

void print_tokens(lexer_t *lexer) {
    wprintf(L"----------<Tokens>----------\n");
    for (int i = 0; i < lexer->size; i++) {
        wprintf(L"`%ls`, len %d, type `%ls`\n", lexer->tokens[i].data, lexer->tokens[i].len, tok_type_to_str(lexer->tokens[i].type));
    }
    wprintf(L"----------------------------\n");
}

void free_lexer(lexer_t *lexer) {
    for (int i = 0; i < lexer->size; i++) {
        free(lexer->tokens[i].data);
    }
    free(lexer->tokens);
    free(lexer);
}