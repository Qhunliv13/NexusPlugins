/**
 * @file pointer_transfer_interface_return.c
 * @brief 返回值处理和函数调用 / Return Value Processing and Function Call / Rückgabewert-Verarbeitung und Funktionsaufruf
 */

#include "pointer_transfer_interface.h"
#include "pointer_transfer_context.h"
#include "pointer_transfer_types.h"
#include "pointer_transfer_platform.h"
#include "pointer_transfer_currying.h"
#include "pointer_transfer_utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

/**
 * @brief 准备返回值类型和缓冲区 / Prepare return type and buffer / Rückgabetyp und Puffer vorbereiten
 * @param state 接口状态 / Interface state / Schnittstellenstatus
 * @param return_type_out 输出返回值类型 / Output return type / Ausgabe-Rückgabetyp
 * @param return_size_out 输出返回值大小 / Output return size / Ausgabe-Rückgabegröße
 * @param struct_buffer_out 输出结构体缓冲区 / Output struct buffer / Ausgabe-Strukturpuffer
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int prepare_return_type_and_buffer(target_interface_state_t* state, pt_return_type_t* return_type_out, 
                                    size_t* return_size_out, void** struct_buffer_out) {
    if (state == NULL || return_type_out == NULL || return_size_out == NULL || struct_buffer_out == NULL) {
        return -1;
    }
    
    pt_return_type_t return_type = state->return_type;
    size_t return_size = state->return_size;
    
    if (return_type == PT_RETURN_TYPE_STRUCT_PTR && return_size > 0) {
#ifdef _WIN32
        if (return_size > 8) {
            return_type = PT_RETURN_TYPE_STRUCT_VAL;
        }
#else
        if (return_size > 16) {
            return_type = PT_RETURN_TYPE_STRUCT_VAL;
        }
#endif
    }
    
    if (return_type == PT_RETURN_TYPE_INTEGER) {
        internal_log_write("INFO", "Return type not explicitly configured for interface, return type set to integer");
    }
    
    void* struct_buffer = NULL;
    if (return_type == PT_RETURN_TYPE_STRUCT_VAL && return_size > 0) {
        struct_buffer = malloc(return_size);
        if (struct_buffer == NULL) {
            internal_log_write("ERROR", "Failed to allocate buffer for struct return value (size=%zu)", return_size);
            return -1;
        }
        memset(struct_buffer, 0, return_size);
    }
    
    *return_type_out = return_type;
    *return_size_out = return_size;
    *struct_buffer_out = struct_buffer;
    return 0;
}

/**
 * @brief 调用函数并获取返回值 / Call function and get return value / Funktion aufrufen und Rückgabewert abrufen
 * @param state 接口状态 / Interface state / Schnittstellenstatus
 * @param actual_param_count 实际参数数量 / Actual parameter count / Tatsächliche Parameteranzahl
 * @param return_type 返回值类型 / Return type / Rückgabetyp
 * @param return_size 返回值大小 / Return size / Rückgabegröße
 * @param struct_buffer 结构体缓冲区 / Struct buffer / Strukturpuffer
 * @param result_int_out 输出整数结果 / Output integer result / Ausgabe-Ganzzahlergebnis
 * @param result_float_out 输出浮点数结果 / Output float result / Ausgabe-Gleitkommaergebnis
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int call_function_and_get_result(target_interface_state_t* state, int actual_param_count,
                                  pt_return_type_t return_type, size_t return_size, void* struct_buffer,
                                  int64_t* result_int_out, double* result_float_out) {
    if (state == NULL || result_int_out == NULL || result_float_out == NULL) {
        return -1;
    }
    
    if (state->param_types == NULL || state->param_values == NULL) {
        internal_log_write("ERROR", "Parameter arrays are NULL for interface");
        return -1;
    }
    
    state->in_use = 1;
    int64_t result_int = 0;
    double result_float = 0.0;
    int32_t call_result = pt_platform_safe_call(state->func_ptr, actual_param_count, 
                                                  (void*)state->param_types, state->param_values,
                                                  (void*)state->param_sizes,
                                                  return_type, return_size, &result_int, &result_float, struct_buffer);
    if (call_result != 0) {
        state->in_use = 0;
        internal_log_write("ERROR", "Call to interface failed (error=%d)", call_result);
        return -1;
    }
    
    *result_int_out = result_int;
    *result_float_out = result_float;
    return 0;
}

/**
 * @brief 记录返回值 / Log return value / Rückgabewert protokollieren
 * @param plugin_name 插件名称 / Plugin name / Plugin-Name
 * @param interface_name 接口名称 / Interface name / Schnittstellenname
 * @param return_type 返回值类型 / Return type / Rückgabetyp
 * @param return_size 返回值大小 / Return size / Rückgabegröße
 * @param result_int 整数结果 / Integer result / Ganzzahlergebnis
 * @param result_float 浮点数结果 / Float result / Gleitkommaergebnis
 * @param struct_buffer 结构体缓冲区 / Struct buffer / Strukturpuffer
 */
