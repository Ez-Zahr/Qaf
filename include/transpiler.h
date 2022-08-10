#ifndef transpiler_h
#define transpiler_h

#include "parser.h"

typedef enum {
    VAR_INT,
    VAR_BOOL
} var_type_t;

typedef struct {
    wchar_t** vars;
    char** mappings;
    var_type_t* types;
    int cap;
    int size;
} map_t;

void add_mapping(map_t* map, wchar_t* var);
char* get_mapping(map_t* map, wchar_t* var);
void set_mapping_type(map_t* map, char* mapping, var_type_t type);
var_type_t get_mapping_type(map_t* map, char* mapping);
void transpile(parser_t* parser);

#endif