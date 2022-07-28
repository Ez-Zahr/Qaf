#include "../include/lexer.h"

void init_lexer(lexer_t* lexer) {
    lexer->cap = 64;
    lexer->size = 0;
    lexer->pos = 0;
    lexer->tokens = (token_t*) calloc(lexer->cap, sizeof(token_t));

    int i;
    for (i = 0; i < lexer->cap; i++) {
        lexer->tokens[i].len = 0;
        lexer->tokens[i].data = (wchar_t*) calloc(1, sizeof(wchar_t));
    }
}

int isaralpha(wchar_t c) {
    return (0x600 <= c && c <= 0x6ff && c != L'؛');
}

tok_type_t get_keyword_type(wchar_t* keyword)
{
    if (!wcscmp(keyword, L"اطبع")) {
        return TOK_PRINT;
    } else if (!wcscmp(keyword, L"صح")) {
        return TOK_TRUE;
    } else if (!wcscmp(keyword, L"خطأ")) {
        return TOK_FALSE;
    } else if (!wcscmp(keyword, L"أو")) {
        return TOK_OR;
    } else if (!wcscmp(keyword, L"و")) {
        return TOK_AND;
    } else if (!wcscmp(keyword, L"ليس")) {
        return TOK_NOT;
    }

    return TOK_ID;
}

int isop(wchar_t c) {
    return (c == L'=' || c == L'+' || c == L'-' || c == L'*' || c == L'/' || c == L'<' || c == L'>');
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
    } else if (!wcscmp(op, L"==")) {
        return TOK_EQ;
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
        exit(1);
    }
}

void skip_whitespace(src_t* src) {
    while (isspace(src->buf[src->pos])) {
        src->pos++;
    }
}

void lex(src_t* src, lexer_t* lexer) {
    while (1) {
        skip_whitespace(src);

        if (src->buf[src->pos] == L'\0') {
            break;
        }

        if (isaralpha(src->buf[src->pos]) || src->buf[src->pos] == L'_') {
            token_t* tok = &lexer->tokens[lexer->size++];
            do {
                tok->data[tok->len++] = src->buf[src->pos++];
                tok->data = (wchar_t*) realloc(tok->data, (tok->len + 1) * sizeof(wchar_t));
            } while (isaralpha(src->buf[src->pos]) || src->buf[src->pos] == L'_' || isdigit(src->buf[src->pos]));
            tok->data[tok->len] = L'\0';
            tok->type = get_keyword_type(tok->data);

        } else if (isdigit(src->buf[src->pos])) {
            token_t* tok = &lexer->tokens[lexer->size++];
            tok->type = TOK_INT;
            int period = 0;
            do {
                if (src->buf[src->pos] == L'.') {
                    if (period) {
                        wprintf(L"Error: Invalid number format\n");
                        exit(1);
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
            do {
                tok->data[tok->len++] = src->buf[src->pos++];
                tok->data = (wchar_t*) realloc(tok->data, (tok->len + 1) * sizeof(wchar_t));
            } while (isop(src->buf[src->pos]));
            tok->data[tok->len] = L'\0';
            tok->type = get_op_type(tok->data);

        } else if (src->buf[src->pos] == L'؛') {
            token_t* tok = &lexer->tokens[lexer->size++];
            tok->type = TOK_SEMI;
            tok->len = 1;
            tok->data = (wchar_t*) realloc(tok->data, 2 * sizeof(wchar_t));
            tok->data[0] = L'؛';
            tok->data[1] = L'\0';
            src->pos++;

        } else {
            wprintf(L"Error: Undefined symbol `%lc` at %d\n", src->buf[src->pos], src->pos);
            exit(1);
        }

        if (lexer->size >= lexer->cap) {
            lexer->cap *= 2;
            lexer->tokens = (token_t*) realloc(lexer->tokens, lexer->cap * sizeof(token_t));
        }
    }

    token_t* tok = &lexer->tokens[lexer->size++];
    tok->type = TOK_EOF;
}

wchar_t* tok_type_to_str(tok_type_t type) {
    switch (type) {
        case TOK_ID:
            return L"TOK_ID";
        case TOK_PRINT:
            return L"TOK_PRINT";
        case TOK_INT:
            return L"TOK_INT";
        case TOK_FLOAT:
            return L"TOK_FLOAT";
        case TOK_TRUE:
            return L"TOK_TRUE";
        case TOK_FALSE:
            return L"TOK_FALSE";
        case TOK_ASSIGN:
            return L"TOK_ASSIGN";
        case TOK_PLUS:
            return L"TOK_PLUS";
        case TOK_MINUS:
            return L"TOK_MINUS";
        case TOK_MUL:
            return L"TOK_MUL";
        case TOK_DIV:
            return L"TOK_DIV";
        case TOK_EQ:
            return L"TOK_EQ";
        case TOK_LT:
            return L"TOK_LT";
        case TOK_LTE:
            return L"TOK_LTE";
        case TOK_GT:
            return L"TOK_GT";
        case TOK_GTE:
            return L"TOK_GTE";
        case TOK_OR:
            return L"TOK_OR";
        case TOK_AND:
            return L"TOK_AND";
        case TOK_NOT:
            return L"TOK_NOT";
        case TOK_SEMI:
            return L"TOK_SEMI";
        case TOK_EOF:
            return L"TOK_EOF";
    }
}

void print_tokens(lexer_t *lexer) {
    int i;
    for (i = 0; i < lexer->size; i++) {
        wprintf(L"`%ls`, len %d, type `%ls`\n", lexer->tokens[i].data, lexer->tokens[i].len, tok_type_to_str(lexer->tokens[i].type));
    }
}

void free_lexer(lexer_t *lexer) {
    int i;
    for (i = 0; i < lexer->cap; i++) {
        free(lexer->tokens[i].data);
    }
    free(lexer->tokens);
}