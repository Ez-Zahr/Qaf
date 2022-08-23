#ifndef scope_h
#define scope_h

#include "util.h"

typedef enum var_type_t {
    VAR_INT,
    VAR_BOOL,
    VAR_STR
} var_type_t;

typedef struct scope_t {
    wchar_t** vars;
    var_type_t* types;
    int size;
    struct scope_t* enclosing;
} scope_t;

scope_t* init_scope(scope_t* enclosing);
int add_var(scope_t* scope, wchar_t* var);
int get_var(scope_t* scope, wchar_t* var);
void free_scope(scope_t* scope);

#endif