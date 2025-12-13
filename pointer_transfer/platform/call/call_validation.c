/**
 * @file call_validation.c
 * @brief 函数调用参数验证 / Function call parameter validation / Funktionsaufruf-Parametervalidierung
 */

#include "pointer_transfer_platform.h"
#include "nxld_plugin_interface.h"
#include <stddef.h>
#include <stdint.h>

/**
 * @brief 验证函数调用参数 / Validate function call parameters / Funktionsaufruf-Parameter validieren
 * @param func_ptr 函数指针 / Function pointer / Funktionszeiger
 * @param param_count 参数数量 / Parameter count / Parameteranzahl
 * @param param_types 参数类型数组 / Parameter types array / Parametertyp-Array
 * @param param_values 参数值数组 / Parameter values array / Parameterwerte-Array
 * @param param_sizes 参数大小数组 / Parameter sizes array / Parametergrößen-Array
 * @param return_type 返回值类型 / Return value type / Rückgabewerttyp
 * @param return_size 返回值大小 / Return value size / Rückgabewertgröße
 * @param result_int 输出整数返回值指针 / Output integer return value pointer / Ausgabe-Integer-Rückgabewert-Zeiger
 * @param result_float 输出浮点返回值指针 / Output floating-point return value pointer / Ausgabe-Gleitkomma-Rückgabewert-Zeiger
 * @param result_struct 输出结构体返回值缓冲区 / Output struct return value buffer / Ausgabe-Struktur-Rückgabewert-Puffer
 * @return 验证通过返回0，失败返回-1 / Returns 0 on validation success, -1 on failure / Gibt 0 bei erfolgreicher Validierung zurück, -1 bei Fehler
 */
int32_t pt_call_validate_params(void* func_ptr, int param_count, nxld_param_type_t* param_types, void** param_values, size_t* param_sizes,
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
    
    return 0;
}