void log_return_value(const char* plugin_name, const char* interface_name, pt_return_type_t return_type,
                      size_t return_size, int64_t result_int, double result_float, void* struct_buffer) {
    if (return_type == PT_RETURN_TYPE_FLOAT) {
        internal_log_write("INFO", "Called %s.%s, result = %f (float)", plugin_name, interface_name, (float)result_float);
    } else if (return_type == PT_RETURN_TYPE_DOUBLE) {
        internal_log_write("INFO", "Called %s.%s, result = %lf (double)", plugin_name, interface_name, result_float);
    } else if (return_type == PT_RETURN_TYPE_STRUCT_PTR) {
        internal_log_write("INFO", "Called %s.%s, result = %p (struct pointer)", plugin_name, interface_name, (void*)result_int);
    } else if (return_type == PT_RETURN_TYPE_STRUCT_VAL) {
        internal_log_write("INFO", "Called %s.%s, result = %p (struct value, size=%zu)", plugin_name, interface_name, struct_buffer, return_size);
    } else {
        internal_log_write("INFO", "Called %s.%s, result = %lld (integer/pointer)", plugin_name, interface_name, (long long)result_int);
    }
}

/**
 * @brief 根据返回值类型选择传递参数 / Select transfer parameter by return type / Übertragungsparameter nach Rückgabetyp auswählen
 * @param return_type 返回值类型 / Return type / Rückgabetyp
 * @param return_size 返回值大小 / Return size / Rückgabegröße
 * @param result_int 整数结果 / Integer result / Ganzzahlergebnis
 * @param result_float 浮点数结果 / Float result / Gleitkommaergebnis
 * @param struct_buffer 结构体缓冲区 / Struct buffer / Strukturpuffer
 * @param ctx 上下文 / Context / Kontext
 * @return 传递参数指针 / Transfer parameter pointer / Übertragungsparameter-Zeiger
 */
void* select_transfer_parameter_by_return_type(pt_return_type_t return_type, size_t return_size,
                                                int64_t result_int, double result_float, void* struct_buffer,
                                                pointer_transfer_context_t* ctx) {
    if (return_type == PT_RETURN_TYPE_FLOAT || return_type == PT_RETURN_TYPE_DOUBLE) {
        internal_log_write("INFO", "Using float return value %lf for transfer", result_float);
        return &result_float;
    } else if (return_type == PT_RETURN_TYPE_STRUCT_VAL && struct_buffer != NULL) {
        internal_log_write("INFO", "Using struct return value (size=%zu) for transfer", return_size);
        return struct_buffer;
    } else if (return_type == PT_RETURN_TYPE_STRUCT_PTR) {
        ctx->stored_size = sizeof(void*);
        ctx->stored_type = NXLD_PARAM_TYPE_STRING;
        internal_log_write("INFO", "Using pointer return value %p for transfer", (void*)(intptr_t)result_int);
        return (void*)(intptr_t)result_int;
    } else {
        ctx->stored_size = sizeof(int64_t);
        ctx->stored_type = NXLD_PARAM_TYPE_INT64;
        internal_log_write("INFO", "Using integer/pointer return value %lld for transfer", (long long)result_int);
        return &result_int;
    }
}

