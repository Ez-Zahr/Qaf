#include "../include/lexer.h"

void init_lexer(lexer_t* lexer) {
    lexer->cap = 64;
    lexer->size = 0;
    lexer->pos = 0;
    
    lexer->tokens = (token_t*) calloc(lexer->cap, sizeof(token_t));
    if (lexer->tokens == NULL) {
        wprintf(L"Failed to initialize lexer\n");
        exit(1);
    }

    int i;
    for (i = 0; i < lexer->cap; i++) {
        lexer->tokens[i].len = 0;
        lexer->tokens[i].data = (wchar_t*) calloc(1, sizeof(wchar_t));
        if (lexer->tokens[i].data == NULL) {
            wprintf(L"Failed to initialize token's data\n");
            exit(1);
        }
    }
}

tok_type_t get_keyword_type(wchar_t* keyword) {
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

tok_type_t get_op_type(wchar_t op) {
    switch (op)
    {
    case L'=':
        return TOK_ASSIGN;
        break;
    case L'+':
        return TOK_PLUS;
        break;
    case L'-':
        return TOK_MINUS;
        break;
    case L'*':
        return TOK_MUL;
        break;
    case L'/':
        return TOK_DIV;
        break;
    default:
        break;
    }
}

void lex(src_t* src, lexer_t* lexer) {
    while (src->pos < src->size) {
        if (isaralpha(src->buf[src->pos]) || src->buf[src->pos] == L'_') {
            token_t* tok = &lexer->tokens[lexer->size++];
            while (isaralpha(src->buf[src->pos]) || src->buf[src->pos] == L'_' || isdigit(src->buf[src->pos]))  {
                tok->data[tok->len++] = src->buf[src->pos++];
                tok->data = (wchar_t*) realloc(tok->data, (tok->len + 1) * sizeof(wchar_t));
                if (tok->data == NULL) {
                    wprintf(L"Failed to resize data\n");
                    exit(1);
                }
            }
            tok->data[tok->len] = L'\0';
            tok->type = get_keyword_type(tok->data);

        } else if (isdigit(src->buf[src->pos]) || src->buf[src->pos] == L'.') {
            token_t* tok = &lexer->tokens[lexer->size++];
            tok->type = TOK_NUM;
            int period = 0;
            while (isdigit(src->buf[src->pos]) || src->buf[src->pos] == L'.') {
                if (src->buf[src->pos] == L'.') {
                    if (period) {
                        wprintf(L"Error: Invalid number format\n");
                        exit(1);
                    } else {
                        period = 1;
                    }
                }
                tok->data[tok->len++] = src->buf[src->pos++];
                tok->data = (wchar_t*) realloc(tok->data, (tok->len + 1) * sizeof(wchar_t));
                if (tok->data == NULL) {
                    wprintf(L"Failed to resize data\n");
                    exit(1);
                }
            }
            tok->data[tok->len] = L'\0';

        } else if (isop(src->buf[src->pos])) {
            token_t* tok = &lexer->tokens[lexer->size++];
            tok->type = get_op_type(src->buf[src->pos]);
            while (isop(src->buf[src->pos])) {
                tok->data[tok->len++] = src->buf[src->pos++];
                tok->data = (wchar_t*) realloc(tok->data, (tok->len + 1) * sizeof(wchar_t));
                if (tok->data == NULL) {
                    wprintf(L"Failed to resize data\n");
                    exit(1);
                }
            }
            tok->data[tok->len] = L'\0';
            
        } else if (src->buf[src->pos] == L'\n') {
            token_t* tok = &lexer->tokens[lexer->size++];
            tok->type = TOK_NEWLINE;
            tok->len = 1;
            tok->data = (wchar_t*) realloc(tok->data, 2 * sizeof(wchar_t));
            if (tok->data == NULL) {
                wprintf(L"Failed to resize data\n");
                exit(1);
            }
            tok->data[0] = L'\n';
            tok->data[1] = L'\0';
            src->pos++;
        } else {
            src->pos++;
        }

        if (lexer->size >= lexer->cap) {
            lexer->cap *= 2;
            lexer->tokens = (token_t*) realloc(lexer->tokens, lexer->cap * sizeof(token_t));
            if (lexer->tokens == NULL) {
                wprintf(L"Failed to resize tokens\n");
                exit(1);
            }
        }
    }

    token_t* tok = &lexer->tokens[lexer->size++];
    tok->type = TOK_EOF;
}

void print_tokens(lexer_t* lexer) {
    int i;
    for (i = 0; i < lexer->size; i++) {
        wprintf(L"`%ls` of length %d type %d\n", lexer->tokens[i].data, lexer->tokens[i].len, lexer->tokens[i].type);
    }
}

void free_lexer(lexer_t* lexer) {
    int i;
    for (i = 0; i < lexer->cap; i++) {
        free(lexer->tokens[i].data);
    }
    free(lexer->tokens);
}