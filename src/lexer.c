#include "../include/lexer.h"

void init_lexer(lexer_t* lexer) {
    lexer->cap = 64;
    lexer->size = 0;
    lexer->pos = 0;
    
    lexer->tokens = (token_t*) calloc(lexer->cap, sizeof(token_t));
    if (lexer->tokens == NULL) {
        printf("Failed to initialize lexer\n");
        exit(1);
    }

    int i;
    for (i = 0; i < lexer->cap; i++) {
        lexer->tokens[i].len = 1;
        lexer->tokens[i].data = (wchar_t*) calloc(lexer->tokens[i].len, sizeof(wchar_t));
        if (lexer->tokens[i].data == NULL) {
            printf("Failed to initialize token's data\n");
            exit(1);
        }
    }
}

token_t* peek(lexer_t* lexer, int offset) {
    if (lexer->pos + offset >= lexer->size) {
        return NULL;
    }

    return &lexer->tokens[lexer->pos + offset];
}

void add_tok(lexer_t* lexer, src_t* src, tok_type_t type) {
    token_t* tok = &lexer->tokens[lexer->size++];
    tok->type = type;
    
    do {
        tok->data[tok->len-1] = src->buf[src->pos++];
        tok->len++;
        
        tok->data = (wchar_t*) realloc(tok->data, tok->len * sizeof(wchar_t));
        if (tok->data == NULL) {
            printf("Failed to resize data\n");
            exit(1);
        }
    } while (!isspace(src->buf[src->pos]) && src->buf[src->pos] != L'\0');
    
    tok->data[tok->len] = L'\0';
    
    if (lexer->size >= lexer->cap) {
        lexer->cap *= 2;
        lexer->tokens = (token_t*) realloc(lexer->tokens, lexer->cap * sizeof(token_t));
        if (lexer->tokens == NULL) {
            printf("Failed to resize tokens array\n");
            exit(1);
        }
    }
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
        if (isaralpha(src->buf[src->pos])) {
            add_tok(lexer, src, TOK_ID);
        } else if (isdigit(src->buf[src->pos])) {
            add_tok(lexer, src, TOK_NUM);
        } else if (isop(src->buf[src->pos])) {
            add_tok(lexer, src, get_op_type(src->buf[src->pos]));
        }

        src->pos++;
    }
}

void free_lexer(lexer_t* lexer) {
    int i;
    for (i = 0; i < lexer->cap; i++) {
        free(lexer->tokens[i].data);
    }
    free(lexer->tokens);
}