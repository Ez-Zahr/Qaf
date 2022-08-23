#include "../include/scope.h"

scope_t* init_scope(scope_t* enclosing) {
    scope_t* scope = (scope_t*) calloc(1, sizeof(scope_t));
    scope->vars = (wchar_t**) calloc(1, sizeof(wchar_t*));
    scope->types = (var_type_t*) calloc(1, sizeof(var_type_t));
    scope->size = 0;
    scope->enclosing = enclosing;
    return scope;
}

int add_var(scope_t* scope, wchar_t* var) {
    if (get_var(scope, var) != -1) {
        return 0;
    }
    scope->vars[scope->size++] = var;
    scope->vars = (wchar_t**) realloc(scope->vars, (scope->size + 1) * sizeof(wchar_t*));
    scope->types = (var_type_t*) realloc(scope->types, (scope->size + 1) * sizeof(var_type_t));
    return 1;
}

int get_var(scope_t* scope, wchar_t* var) {
    for (int i = 0; i < scope->size; i++) {
        if (!wcscmp(scope->vars[i], var)) {
            return i;
        }
    }
    return -1;
}

void free_scope(scope_t* scope) {
    if (!scope) {
        return;
    }
    free(scope->vars);
    free(scope->types);
    free(scope);
}