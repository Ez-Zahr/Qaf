#include "../include/scope.h"

scope_t* init_scope(scope_t* enclosing) {
    scope_t* scope = (scope_t*) calloc(1, sizeof(scope_t));
    scope->vars = (wchar_t**) calloc(1, sizeof(wchar_t*));
    scope->types = (var_type_t*) calloc(1, sizeof(var_type_t));
    scope->nargs = (int*) calloc(1, sizeof(int));
    scope->depths = (int*) calloc(1, sizeof(int));
    scope->size = 0;
    scope->cur_depth = 0;
    scope->enclosing = enclosing;
    return scope;
}

int add_var(scope_t* scope, wchar_t* var) {
    for (int i = 0; i < scope->size; i++) {
        if (!wcscmp(scope->vars[i], var) && scope->depths[i] == scope->cur_depth) {
            return -1;
        }
    }
    
    scope->vars[scope->size] = var;
    scope->depths[scope->size++] = scope->cur_depth;
    scope->vars = (wchar_t**) realloc(scope->vars, (scope->size + 1) * sizeof(wchar_t*));
    scope->types = (var_type_t*) realloc(scope->types, (scope->size + 1) * sizeof(var_type_t));
    scope->nargs = (int*) realloc(scope->nargs, (scope->size + 1) * sizeof(int));
    scope->depths = (int*) realloc(scope->depths, (scope->size + 1) * sizeof(int));
    return scope->size - 1;
}

int get_var(scope_t* scope, wchar_t* var) {
    for (int depth = scope->cur_depth; depth >= 0; depth--) {
        for (int i = 0; i < scope->size; i++) {
            if (!wcscmp(scope->vars[i], var) && scope->depths[i] == depth) {
                return i;
            }
        }
    }
    return -1;
}

void set_var_type(scope_t* scope, int offset, var_type_t type) {
    scope->types[offset] = type;
}

int get_var_type(scope_t* scope, int offset) {
    return scope->types[offset];
}

void set_func_argc(scope_t* scope, int offset, int argc) {
    scope->nargs[offset] = argc;
}

int get_func_argc(scope_t* scope, int offset) {
    return scope->nargs[offset];
}

void pop_cur_depth(scope_t* scope) {
    for (int i = 0; i < scope->size; i++) {
        if (scope->depths[i] == scope->cur_depth) {
            scope->depths[i] = -1;
        }
    }
    scope->cur_depth--;
}

void free_scope(scope_t* scope) {
    if (!scope) {
        return;
    }
    free(scope->vars);
    free(scope->types);
    free(scope->depths);
    free(scope);
}