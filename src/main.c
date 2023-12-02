#include "../include/util.h"
#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/compiler.h"

int main(int argc, char* argv[]) {
    init_allocs();

    set_locale();

    args_t* args = init_args();
    read_args(argc, argv, args);

    src_t* src = init_src();
    read_src(args->filename, src);

    lexer_t* lexer = init_lexer();
    lex(src, lexer);
    if (args->_t) {
        print_tokens(lexer);
    }

    ast_t* root = init_ast_list();
    parse(lexer, root);
    if (args->_a) {
        print_ast_list(root);
    }

    sections_t* sections = init_sections();
    compile(root, sections);
    write_asm(sections, "a.s");

    if (!args->_s) {
        system("as a.s -o a.o");
        system("ld a.o -o a.exe");
        system("rm a.s a.o");
    }
    
    free_allocs();
    return ERR_NONE;
}