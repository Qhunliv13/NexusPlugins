/**
 * @file pointer_transfer_currying_call_return.c
 * @brief 柯里化函数调用返回值处理 / Currying Function Call Return Value Handling / Currying-Funktionsaufruf-Rückgabewert-Behandlung
 */

#include "pointer_transfer_currying.h"
#include <string.h>
#include <stdint.h>

/**
 * @brief 调用返回FLOAT类型的函数 / Call function returning FLOAT type / Funktion aufrufen, die FLOAT-Typ zurückgibt
 */
int32_t pt_call_curried_func_float(void* func_ptr, void* serialized_pack, double* result_float, int64_t* result_int) {
    if (func_ptr == NULL || serialized_pack == NULL || result_float == NULL || result_int == NULL) {
        return -1;
    }
    
    typedef float (*curried_func_float_t)(void*);
    curried_func_float_t f = (curried_func_float_t)func_ptr;
    *result_float = (double)f(serialized_pack);
    *result_int = 0;
    return 0;
}

/**
 * @brief 调用返回DOUBLE类型的函数 / Call function returning DOUBLE type / Funktion aufrufen, die DOUBLE-Typ zurückgibt
 */
int32_t pt_call_curried_func_double(void* func_ptr, void* serialized_pack, double* result_float, int64_t* result_int) {
    if (func_ptr == NULL || serialized_pack == NULL || result_float == NULL || result_int == NULL) {
        return -1;
    }
    
    typedef double (*curried_func_double_t)(void*);
    curried_func_double_t f = (curried_func_double_t)func_ptr;
    *result_float = f(serialized_pack);
    *result_int = 0;
    return 0;
}

/**
 * @brief 调用返回STRUCT_PTR类型的函数 / Call function returning STRUCT_PTR type / Funktion aufrufen, die STRUCT_PTR-Typ zurückgibt
 */
int32_t pt_call_curried_func_struct_ptr(void* func_ptr, void* serialized_pack, void* result_struct, size_t return_size, int64_t* result_int, double* result_float) {
    if (func_ptr == NULL || serialized_pack == NULL || result_int == NULL || result_float == NULL) {
        return -1;
    }
    
    typedef void* (*curried_func_ptr_t)(void*);
    curried_func_ptr_t f = (curried_func_ptr_t)func_ptr;
    void* struct_ptr = f(serialized_pack);
    if (result_struct != NULL && return_size > 0 && struct_ptr != NULL) {
        memcpy(result_struct, struct_ptr, return_size);
    }
    *result_int = (int64_t)struct_ptr;
    *result_float = 0.0;
    return 0;
}

/**
 * @brief 调用返回STRUCT_VAL类型的函数 / Call function returning STRUCT_VAL type / Funktion aufrufen, die STRUCT_VAL-Typ zurückgibt
 */
int32_t pt_call_curried_func_struct_val(void* func_ptr, void* serialized_pack, void* result_struct, int64_t* result_int, double* result_float) {
    if (func_ptr == NULL || serialized_pack == NULL || result_int == NULL || result_float == NULL) {
        return -1;
    }
    
    typedef void (*curried_func_struct_val_t)(void*, void*);
    curried_func_struct_val_t f = (curried_func_struct_val_t)func_ptr;
    if (result_struct != NULL) {
        f(serialized_pack, result_struct);
    }
    *result_int = 0;
    *result_float = 0.0;
    return 0;
}

/**
 * @brief 调用返回整数类型的函数 / Call function returning integer type / Funktion aufrufen, die Integer-Typ zurückgibt
 */
int32_t pt_call_curried_func_int(void* func_ptr, void* serialized_pack, int64_t* result_int, double* result_float) {
    if (func_ptr == NULL || serialized_pack == NULL || result_int == NULL || result_float == NULL) {
        return -1;
    }
    
    typedef int32_t (*curried_func_int_t)(void*);
    curried_func_int_t f = (curried_func_int_t)func_ptr;
    *result_int = (int64_t)f(serialized_pack);
    *result_float = 0.0;
    return 0;
}

