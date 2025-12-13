/**
 * @file pointer_transfer_currying_call.c
 * @brief 柯里化函数调用 / Currying Function Call / Currying-Funktionsaufruf
 */

#include "pointer_transfer_currying.h"
#include "pointer_transfer_utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/**
 * @brief 柯里化调用函数 / Call function using currying / Funktion mit Currying aufrufen
 */
int32_t pt_call_with_currying(void* func_ptr, pt_param_pack_t* pack, 
                               pt_return_type_t return_type, size_t return_size,
                               int64_t* result_int, double* result_float, void* result_struct) {
    if (func_ptr == NULL || pack == NULL || result_int == NULL || result_float == NULL) {
        return -1;
    }
    
    if (pt_validate_param_pack(pack) != 0) {
        internal_log_write("ERROR", "Call with currying failed: param pack validation failed");
        return -1;
    }
    
    /* 序列化参数包为连续内存块，确保数据自包含和安全性 / Serialize parameter pack to contiguous memory block to ensure data self-containment and safety / Parameterpaket in zusammenhängenden Speicherblock serialisieren, um Daten-Selbständigkeit und Sicherheit zu gewährleisten */
    void* serialized_data = pt_serialize_param_pack(pack);
    if (serialized_data == NULL) {
        internal_log_write("ERROR", "Call with currying failed: failed to serialize parameter pack");
        return -1;
    }
    
    /* 序列化后的数据格式与pt_param_pack_t兼容，可直接作为参数包使用 / Serialized data format is compatible with pt_param_pack_t and can be used directly as parameter pack / Serialisiertes Datenformat ist mit pt_param_pack_t kompatibel und kann direkt als Parameterpaket verwendet werden */
    pt_param_pack_t* serialized_pack = pt_deserialize_param_pack(serialized_data);
    if (serialized_pack == NULL) {
        internal_log_write("ERROR", "Call with currying failed: failed to deserialize parameter pack");
        pt_free_serialized_param_pack(serialized_data);
        return -1;
    }
    
    typedef int32_t (*curried_func_int_t)(void*);
    typedef float (*curried_func_float_t)(void*);
    typedef double (*curried_func_double_t)(void*);
    typedef int64_t (*curried_func_int64_t)(void*);
    typedef void* (*curried_func_ptr_t)(void*);
    
    int32_t call_result = 0;
    
    switch (return_type) {
        case PT_RETURN_TYPE_FLOAT: {
            curried_func_float_t f = (curried_func_float_t)func_ptr;
            *result_float = (double)f((void*)serialized_pack);
            *result_int = 0;
            break;
        }
        case PT_RETURN_TYPE_DOUBLE: {
            curried_func_double_t f = (curried_func_double_t)func_ptr;
            *result_float = f((void*)serialized_pack);
            *result_int = 0;
            break;
        }
        case PT_RETURN_TYPE_STRUCT_PTR: {
            curried_func_ptr_t f = (curried_func_ptr_t)func_ptr;
            void* struct_ptr = f((void*)serialized_pack);
            if (result_struct != NULL && return_size > 0 && struct_ptr != NULL) {
                memcpy(result_struct, struct_ptr, return_size);
            }
            *result_int = (int64_t)struct_ptr;
            *result_float = 0.0;
            break;
        }
        case PT_RETURN_TYPE_STRUCT_VAL: {
            typedef void (*curried_func_struct_val_t)(void*, void*);
            curried_func_struct_val_t f = (curried_func_struct_val_t)func_ptr;
            if (result_struct != NULL) {
                f((void*)serialized_pack, result_struct);
            }
            *result_int = 0;
            *result_float = 0.0;
            break;
        }
        default:
        {
            curried_func_int_t f = (curried_func_int_t)func_ptr;
            *result_int = (int64_t)f((void*)serialized_pack);
            *result_float = 0.0;
            break;
        }
    }
    
    /* 释放序列化的参数包（连续内存块） / Free serialized parameter pack (contiguous memory block) / Serialisiertes Parameterpaket freigeben (zusammenhängender Speicherblock) */
    pt_free_serialized_param_pack(serialized_data);
    
    return call_result;
}

