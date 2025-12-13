/**
 * @file call.c
 * @brief 平台函数调用函数 / Platform function call function / Plattform-Funktionsaufruf-Funktion
 */

#include "pointer_transfer_platform.h"
#include "pointer_transfer_utils.h"
#include "pointer_transfer_currying.h"
#include "nxld_plugin_interface.h"
#include <stdlib.h>
#include <string.h>

/**
 * @brief 动态函数调用 / Dynamic function call / Dynamischer Funktionsaufruf
 */
static int32_t call_function_dynamic(void* func_ptr, int param_count, nxld_param_type_t* param_types, void** param_values, size_t* param_sizes,
                                      pt_return_type_t return_type, size_t return_size, int64_t* result_int, double* result_float, void* result_struct) {
    if (func_ptr == NULL) {
        return -1;
    }
    if (param_count < 0) {
        return -1;
    }
    if (param_count > 0 && (param_types == NULL || param_values == NULL)) {
        return -1;
    }
    if (result_int == NULL || result_float == NULL) {
        return -1;
    }
    
    if (param_count > 0) {
        for (int i = 0; i < param_count; i++) {
            if (param_types[i] < NXLD_PARAM_TYPE_VOID || param_types[i] > NXLD_PARAM_TYPE_UNKNOWN) {
                return -1;
            }
            if ((param_types[i] == NXLD_PARAM_TYPE_POINTER || 
                 param_types[i] == NXLD_PARAM_TYPE_STRING ||
                 param_types[i] == NXLD_PARAM_TYPE_ANY) && 
                param_sizes != NULL && param_sizes[i] == 0 && param_values[i] != NULL) {
                return -1;
            }
        }
    }
    
    if (return_type < PT_RETURN_TYPE_INTEGER || return_type > PT_RETURN_TYPE_STRUCT_VAL) {
        return -1;
    }
    
    if (return_type == PT_RETURN_TYPE_STRUCT_VAL) {
        if (return_size == 0) {
            return -1;
        }
        if (result_struct == NULL) {
            return -1;
        }
    }
    
    pt_param_pack_t* pack = pt_create_param_pack(param_count, param_types, param_values, param_sizes);
    if (pack == NULL) {
        return -1;
    }
    
    if (pt_validate_param_pack(pack) != 0) {
        pt_free_param_pack(pack);
        return -1;
    }
    
    int32_t result = pt_call_with_currying(func_ptr, pack, return_type, return_size, result_int, result_float, result_struct);
    
    pt_free_param_pack(pack);
    return result;
}

/**
 * @brief 平台函数调用 / Platform function call / Plattform-Funktionsaufruf
 */
int32_t pt_platform_safe_call(void* func_ptr, int param_count, void* param_types, void** param_values, void* param_sizes,
                               pt_return_type_t return_type, size_t return_size, int64_t* result_int, double* result_float, void* result_struct) {
    return call_function_dynamic(func_ptr, param_count, (nxld_param_type_t*)param_types, param_values, (size_t*)param_sizes,
                                 return_type, return_size, result_int, result_float, result_struct);
}

