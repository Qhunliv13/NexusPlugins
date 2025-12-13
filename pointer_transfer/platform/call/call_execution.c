/**
 * @file call_execution.c
 * @brief 函数调用执行 / Function call execution / Funktionsaufruf-Ausführung
 */

#include "pointer_transfer_platform.h"
#include "pointer_transfer_utils.h"
#include "pointer_transfer_currying.h"
#include "nxld_plugin_interface.h"
#include <stddef.h>
#include <stdint.h>

/**
 * @brief 执行动态函数调用 / Execute dynamic function call / Dynamischen Funktionsaufruf ausführen
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
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int32_t pt_call_execute_function(void* func_ptr, int param_count, nxld_param_type_t* param_types, void** param_values, size_t* param_sizes,
                                 pt_return_type_t return_type, size_t return_size, int64_t* result_int, double* result_float, void* result_struct) {
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

