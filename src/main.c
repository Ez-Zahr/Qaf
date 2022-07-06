#include "../include/util.h"
#include "../include/lexer.h"

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

    free(src.buf);
    free_lexer(&lexer);
    return 0;
}