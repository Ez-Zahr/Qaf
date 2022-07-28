#ifndef util_h
#define util_h

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>

typedef struct {
    char* filename;
    wchar_t* buf;
    int cap;
    int size;
    int pos;
} src_t;

typedef enum {
    OBJ_NULL,
    OBJ_INT,
    OBJ_BOOL,
    OBJ_CHAR,
    OBJ_STR
} obj_type_t;

typedef struct {
    void* data;
    obj_type_t type;
} obj_t;

typedef struct {
    wchar_t* id;
    obj_t* obj;
} var_t;

typedef struct {
    var_t* list;
    int cap;
    int size;
} vars_t;

wchar_t* wcsrev(wchar_t* str);

void init_src(src_t* src);
void read_src(char* filename, src_t* src);
void free_src(src_t* src);

void print_obj(obj_t* obj, int newline);
void free_obj(obj_t* obj);

void init_vars(vars_t* vars);
void add_var(vars_t* vars, wchar_t* id, obj_t* val);
void update_var(vars_t* vars, wchar_t* id, obj_t* val);
int is_var(vars_t* vars, wchar_t* id);
obj_t* get_var(vars_t* vars, wchar_t* id);
void print_vars(vars_t* vars);
void free_vars(vars_t* vars);

#endif