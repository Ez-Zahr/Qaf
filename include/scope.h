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
    var_type_t* vars_types;
    int* vars_depths;
    int vars_size;
    int depth;
    wchar_t** args;
    var_type_t* args_types;
    int args_size;
    wchar_t** funcs;
    int* funcs_argc;
    int funcs_size;
    struct scope_t* enclosing;
} scope_t;

scope_t* init_scope(scope_t* enclosing);
int add_var(scope_t* scope, wchar_t* var, int depth);
int get_var_offset(scope_t* scope, wchar_t* var);
void set_var_type(scope_t* scope, int offset, var_type_t type);
int get_var_type(scope_t* scope, int offset);
int add_arg(scope_t* scope, wchar_t* arg);
int get_arg_offset(scope_t* scope, wchar_t* arg);
int add_func(scope_t* scope, wchar_t* func);
int get_func_offset(scope_t* scope, wchar_t* func);
void set_func_argc(scope_t* scope, int offset, int argc);
int get_func_argc(scope_t* scope, int offset);
void push_depth(scope_t* scope);
void pop_depth(scope_t* scope);

#endif