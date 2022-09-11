#include "../include/scope.h"

scope_t* init_scope(scope_t* enclosing) {
    scope_t* scope = (scope_t*) smart_alloc(1, sizeof(scope_t));
    scope->vars = (wchar_t**) smart_alloc(1, sizeof(wchar_t*));
    scope->vars_types = (var_type_t*) smart_alloc(1, sizeof(var_type_t));
    scope->vars_depths = (int*) smart_alloc(1, sizeof(int));
    scope->vars_size = 0;
    scope->depth = 0;
    scope->args = (wchar_t**) smart_alloc(1, sizeof(wchar_t*));
    scope->args_types = (var_type_t*) smart_alloc(1, sizeof(var_type_t));
    scope->args_size = 0;
    scope->funcs = (wchar_t**) smart_alloc(1, sizeof(wchar_t*));
    scope->funcs_argc = (int*) smart_alloc(1, sizeof(int));
    scope->funcs_size = 0;
    scope->enclosing = enclosing;
    return scope;
}

int add_var(scope_t* scope, wchar_t* var, int depth) {
    for (int i = 0; i < scope->vars_size; i++) {
        if (!wcscmp(scope->vars[i], var) && scope->vars_depths[i] == depth) {
            return -1;
        }
    }
    scope->vars[scope->vars_size] = var;
    scope->vars_types[scope->vars_size] = -1;
    scope->vars_depths[scope->vars_size++] = depth;
    scope->vars = (wchar_t**) smart_realloc(scope->vars, scope->vars_size + 1, sizeof(wchar_t*));
    scope->vars_types = (var_type_t*) smart_realloc(scope->vars_types, scope->vars_size + 1, sizeof(var_type_t));
    scope->vars_depths = (int*) smart_realloc(scope->vars_depths, scope->vars_size + 1, sizeof(int));
    return scope->args_size + scope->vars_size - 1;
}

int get_var_offset(scope_t* scope, wchar_t* var) {
    if (!scope) return -1;
    for (int depth = scope->depth; depth >= 0; depth--) {
        for (int i = scope->vars_size - 1; i >= 0; i--) {
            if (!wcscmp(scope->vars[i], var) && scope->vars_depths[i] == depth) {
                return scope->args_size + i;
            }
        }
    }
    for (int i = 0; i < scope->args_size; i++) {
        if (!wcscmp(scope->args[i], var)) {
            return i;
        }
    }
    return get_var_offset(scope->enclosing, var);
}

void set_var_type(scope_t* scope, int offset, var_type_t type) {
    scope->vars_types[offset] = type;
}

int get_var_type(scope_t* scope, int offset) {
    return scope->vars_types[offset];
}

int add_arg(scope_t* scope, wchar_t* arg) {
    if (get_arg_offset(scope, arg) != -1) {
        return -1;
    }
    scope->args[scope->args_size++] = arg;
    scope->args = (wchar_t**) smart_realloc(scope->args, scope->args_size + 1, sizeof(wchar_t*));
    return scope->args_size - 1;
}

int get_arg_offset(scope_t* scope, wchar_t* arg) {
    for (int i = 0; i < scope->args_size; i++) {
        if (!wcscmp(scope->args[i], arg)) {
            return i;
        }
    }
    return -1;
}

int add_func(scope_t* scope, wchar_t* func) {
    if (get_func_offset(scope, func) != -1) {
        return -1;
    }
    scope->funcs[scope->funcs_size++] = func;
    scope->funcs = (wchar_t**) smart_realloc(scope->funcs, scope->funcs_size + 1, sizeof(wchar_t*));
    return scope->funcs_size - 1;
}

int get_func_offset(scope_t* scope, wchar_t* func) {
    for (int i = 0; i < scope->funcs_size; i++) {
        if (!wcscmp(scope->funcs[i], func)) {
            return i;
        }
    }
    return -1;
}

void set_func_argc(scope_t* scope, int offset, int argc) {
    scope->funcs_argc[offset] = argc;
}

int get_func_argc(scope_t* scope, int offset) {
    return scope->funcs_argc[offset];
}

void push_depth(scope_t* scope) {
    scope->depth++;
}

void pop_depth(scope_t* scope) {
    for (int i = 0; i < scope->vars_size; i++) {
        if (scope->vars_depths[i] == scope->depth) {
            scope->vars_depths[i] = -1;
        }
    }
    scope->depth--;
}