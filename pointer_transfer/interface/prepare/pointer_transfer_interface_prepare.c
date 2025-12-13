/**
 * @file pointer_transfer_interface_prepare.c
 * @brief 接口调用准备 / Interface Call Preparation / Schnittstellenaufruf-Vorbereitung
 */

#include "pointer_transfer_interface.h"
#include "pointer_transfer_context.h"
#include "pointer_transfer_utils.h"
#include <stdlib.h>

/**
 * @brief 准备接口调用 / Prepare interface call / Schnittstellenaufruf vorbereiten
 * @param rule 传递规则 / Transfer rule / Übertragungsregel
 * @param ptr 要传递的指针 / Pointer to transfer / Zu übertragender Zeiger
 * @param state_out 输出接口状态 / Output interface state / Ausgabe-Schnittstellenstatus
 * @param actual_param_count_out 输出实际参数数量 / Output actual parameter count / Ausgabe-Tatsächliche Parameteranzahl
 * @param return_type_out 输出返回值类型 / Output return type / Ausgabe-Rückgabetyp
 * @param return_size_out 输出返回值大小 / Output return size / Ausgabe-Rückgabegröße
 * @param struct_buffer_out 输出结构体缓冲区 / Output struct buffer / Ausgabe-Strukturpuffer
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int prepare_interface_call(const pointer_transfer_rule_t* rule, void* ptr,
                           target_interface_state_t** state_out, int* actual_param_count_out,
                           pt_return_type_t* return_type_out, size_t* return_size_out, void** struct_buffer_out) {
    if (rule == NULL || rule->target_plugin == NULL || rule->target_interface == NULL ||
        state_out == NULL || actual_param_count_out == NULL || return_type_out == NULL ||
        return_size_out == NULL || struct_buffer_out == NULL) {
        return -1;
    }
    
    void* handle = NULL;
    void* func_ptr = NULL;
    if (load_plugin_and_get_function(rule, &handle, &func_ptr) != 0) {
        return -1;
    }
    
    target_interface_state_t* state = find_or_create_interface_state(rule->target_plugin, rule->target_interface, handle, func_ptr);
    if (state == NULL) {
        internal_log_write("ERROR", "Failed to create interface state for %s.%s", rule->target_plugin, rule->target_interface);
        return -1;
    }
    
    if (validate_and_set_parameter(rule, state, ptr) != 0) {
        return -1;
    }
    
    if (validate_parameter_readiness(rule, state) != 0) {
        return -1;
    }
    
    int actual_param_count = calculate_actual_param_count(rule, state);
    
    pt_return_type_t return_type;
    size_t return_size;
    void* struct_buffer = NULL;
    if (prepare_return_type_and_buffer(state, &return_type, &return_size, &struct_buffer) != 0) {
        return -1;
    }
    
    if (validate_variadic_min_param_requirement(state, actual_param_count, rule->target_plugin, rule->target_interface) != 0) {
        if (struct_buffer != NULL) {
            free(struct_buffer);
        }
        return -1;
    }
    
    *state_out = state;
    *actual_param_count_out = actual_param_count;
    *return_type_out = return_type;
    *return_size_out = return_size;
    *struct_buffer_out = struct_buffer;
    
    return 0;
}

/**
 * @brief 执行接口调用 / Execute interface call / Schnittstellenaufruf ausführen
 * @param state 接口状态 / Interface state / Schnittstellenstatus
 * @param rule 传递规则 / Transfer rule / Übertragungsregel
 * @param actual_param_count 实际参数数量 / Actual parameter count / Tatsächliche Parameteranzahl
 * @param return_type 返回值类型 / Return type / Rückgabetyp
 * @param return_size 返回值大小 / Return size / Rückgabegröße
 * @param struct_buffer 结构体缓冲区 / Struct buffer / Strukturpuffer
 * @param result_int_out 输出整数结果 / Output integer result / Ausgabe-Ganzzahlergebnis
 * @param result_float_out 输出浮点数结果 / Output float result / Ausgabe-Gleitkommaergebnis
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int execute_interface_call(target_interface_state_t* state, const pointer_transfer_rule_t* rule,
                           int actual_param_count, pt_return_type_t return_type, size_t return_size, void* struct_buffer,
                           int64_t* result_int_out, double* result_float_out) {
    if (state == NULL || rule == NULL || result_int_out == NULL || result_float_out == NULL) {
        return -1;
    }
    
    internal_log_write("INFO", "Calling %s.%s (param_count=%d, return_type=%d, return_size=%zu)", 
                 rule->target_plugin, rule->target_interface, actual_param_count, return_type, return_size);
    
    if (validate_plugin_function(state, rule->target_plugin_path, rule->target_interface, actual_param_count, return_type) != 0) {
        return -1;
    }
    
    if (call_function_and_get_result(state, actual_param_count, return_type, return_size, struct_buffer,
                                     result_int_out, result_float_out) != 0) {
        return -1;
    }
    
    log_return_value(rule->target_plugin, rule->target_interface, return_type, return_size,
                     *result_int_out, *result_float_out, struct_buffer);
    
    return 0;
}

