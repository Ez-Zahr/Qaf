#include "../include/util.h"
#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/compiler.h"

extern ERROR_STATUS err_status;

int main(int argc, char* argv[]) {
    if (setlocale(LC_ALL, "ar_SA.utf8") == NULL) {
        wprintf(L"Failed to set locale\n");
        return 1;
    }

    err_status = ERR_NONE;

    int _t = 0, _a = 0, _s = 0;
    char* filename = 0;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-t")) {
            _t = 1;
        } else if (!strcmp(argv[i], "-a")) {
            _a = 1;
        } else if (!strcmp(argv[i], "-s")) {
            _s = 1;
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

    src_t* src = init_src();
    lexer_t* lexer = init_lexer();
    ast_t* root = init_ast_list();
    sections_t* sections = init_sections();
    
    read_src(filename, src);

    if (err_status == ERR_NONE) {
        lex(src, lexer);
        if (_t) {
            print_tokens(lexer);
        }
    }

    if (err_status == ERR_NONE) {
        parse(lexer, root);
        if (_a) {
            print_ast_list(root);
        }
    }

    // if (err_status == ERR_NONE) {
    //     compile(root, sections);
    // }

    // if (err_status == ERR_NONE) {
    //     write_asm(sections, "a.s");
    // }

    // if (err_status == ERR_NONE && !_s) {
    //     (void) (system("as a.s -o a.o") + 1);
    //     (void) (system("ld a.o -o a.out") + 1);
    //     (void) (system("rm a.s a.o") + 1);
    // }
    
    free_sections(sections);
    free_ast_list(root);
    free_lexer(lexer);
    free_src(src);
    return err_status;
}