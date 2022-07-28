#include "../include/util.h"
#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/evaluator.h"

int main(int argc, char* argv[]) {
    if (setlocale(LC_ALL, "ar_SA.utf8") == NULL) {
        wprintf(L"Failed to set locale\n");
        return 1;
    }

    if (argc < 2) {
        wprintf(L"Please specify a .qaf input file (./qaf <filename>.qaf)\n");
        return 1;
    }

    src_t src;
    init_src(&src);
    lexer_t lexer;
    init_lexer(&lexer);
    parser_t parser;
    init_parser(&parser);
    vars_t vars;
    init_vars(&vars);
    
    read_src(argv[1], &src);
    lex(&src, &lexer);
    // print_tokens(&lexer);

    while (1) {
        if (lexer.tokens[lexer.pos].type == TOK_SEMI) {
            lexer.pos++;
            continue;
        } else if (lexer.tokens[lexer.pos].type == TOK_EOF) {
            break;
        }
        
        parser.parseTrees[parser.size++] = parse(&lexer);
        if (parser.size >= parser.cap) {
            parser.cap *= 2;
            parser.parseTrees = (node_t**) realloc(parser.parseTrees, parser.cap * sizeof(node_t*));
        }
    }
    // print_parser(&parser);

    // eval(&parser, &vars);
    // print_vars(&vars); wprintf(L"\n");

    free_src(&src);
    free_lexer(&lexer);
    free_parser(&parser);
    free_vars(&vars);
    return 0;
}