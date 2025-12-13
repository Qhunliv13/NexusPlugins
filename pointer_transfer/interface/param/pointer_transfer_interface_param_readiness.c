/**
 * @file pointer_transfer_interface_param_readiness.c
 * @brief 参数就绪状态验证 / Parameter Readiness Validation / Parameterbereitschaft-Validierung
 */

#include "pointer_transfer_interface.h"
#include "pointer_transfer_utils.h"
#include <stdio.h>
#include <string.h>

/**
 * @brief 计算可变参数的实际就绪数量 / Calculate actual ready count for variadic parameters / Tatsächliche bereite Anzahl für variable Parameter berechnen
 * @param state 接口状态 / Interface state / Schnittstellenstatus
 * @return 实际就绪的参数数量 / Actual ready parameter count / Tatsächliche bereite Parameteranzahl
 */
int calculate_variadic_ready_count(target_interface_state_t* state) {
    if (state == NULL || state->param_ready == NULL) {
        return 0;
    }
    
    int actual_ready_count = 0;
    for (int i = 0; i < state->param_count; i++) {
        if (state->param_ready[i]) {
            actual_ready_count = i + 1;
        } else {
            break;
        }
    }
    
    return actual_ready_count;
}

/**
 * @brief 构建未就绪参数列表字符串 / Build unready parameters list string / Liste der nicht bereiten Parameter als String erstellen
 * @param state 接口状态 / Interface state / Schnittstellenstatus
 * @param start_index 开始索引 / Start index / Startindex
 * @param end_index 结束索引 / End index / Endindex
 * @param unready_params_out 输出未就绪参数字符串 / Output unready parameters string / Ausgabe-String der nicht bereiten Parameter
 * @param max_size 最大大小 / Maximum size / Maximale Größe
 */
void build_unready_params_string(target_interface_state_t* state, int start_index, int end_index,
                                  char* unready_params_out, size_t max_size) {
    if (state == NULL || unready_params_out == NULL || max_size == 0) {
        return;
    }
    
    unready_params_out[0] = '\0';
    int first = 1;
    size_t pos = 0;
    
    for (int i = start_index; i < end_index && pos < max_size - 1; i++) {
        int should_include = 0;
        if (state->is_variadic) {
            should_include = 1;
        } else {
            if (state->param_ready != NULL && i < state->param_count && !state->param_ready[i]) {
                should_include = 1;
            }
        }
        
        if (should_include) {
            if (!first && pos < max_size - 3) {
                unready_params_out[pos++] = ',';
                unready_params_out[pos++] = ' ';
                unready_params_out[pos] = '\0';
            }
            char param_str[32];
            snprintf(param_str, sizeof(param_str), "%d", i);
            size_t param_len = strlen(param_str);
            if (pos + param_len < max_size - 1) {
                memcpy(unready_params_out + pos, param_str, param_len);
                pos += param_len;
                unready_params_out[pos] = '\0';
            }
            first = 0;
        }
    }
}

/**
 * @brief 验证可变参数就绪状态 / Validate variadic parameter readiness / Variable Parameterbereitschaft validieren
 * @param rule 传递规则 / Transfer rule / Übertragungsregel
 * @param state 接口状态 / Interface state / Schnittstellenstatus
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int validate_variadic_parameter_readiness(const pointer_transfer_rule_t* rule, target_interface_state_t* state) {
    if (rule == NULL || state == NULL) {
        return -1;
    }
    
    int actual_ready_count = calculate_variadic_ready_count(state);
    
    if (actual_ready_count < state->min_param_count) {
        char unready_params[256] = {0};
        build_unready_params_string(state, actual_ready_count, state->min_param_count, unready_params, sizeof(unready_params));
        internal_log_write("WARNING", "Required parameters not all ready for %s.%s (min_required=%d, actual_ready=%d, unready_params=[%s], is_variadic=1)", 
                     rule->target_plugin, rule->target_interface, state->min_param_count, actual_ready_count, unready_params);
        return -1;
    }
    
    return 0;
}

/**
 * @brief 验证固定参数就绪状态 / Validate fixed parameter readiness / Feste Parameterbereitschaft validieren
 * @param rule 传递规则 / Transfer rule / Übertragungsregel
 * @param state 接口状态 / Interface state / Schnittstellenstatus
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int validate_fixed_parameter_readiness(const pointer_transfer_rule_t* rule, target_interface_state_t* state) {
    if (rule == NULL || state == NULL) {
        return -1;
    }
    
    if (state->param_ready == NULL) {
        internal_log_write("ERROR", "param_ready array is NULL for interface %s.%s", 
                      rule->target_plugin, rule->target_interface);
        return -1;
    }
    
    int required_param_count = state->param_count;
    int all_required_ready = 1;
    for (int i = 0; i < required_param_count; i++) {
        if (!state->param_ready[i]) {
            all_required_ready = 0;
            break;
        }
    }
    
    if (!all_required_ready) {
        char unready_params[256] = {0};
        build_unready_params_string(state, 0, required_param_count, unready_params, sizeof(unready_params));
        internal_log_write("WARNING", "Required parameters not all ready for %s.%s (required_count=%d, unready_params=[%s], is_variadic=%d)", 
                  rule->target_plugin, rule->target_interface, required_param_count, unready_params, state->is_variadic);
        return -1;
    }
    
    return 0;
}

