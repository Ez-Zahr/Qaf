#ifndef evaluator_h
#define evaluator_h

#include "parser.h"

typedef enum {
    OBJ_INT,
    OBJ_STR
} obj_type_t;

typedef struct {
    obj_type_t type;
    int int_val;
    wchar_t* str_val;
} obj_t;

void eval(parser_t* parser);

#endif