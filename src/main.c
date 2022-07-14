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
        wprintf(L"Please specify a source file (./qaf <filename>)\n");
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
        if (lexer.tokens[lexer.pos].type == TOK_NEWLINE) {
            lexer.pos++;
            continue;
        } else if (lexer.tokens[lexer.pos].type == TOK_EOF) {
            break;
        }
        
        parse(&lexer, &parser);
        // print_tree(parser.parseTree, 0);
        
        eval(&parser, &vars);
        // print_vars(&vars); wprintf(L"\n");
        
        free_parser(&parser);
    }

    free_src(&src);
    free_lexer(&lexer);
    free_parser(&parser);
    free_vars(&vars);
    return 0;
}