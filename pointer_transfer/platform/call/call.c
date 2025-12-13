/**
 * @file call.c
 * @brief 平台函数调用 / Platform function call / Plattform-Funktionsaufruf
 */

#include "pointer_transfer_platform.h"
#include "pointer_transfer_types.h"
#include "nxld_plugin_interface.h"
#include <stddef.h>
#include <stdint.h>

extern int32_t pt_call_validate_params(void* func_ptr, int param_count, nxld_param_type_t* param_types, void** param_values, size_t* param_sizes,
                                        pt_return_type_t return_type, size_t return_size, int64_t* result_int, double* result_float, void* result_struct);

extern int32_t pt_call_execute_function(void* func_ptr, int param_count, nxld_param_type_t* param_types, void** param_values, size_t* param_sizes,
                                         pt_return_type_t return_type, size_t return_size, int64_t* result_int, double* result_float, void* result_struct);

/**
 * @brief 平台函数调用 / Platform function call / Plattform-Funktionsaufruf
 */
int32_t pt_platform_safe_call(void* func_ptr, int param_count, void* param_types, void** param_values, void* param_sizes,
                               pt_return_type_t return_type, size_t return_size, int64_t* result_int, double* result_float, void* result_struct) {
    nxld_param_type_t* types = (nxld_param_type_t*)param_types;
    size_t* sizes = (size_t*)param_sizes;
    
    if (pt_call_validate_params(func_ptr, param_count, types, param_values, sizes,
                                return_type, return_size, result_int, result_float, result_struct) != 0) {
        return -1;
    }
    
    return pt_call_execute_function(func_ptr, param_count, types, param_values, sizes,
                                    return_type, return_size, result_int, result_float, result_struct);
}

