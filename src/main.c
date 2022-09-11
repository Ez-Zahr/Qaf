#include "../include/util.h"
#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/compiler.h"

int main(int argc, char* argv[]) {
    if (setlocale(LC_ALL, "ar_SA.utf8") == NULL) {
        wprintf(L"Failed to set locale\n");
        return ERR_MAIN;
    }

    int _t = 0, _a = 0, _s = 0;
    char* filename = 0;
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
                case 't': _t = 1; break;
                case 'a': _a = 1; break;
                case 's': _s = 1; break;
                default: wprintf(L"Error: Unknown option `%s`\n", argv[i]); return ERR_MAIN;
            }
        } else if (endsWith(argv[i], ".qaf") && !filename) {
            filename = argv[i];
        } else {
            wprintf(L"Error: Unknown argument '%s'\n", argv[i]);
            return ERR_MAIN;
        }
    }
    if (!filename) {
        wprintf(L"Please specify a .qaf input file (./qaf <filename>.qaf)\n");
        return ERR_MAIN;
    }

    init_allocs();

    src_t* src = init_src();
    read_src(filename, src);

    lexer_t* lexer = init_lexer();
    lex(src, lexer);
    if (_t) {
        print_tokens(lexer);
    }

    ast_t* root = init_ast_list();
    parse(lexer, root);
    if (_a) {
        print_ast_list(root);
    }

    sections_t* sections = init_sections();
    compile(root, sections);
    write_asm(sections, "a.s");

    if (!_s) {
        if (system("as a.s -o a.o") &&
            system("ld a.o -o a.out") &&
            system("rm a.s a.o")) {}
    }
    
    free_allocs();
    return ERR_NONE;
}