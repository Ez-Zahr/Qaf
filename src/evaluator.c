#include "../include/evaluator.h"

obj_t* get_prim_expr(obj_t* obj, vars_t* vars) {
    switch (obj->type) {
        case OBJ_STR: {
            if (is_var(vars, (wchar_t*)obj->data)) {
                obj_t* var_val = get_var(vars, (wchar_t*)obj->data);
                free_obj(obj);
                return var_val;
            } else {
                wprintf(L"Error: Undefiend variable `%ls`\n", obj->data);
                exit(1);
            }
        }
        
        default:
            return obj;
    }
}

obj_t* _eval(node_t* node, vars_t* vars) {
    if (node == NULL) {
        obj_t* obj = (obj_t*) calloc(1, sizeof(obj_t));
        obj->type = OBJ_NULL;
        return obj;
    }
    
    switch (node->tok->type) {
        case TOK_ID: {
            obj_t* obj = (obj_t*) calloc(1, sizeof(obj_t));
            obj->type = OBJ_STR;
            obj->data = calloc(node->tok->len + 1, sizeof(wchar_t));
            wcscpy((wchar_t*)obj->data, node->tok->data);
            return obj;
        }

        case TOK_KEYWORD: {
            obj_t* obj;
            if (!wcscmp(node->tok->data, L"اطبع")) {
                obj = get_prim_expr(_eval(node->left, vars), vars);
                wprintf(L"%d\n", *(int*)obj->data);
                free_obj(obj);
            }

            obj = (obj_t*) calloc(1, sizeof(obj_t));
            obj->type = OBJ_NULL;
            return obj;
        }
        
        case TOK_NUM: {
            obj_t* obj = (obj_t*) calloc(1, sizeof(obj_t));
            obj->type = OBJ_INT;
            obj->data = calloc(1, sizeof(int));
            char buf[32];
            wcstombs(buf, node->tok->data, 32);
            *(int*)obj->data = atoi(buf);
            return obj;
        }

        case TOK_ASSIGN: {
            obj_t* var_id = _eval(node->left, vars);
            obj_t* var_val = get_prim_expr(_eval(node->right, vars), vars);
            if (var_id->type != OBJ_STR || var_val->type != OBJ_INT) {
                wprintf(L"Error: Invalid operand to `=` operator\n");
                exit(1);
            }
            
            if (is_var(vars, var_id->data)) {
                update_var(vars, var_id->data, var_val);
            } else {
                add_var(vars, var_id->data, var_val);
            }

            free_obj(var_id);
            obj_t* obj = (obj_t*) calloc(1, sizeof(obj_t));
            obj->type = OBJ_NULL;
            return obj;
        }

        case TOK_PLUS: {
            obj_t* left = get_prim_expr(_eval(node->left, vars), vars);
            obj_t* right = get_prim_expr(_eval(node->right, vars), vars);
            if (left->type != OBJ_INT || right->type != OBJ_INT) {
                wprintf(L"Error: Invalid operand to `+` operator %d %d\n", left->type, right->type);
                exit(1);
            }

            obj_t* obj = (obj_t*) calloc(1, sizeof(obj_t));
            obj->type = OBJ_INT;
            obj->data = calloc(1, sizeof(int));
            *(int*)obj->data = (*(int*)left->data) + (*(int*)right->data);
            
            free_obj(left);
            free_obj(right);
            return obj;
        }

        case TOK_MINUS: {
            obj_t* left = get_prim_expr(_eval(node->left, vars), vars);
            obj_t* right = get_prim_expr(_eval(node->right, vars), vars);
            if (left->type != OBJ_INT || right->type != OBJ_INT) {
                wprintf(L"Error: Invalid operand to `-` operator\n");
                exit(1);
            }

            obj_t* obj = (obj_t*) calloc(1, sizeof(obj_t));
            obj->type = OBJ_INT;
            obj->data = calloc(1, sizeof(int));
            *(int*)obj->data = (*(int*)left->data) - (*(int*)right->data);
            
            free_obj(left);
            free_obj(right);
            return obj;
        }

        case TOK_MUL: {
            obj_t* left = get_prim_expr(_eval(node->left, vars), vars);
            obj_t* right = get_prim_expr(_eval(node->right, vars), vars);
            if (left->type != OBJ_INT || right->type != OBJ_INT) {
                wprintf(L"Error: Invalid operand to `*` operator\n");
                exit(1);
            }

            obj_t* obj = (obj_t*) calloc(1, sizeof(obj_t));
            obj->type = OBJ_INT;
            obj->data = calloc(1, sizeof(int));
            *(int*)obj->data = (*(int*)left->data) * (*(int*)right->data);
            
            free_obj(left);
            free_obj(right);
            return obj;
        }

        case TOK_DIV: {
            obj_t* left = get_prim_expr(_eval(node->left, vars), vars);
            obj_t* right = get_prim_expr(_eval(node->right, vars), vars);
            if (left->type != OBJ_INT || right->type != OBJ_INT) {
                wprintf(L"Error: Invalid operand to `/` operator\n");
                exit(1);
            }

            obj_t* obj = (obj_t*) calloc(1, sizeof(obj_t));
            obj->type = OBJ_INT;
            obj->data = calloc(1, sizeof(int));
            *(int*)obj->data = (*(int*)left->data) / (*(int*)right->data);
            
            free_obj(left);
            free_obj(right);
            return obj;
        }

        default: {
            obj_t* obj = (obj_t*) calloc(1, sizeof(obj_t));
            obj->type = OBJ_NULL;
            return obj;
        }
    }
}

void eval(parser_t* parser, vars_t* vars) {
    obj_t* obj = _eval(parser->parseTree, vars);
    free_obj(obj);
}