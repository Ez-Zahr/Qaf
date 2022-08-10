#include "../include/transpiler.h"

void init_map(map_t* map) {
    map->cap = 32;
    map->size = 0;
    map->vars = (wchar_t**) calloc(map->cap, sizeof(wchar_t*));
    map->mappings = (char**) calloc(map->cap, sizeof(char*));
    map->types = (var_type_t*) calloc(map->cap, sizeof(var_type_t));
}

void add_mapping(map_t* map, wchar_t* var) {
    map->vars[map->size] = var;
    map->types[map->size] = -1;
    map->mappings[map->size] = (char*) calloc(8, sizeof(char));
    sprintf(map->mappings[map->size], "var%d", map->size);
    map->size++;
    if (map->size >= map->cap) {
        map->cap *= 2;
        map->vars = (wchar_t**) realloc(map->vars, map->cap * sizeof(wchar_t*));
        map->mappings = (char**) realloc(map->mappings, map->cap * sizeof(char*));
        map->types = (var_type_t*) realloc(map->types, map->cap * sizeof(var_type_t));
    }
}

char* get_mapping(map_t* map, wchar_t* var) {
    for (int i = 0; i < map->size; i++) {
        if (!wcscmp(map->vars[i], var)) {
            char* tmp = (char*) calloc(strlen(map->mappings[i]) + 1, sizeof(char));
            strcpy(tmp, map->mappings[i]);
            return tmp;
        }
    }
    return 0;
}

void set_mapping_type(map_t* map, char* mapping, var_type_t type) {
    for (int i = 0; i < map->size; i++) {
        if (!strcmp(map->mappings[i], mapping)) {
            map->types[i] = type;
        }
    }
}

var_type_t get_mapping_type(map_t* map, char* mapping) {
    for (int i = 0; i < map->size; i++) {
        if (!strcmp(map->mappings[i], mapping)) {
            return map->types[i];
        }
    }
    return -1;
}

void free_map(map_t* map) {
    free(map->vars);
    for (int i = 0; i < map->size; i++) {
        free(map->mappings[i]);
    }
    free(map->mappings);
}

char* _transpile_instr(node_t* ast, map_t* map, char* format);

char* _transpile(node_t* ast, map_t* map) {
    switch (ast->tok->type) {
        case TOK_PRINT: {
            char* left = _transpile(ast->left, map);
            char* instr = (char*) calloc(strlen(left) + 24, sizeof(char));
            sprintf(instr, "printf(\"%%d\\n\", %s)", left);
            return instr;
        }

        case TOK_ID: {
            char* var;
            if (!(var = get_mapping(map, ast->tok->data))) {
                add_mapping(map, ast->tok->data);
                var = get_mapping(map, ast->tok->data);
            }
            return var;
        }

        case TOK_INT: {
            int len = wcslen(ast->tok->data);
            char* buf = (char*) calloc(len + 1, sizeof(char));
            wcstombs(buf, ast->tok->data, len);
            return buf;
        }

        case TOK_BOOL: {
            char* expr = (char*) calloc(2, sizeof(char));
            sprintf(expr, "%d", !wcscmp(ast->tok->data, L"صح"));
            return expr;
        }

        case TOK_ASSIGN: {
            return _transpile_instr(ast, map, "%s = %s;");
        }

        case TOK_PLUS: {
            return _transpile_instr(ast, map, "%s + %s");
        }

        case TOK_MINUS: {
            return _transpile_instr(ast, map, "%s - %s");
        }

        case TOK_MUL: {
            return _transpile_instr(ast, map, "%s * %s");
        }

        case TOK_DIV: {
            return _transpile_instr(ast, map, "%s / %s");
        }

        case TOK_MOD: {
            return _transpile_instr(ast, map, "%s %% %s");
        }

        case TOK_AND: {
            return _transpile_instr(ast, map, "%s && %s");
        }

        case TOK_OR: {
            return _transpile_instr(ast, map, "%s || %s");
        }

        case TOK_NOT: {
            char* left = _transpile(ast->left, map);
            char* instr = (char*) calloc(strlen(left) + 4, sizeof(char));
            sprintf(instr, "!%s", left);
            free(left);
            return instr;
        }

        case TOK_EQ: {
            return _transpile_instr(ast, map, "%s == %s");
        }

        case TOK_NE: {
            return _transpile_instr(ast, map, "%s != %s");
        }

        case TOK_LT: {
            return _transpile_instr(ast, map, "%s < %s");
        }

        case TOK_LTE: {
            return _transpile_instr(ast, map, "%s <= %s");
        }

        case TOK_GT: {
            return _transpile_instr(ast, map, "%s > %s");
        }

        case TOK_GTE: {
            return _transpile_instr(ast, map, "%s >= %s");
        }

        default: {
            wprintf(L"Error: Undefined transpilation for `%ls`\n", tok_type_to_str(ast->tok->type));
            exit(1);
        }
    }
}

char* _transpile_instr(node_t* ast, map_t* map, char* format) {
    char* left = _transpile(ast->left, map);
    char* right = _transpile(ast->right, map);
    char* instr = (char*) calloc(strlen(left) + strlen(right) + strlen(format) + 1, sizeof(char));
    sprintf(instr, format, left, right);
    free(left);
    free(right);
    return instr;
}

void transpile(parser_t* parser) {
    map_t map;
    init_map(&map);

    char* buf = (char*) calloc(38, sizeof(char));
    strcpy(buf, "#include <stdio.h>\n\nint main(void) {\n");
    char* instrLines = (char*) calloc(1, sizeof(char));
    for (int i = 0; i < parser->size; i++) {
        char* instr = _transpile(parser->astList[i], &map);
        instrLines = (char*) realloc(instrLines, (strlen(instrLines) + strlen(instr) + 3) * sizeof(char));
        strcat(instrLines, "\t");
        strcat(instrLines, instr);
        strcat(instrLines, "\n");
    }
    buf = (char*) realloc(buf, (strlen(buf) + 2) * sizeof(char));
    strcat(buf, "}");

    FILE* output;

    if ((output = fopen("a.c", "w")) == NULL) {
        wprintf(L"Could not open file a.c\n");
        exit(1);
    }

    fputs(buf, output);
    fclose(output);

    // system("gcc a.c");
    
    free_map(&map);
}