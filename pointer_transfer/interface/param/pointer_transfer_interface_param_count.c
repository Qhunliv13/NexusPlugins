/**
 * @file pointer_transfer_interface_param_count.c
 * @brief 参数数量计算 / Parameter Count Calculation / Parameteranzahl-Berechnung
 */

#include "pointer_transfer_interface.h"
#include "pointer_transfer_context.h"
#include "pointer_transfer_utils.h"
#include <string.h>
#include <limits.h>

/**
 * @brief 计算可变参数的基础就绪数量 / Calculate base ready count for variadic parameters / Basis-bereite Anzahl für variable Parameter berechnen
 * @param state 接口状态 / Interface state / Schnittstellenstatus
 * @return 基础就绪数量 / Base ready count / Basis-bereite Anzahl
 */
int calculate_variadic_base_ready_count(target_interface_state_t* state) {
    if (state == NULL || state->param_ready == NULL) {
        return 0;
    }
    
    int actual_param_count = 0;
    for (int i = 0; i < state->param_count; i++) {
        if (state->param_ready[i]) {
            actual_param_count = i + 1;
        } else {
            break;
        }
    }
    
    return actual_param_count;
}

/**
 * @brief 检查中间参数是否就绪 / Check if intermediate parameters are ready / Prüfen, ob Zwischenparameter bereit sind
 * @param state 接口状态 / Interface state / Schnittstellenstatus
 * @param start_index 开始索引 / Start index / Startindex
 * @param end_index 结束索引 / End index / Endindex
 * @return 全部就绪返回1，否则返回0 / Returns 1 if all ready, 0 otherwise / Gibt 1 zurück, wenn alle bereit, sonst 0
 */
int check_intermediate_parameters_ready(target_interface_state_t* state, int start_index, int end_index) {
    if (state == NULL || state->param_ready == NULL) {
        return 0;
    }
    
    for (int j = start_index; j < end_index; j++) {
        if (j < state->param_count && !state->param_ready[j]) {
            return 0;
        }
    }
    
    return 1;
}

/**
 * @brief 更新可变参数数量（基于额外规则） / Update variadic parameter count based on extra rules / Variable Parameteranzahl basierend auf zusätzlichen Regeln aktualisieren
 * @param rule 传递规则 / Transfer rule / Übertragungsregel
 * @param state 接口状态 / Interface state / Schnittstellenstatus
 * @param base_count 基础数量 / Base count / Basisanzahl
 * @return 更新后的数量 / Updated count / Aktualisierte Anzahl
 */
int update_variadic_count_from_extra_rules(const pointer_transfer_rule_t* rule, target_interface_state_t* state, int base_count) {
    if (rule == NULL || state == NULL) {
        return base_count;
    }
    
    pointer_transfer_context_t* ctx = get_global_context();
    int actual_param_count = base_count;
    
    if (ctx->rules == NULL || state->param_ready == NULL) {
        return actual_param_count;
    }
    
    for (size_t i = 0; i < ctx->rule_count; i++) {
        pointer_transfer_rule_t* extra_rule = &ctx->rules[i];
        if (extra_rule->enabled && 
            extra_rule->target_plugin != NULL && 
            extra_rule->target_interface != NULL &&
            strcmp(extra_rule->target_plugin, rule->target_plugin) == 0 &&
            strcmp(extra_rule->target_interface, rule->target_interface) == 0 &&
            extra_rule->target_param_index >= actual_param_count) {
            
            if (check_intermediate_parameters_ready(state, actual_param_count, extra_rule->target_param_index)) {
                if (extra_rule->target_param_index == INT_MAX || extra_rule->target_param_index < 0) {
                    internal_log_write("ERROR", "Invalid parameter index detected for %s.%s: target_param_index=%d, skipping this rule", 
                                 rule->target_plugin, rule->target_interface, extra_rule->target_param_index);
                } else if (extra_rule->target_param_index >= state->param_count) {
                    internal_log_write("WARNING", "Parameter index %d exceeds param_count %d for %s.%s, limiting to param_count", 
                                 extra_rule->target_param_index, state->param_count, rule->target_plugin, rule->target_interface);
                    actual_param_count = state->param_count;
                } else {
                    actual_param_count = extra_rule->target_param_index + 1;
                }
            } else {
                internal_log_write("WARNING", "Variadic parameter gap detected for %s.%s: parameter %d is set but intermediate parameters are not ready (actual_param_count=%d)", 
                             rule->target_plugin, rule->target_interface, extra_rule->target_param_index, actual_param_count);
            }
        }
    }
    
    return actual_param_count;
}

/**
 * @brief 验证并记录可变参数数量 / Validate and log variadic parameter count / Variable Parameteranzahl validieren und protokollieren
 * @param rule 传递规则 / Transfer rule / Übertragungsregel
 * @param state 接口状态 / Interface state / Schnittstellenstatus
 * @param actual_param_count 实际参数数量 / Actual parameter count / Tatsächliche Parameteranzahl
 */
void validate_and_log_variadic_count(const pointer_transfer_rule_t* rule, target_interface_state_t* state, int actual_param_count) {
    if (rule == NULL || state == NULL) {
        return;
    }
    
    state->actual_param_count = actual_param_count;
    
    if (actual_param_count < state->min_param_count) {
        internal_log_write("WARNING", "Variadic interface %s.%s: actual_param_count=%d is less than min_param_count=%d, this may cause call failure", 
                     rule->target_plugin, rule->target_interface, actual_param_count, state->min_param_count);
    }
    
    internal_log_write("INFO", "Variadic interface %s.%s: using %d parameters (min_required=%d, max_available=%d)", 
                 rule->target_plugin, rule->target_interface, actual_param_count, state->min_param_count, state->param_count);
}

