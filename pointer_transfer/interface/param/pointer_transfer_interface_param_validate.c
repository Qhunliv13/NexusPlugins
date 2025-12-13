/**
 * @file pointer_transfer_interface_param_validate.c
 * @brief 参数验证 / Parameter Validation / Parameter-Validierung
 */

#include "pointer_transfer_interface.h"
#include "pointer_transfer_utils.h"
#include <string.h>

/**
 * @brief 验证参数索引 / Validate parameter index / Parameterindex validieren
 * @param rule 传递规则 / Transfer rule / Übertragungsregel
 * @param state 接口状态 / Interface state / Schnittstellenstatus
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int validate_parameter_index(const pointer_transfer_rule_t* rule, target_interface_state_t* state) {
    if (rule == NULL || state == NULL) {
        return -1;
    }
    
    if (state->param_count > 0) {
        if (rule->target_param_index < 0 || (!state->is_variadic && rule->target_param_index >= state->param_count)) {
            internal_log_write("ERROR", "Invalid parameter index %d for interface %s.%s (param_count=%d, is_variadic=%d)",
                          rule->target_param_index, rule->target_plugin, rule->target_interface, state->param_count, state->is_variadic);
            return -1;
        }
    }
    
    return 0;
}

/**
 * @brief 验证参数指针 / Validate parameter pointer / Parameterzeiger validieren
 * @param rule 传递规则 / Transfer rule / Übertragungsregel
 * @param state 接口状态 / Interface state / Schnittstellenstatus
 * @param ptr 指针值 / Pointer value / Zeigerwert
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int validate_parameter_pointer(const pointer_transfer_rule_t* rule, target_interface_state_t* state, void* ptr) {
    if (rule == NULL || state == NULL) {
        return -1;
    }
    
    if (state->param_count > 0) {
        if (ptr == NULL && (rule->target_param_value == NULL || strlen(rule->target_param_value) == 0)) {
            internal_log_write("ERROR", "Invalid NULL pointer for interface %s.%s with %d parameters", 
                          rule->target_plugin, rule->target_interface, state->param_count);
            return -1;
        }
    }
    
    return 0;
}

/**
 * @brief 验证参数数组 / Validate parameter arrays / Parameterarrays validieren
 * @param rule 传递规则 / Transfer rule / Übertragungsregel
 * @param state 接口状态 / Interface state / Schnittstellenstatus
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int validate_parameter_arrays(const pointer_transfer_rule_t* rule, target_interface_state_t* state) {
    if (rule == NULL || state == NULL) {
        return -1;
    }
    
    if (state->param_count > 0 && rule->target_param_index < state->param_count) {
        if (state->param_values == NULL || state->param_ready == NULL) {
            internal_log_write("ERROR", "Parameter arrays are NULL for interface %s.%s", 
                          rule->target_plugin, rule->target_interface);
            return -1;
        }
    }
    
    return 0;
}

