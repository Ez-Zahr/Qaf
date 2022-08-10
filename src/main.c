#include "../include/util.h"
#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/compiler.h"
#include "../include/transpiler.h"

int main(int argc, char* argv[]) {
    if (setlocale(LC_ALL, "ar_SA.utf8") == NULL) {
        wprintf(L"Failed to set locale\n");
        return 1;
    }

    int _t = 0, _a = 0;
    char* filename = 0;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-t")) {
            _t = 1;
        } else if (!strcmp(argv[i], "-a")) {
            _a = 1;
        } else if (endsWith(argv[i], ".qaf") && !filename) {
            filename = argv[i];
        } else {
            wprintf(L"Error: Unknown argument '%s'\n", argv[i]);
            return 1;
        }
    }
    if (!filename) {
        wprintf(L"Please specify a .qaf input file (./qaf <filename>.qaf)\n");
        return 1;
    }

    src_t src;
    init_src(&src);
    lexer_t lexer;
    init_lexer(&lexer);
    parser_t parser;
    init_parser(&parser);
    
    read_src(filename, &src);

    lex(&src, &lexer);
    if (_t) {
        print_tokens(&lexer);
    }

    parse(&lexer, &parser);
    if (_a) {
        print_parser(&parser);
    }

    compile(&parser);
    // transpile(&parser);

    free_parser(&parser);
    free_lexer(&lexer);
    free_src(&src);
    return 0;
}