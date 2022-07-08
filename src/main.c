#include "../include/util.h"
#include "../include/lexer.h"
#include "../include/parser.h"

int main(int argc, char* argv[]) {
    if (setlocale(LC_ALL, "ar_SA.utf8") == NULL) {
        printf("Failed to set locale\n");
        return 1;
    }

    if (argc < 2) {
        printf("Please specify a source file (./qaf <filename>)\n");
        return 1;
    }

    src_t src;
    init_src(&src, argv[1]);
    read_src(argv[1], &src);

    lexer_t lexer;
    init_lexer(&lexer);
    lex(&src, &lexer);

    // int i;
    // for (i = 0; i < lexer.size; i++) {
    //     wprintf(L"%ls\n", lexer.tokens[i].data);
    // }

    vars_t vars;
    init_vars(&vars);

    parser_t parser;
    init_parser(&parser);
    parse(&lexer, &parser, &vars);
    print_tree(parser.parseTree, 0);
    
    free_src(&src);
    free_lexer(&lexer);
    free_vars(&vars);
    free_parser(&parser);
    return 0;
}