#include "../include/scope.h"

scope_t* init_scope(scope_t* enclosing) {
    scope_t* scope = (scope_t*) calloc(1, sizeof(scope_t));
    scope->ids = (wchar_t**) calloc(1, sizeof(wchar_t*));
    scope->types = (var_type_t*) calloc(1, sizeof(var_type_t));
    scope->depths = (int*) calloc(1, sizeof(int));
    scope->size = 0;
    scope->enclosing = enclosing;
    scope->cur_depth = 0;
    return scope;
}

int add_var(scope_t* scope, wchar_t* var) {
    for (int i = 0; i < scope->size; i++) {
        if (!wcscmp(scope->ids[i], var) && scope->depths[i] == scope->cur_depth) {
            return -1;
        }
    }
    
    scope->ids[scope->size] = var;
    scope->depths[scope->size++] = scope->cur_depth;
    scope->ids = (wchar_t**) realloc(scope->ids, (scope->size + 1) * sizeof(wchar_t*));
    scope->types = (var_type_t*) realloc(scope->types, (scope->size + 1) * sizeof(var_type_t));
    scope->depths = (int*) realloc(scope->depths, (scope->size + 1) * sizeof(int));
    return scope->size - 1;
}

int get_var(scope_t* scope, wchar_t* var) {
    for (int cur_depth = scope->cur_depth; cur_depth >= 0; cur_depth--) {
        for (int i = 0; i < scope->size; i++) {
            if (!wcscmp(scope->ids[i], var) && scope->depths[i] == cur_depth) {
                return i;
            }
        }
    }
    return -1;
}

void free_scope(scope_t* scope) {
    if (!scope) {
        return;
    }
    free(scope->ids);
    free(scope->types);
    free(scope->depths);
    free(scope);
}