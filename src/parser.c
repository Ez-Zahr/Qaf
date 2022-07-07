#include "../include/parser.h"

void init_parser(parser_t* parser) {
}

void parse(lexer_t* lexer, parser_t* parser) {
    // if (lexer->tokens[lexer->pos].type == TOK_ID) {
    // }
    parse_term(lexer, parser);
}

void parse_term(lexer_t* lexer, parser_t* parser) {
    parse_factor(lexer, parser);
}

void parse_factor(lexer_t* lexer, parser_t* parser) {
    parse_primary_expr(lexer, parser);
}

void parse_primary_expr(lexer_t* lexer, parser_t* parser) {
    if (lexer->tokens[lexer->pos].type == TOK_NUM) {
        
    }
}