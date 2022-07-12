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
    init_src(&src, argv[1]);
    lexer_t lexer;
    init_lexer(&lexer);
    parser_t parser;
    init_parser(&parser);
    
    read_src(argv[1], &src);
    lex(&src, &lexer);

    if (0) {
        int i;
        wprintf(L"%d\n", lexer.size);
        for (i = 0; i < lexer.size; i++) {
            wprintf(L"`%ls` of length %d type %d\n", lexer.tokens[i].data, lexer.tokens[i].len, lexer.tokens[i].type);
        }
    }

    while (1) {
        if (lexer.tokens[lexer.pos].type == TOK_NEWLINE) {
            lexer.pos++;
        }
        if (lexer.tokens[lexer.pos].type == TOK_EOF) {
            break;
        }
        parse(&lexer, &parser);
        if (0) {
            print_tree(parser.parseTree, 0);
        }
        eval(&parser);
        free_parse_tree(parser.parseTree);
        parser.parseTree = NULL;
    }

    if (0) {
        int i;
        for (i = 0; i < parser.vars->size; i++) {
            wprintf(L"%ls = %d\n", parser.vars->list[i].id, parser.vars->list[i].val->val);
        }
    }

    free_src(&src);
    free_lexer(&lexer);
    free_parser(&parser);
    return 0;
}