#include "../include/util.h"

wchar_t* wcsrev(wchar_t* str) {
    wchar_t *p1, *p2;
    if (!str || !*str)
        return str;
    for (p1 = str, p2 = str + wcslen(str) - 1; p2 > p1; ++p1, --p2){
        *p1 ^= *p2;
        *p2 ^= *p1;
        *p1 ^= *p2;
    }
    return str;
}

void init_src(src_t* src) {
    src->cap = 1024;
    src->size = 0;
    src->pos = 0;
    
    src->buf = (wchar_t*) calloc(src->cap, sizeof(wchar_t));
    if (src->buf == NULL) {
        wprintf(L"Failed to initialize source buffer\n");
        exit(1);
    }
}

void read_src(char* filename, src_t* src) {
    FILE* input;

    if ((input = fopen(filename, "r")) == NULL) {
        wprintf(L"Could not open file %s\n", filename);
        exit(1);
    }

    wint_t c;
    while ((c = fgetwc(input)) != WEOF) {
        src->buf[src->size++] = c;
        
        if (src->size >= src->cap) {
            src->cap *= 2;
            src->buf = (wchar_t*) realloc(src->buf, src->cap * sizeof(wchar_t));
            if (src->buf == NULL) {
                wprintf(L"Failed to resize source buffer\n");
                exit(1);
            }
        }
    }
    src->buf[src->size] = L'\0';
    
    fclose(input);
}

void free_src(src_t* src) {
    free(src->buf);
}

void print_obj(obj_t* obj, int newline) {
    switch (obj->type) {
        case OBJ_INT:
            wprintf(L"%d", *(int*)obj->data);
            break;
        case OBJ_BOOL:
            wprintf(L"%ls", (*(int*)obj->data)? L"صح":L"خطأ");
            break;
        case OBJ_CHAR:
            wprintf(L"%lc", *(wchar_t*)obj->data);
            break;
        case OBJ_STR:
            wprintf(L"%ls", (wchar_t*)obj->data);
            break;
    }

    if (newline) {
        wprintf(L"\n");
    }
}

void free_obj(obj_t* obj) {
    if (obj->type != OBJ_NULL) {
        free(obj->data);
    }
    free(obj);
}

void init_vars(vars_t* vars) {
    vars->cap = 64;
    vars->size = 0;

    vars->list = (var_t*) calloc(vars->cap, sizeof(var_t));
    if (vars->list == NULL) {
        wprintf(L"Failed to initialize variables list\n");
        exit(1);
    }
}

void add_var(vars_t* vars, wchar_t* id, obj_t* obj) {
    vars->list[vars->size].id = (wchar_t*) calloc(wcslen(id) + 1, sizeof(wchar_t));
    wcscpy(vars->list[vars->size].id, id);
    vars->list[vars->size++].obj = obj;

    if (vars->size >= vars->cap) {
        vars->cap *= 2;
        vars->list = (var_t*) realloc(vars->list, vars->cap * sizeof(var_t));
        if (vars->list == NULL) {
            wprintf(L"Failed to resize variables list\n");
            exit(1);
        }
    }
}

void update_var(vars_t* vars, wchar_t* id, obj_t* obj) {
    int i;
    for (i = 0; i < vars->size; i++) {
        if (wcscmp(vars->list[i].id, id) == 0) {
            free_obj(vars->list[i].obj);
            vars->list[i].obj = obj;
        }
    }
}

int is_var(vars_t* vars, wchar_t* id) {
    int i;
    for (i = 0; i < vars->size; i++) {
        if (wcscmp(vars->list[i].id, id) == 0) {
            return 1;
        }
    }

    return 0;
}

void copy_obj(obj_t* dest, obj_t* src) {
    switch (src->type) {
        case OBJ_INT:
            dest->type = OBJ_INT;
            dest->data = calloc(1, sizeof(int));
            *(int*)dest->data = *(int*)src->data;
            break;

        case OBJ_BOOL:
            dest->type = OBJ_BOOL;
            dest->data = calloc(1, sizeof(int));
            *(int*)dest->data = *(int*)src->data;
            break;
        
        case OBJ_CHAR:
            dest->type = OBJ_CHAR;
            dest->data = calloc(1, sizeof(char));
            *(char*)dest->data = *(char*)src->data;
            break;

        case OBJ_STR:
            dest->type = OBJ_STR;
            dest->data = calloc(wcslen((wchar_t*)src->data), sizeof(char));
            wcscpy((wchar_t*)dest->data, (wchar_t*)src->data);
            break;
        
        default:
            wprintf(L"Error: Cannot copy object of type %d\n", src->type);
            exit(1);
    }
}

obj_t* get_var(vars_t* vars, wchar_t* id) {
    obj_t* obj = (obj_t*) calloc(1, sizeof(obj_t));

    int i;
    for (i = 0; i < vars->size; i++) {
        if (wcscmp(vars->list[i].id, id) == 0) {
            copy_obj(obj, vars->list[i].obj);
            return obj;
        }
    }

    obj->type = OBJ_NULL;
    return obj;
}

void print_vars(vars_t* vars) {
    int i;
    for (i = 0; i < vars->size; i++) {
        wprintf(L"%ls = %d\n", vars->list[i].id, *(int*)(vars->list[i].obj->data));
    }
}

void free_vars(vars_t* vars) {
    int i;
    for (i = 0; i < vars->size; i++) {
        free(vars->list[i].id);
        free_obj(vars->list[i].obj);
    }
    free(vars->list);
}