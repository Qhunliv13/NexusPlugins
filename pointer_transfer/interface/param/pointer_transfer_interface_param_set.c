/**
 * @file pointer_transfer_interface_param_set.c
 * @brief 参数设置 / Parameter Setting / Parameter-Einstellung
 */

#include "pointer_transfer_interface.h"
#include "pointer_transfer_context.h"
#include "pointer_transfer_utils.h"
#include <string.h>

/**
 * @brief 设置参数值（从常量字符串） / Set parameter value from constant string / Parameterwert aus Konstantenstring setzen
 * @param rule 传递规则 / Transfer rule / Übertragungsregel
 * @param state 接口状态 / Interface state / Schnittstellenstatus
 * @param ptr 指针值（用于回退） / Pointer value (for fallback) / Zeigerwert (für Fallback)
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int set_parameter_from_const_string(const pointer_transfer_rule_t* rule, target_interface_state_t* state, void* ptr) {
    if (rule == NULL || state == NULL) {
        return -1;
    }
    
    if (rule->target_param_value == NULL || strlen(rule->target_param_value) == 0) {
        return 0;
    }
    
    if (!set_parameter_value_from_const_string((struct target_interface_state_s*)state, rule->target_param_index, 
                                                rule->target_param_value, rule->target_plugin, rule->target_interface)) {
        internal_log_write("WARNING", "Failed to parse constant value for parameter %d of %s.%s, falling back to pointer", 
                    rule->target_param_index, rule->target_plugin, rule->target_interface);
        /* 回退到指针设置，确保所有字段一致 / Fallback to pointer setting, ensure all fields are consistent / Fallback auf Zeigereinstellung, sicherstellen, dass alle Felder konsistent sind */
        pointer_transfer_context_t* ctx = get_global_context();
        state->param_values[rule->target_param_index] = ptr;
        state->param_ready[rule->target_param_index] = 1;
        if (state->param_sizes != NULL) {
            state->param_sizes[rule->target_param_index] = ctx->stored_size > 0 ? ctx->stored_size : sizeof(void*);
        }
    }
    
    return 0;
}

/**
 * @brief 设置参数值（从指针） / Set parameter value from pointer / Parameterwert aus Zeiger setzen
 * @param rule 传递规则 / Transfer rule / Übertragungsregel
 * @param state 接口状态 / Interface state / Schnittstellenstatus
 * @param ptr 指针值 / Pointer value / Zeigerwert
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int set_parameter_from_pointer(const pointer_transfer_rule_t* rule, target_interface_state_t* state, void* ptr) {
    if (rule == NULL || state == NULL) {
        return -1;
    }
    
    pointer_transfer_context_t* ctx = get_global_context();
    set_parameter_value_from_pointer((struct target_interface_state_s*)state, rule->target_param_index, 
                                    ptr, ctx->stored_size, rule->target_plugin, rule->target_interface);
    
    return 0;
}

/**
 * @brief 应用常量值规则 / Apply constant value rules / Konstantwert-Regeln anwenden
 * @param rule 传递规则 / Transfer rule / Übertragungsregel
 * @param state 接口状态 / Interface state / Schnittstellenstatus
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int apply_constant_value_rules(const pointer_transfer_rule_t* rule, target_interface_state_t* state) {
    if (rule == NULL || state == NULL) {
        return -1;
    }
    
    pointer_transfer_context_t* ctx = get_global_context();
    
    if (ctx->rules == NULL) {
        return 0;
    }
    
    for (size_t i = 0; i < ctx->rule_count; i++) {
        pointer_transfer_rule_t* const_rule = &ctx->rules[i];
        if (!const_rule->enabled || const_rule->target_param_value == NULL || strlen(const_rule->target_param_value) == 0) {
            continue;
        }
        if (const_rule->target_plugin != NULL && const_rule->target_interface != NULL &&
            strcmp(const_rule->target_plugin, rule->target_plugin) == 0 &&
            strcmp(const_rule->target_interface, rule->target_interface) == 0) {
            if (const_rule->target_param_index >= 0 && const_rule->target_param_index < state->param_count &&
                state->param_ready != NULL && !state->param_ready[const_rule->target_param_index]) {
                if (!set_parameter_value_from_const_string((struct target_interface_state_s*)state, const_rule->target_param_index, 
                                                            const_rule->target_param_value, const_rule->target_plugin, const_rule->target_interface)) {
                    internal_log_write("WARNING", "Failed to parse constant value for parameter %d of %s.%s", 
                        const_rule->target_param_index, const_rule->target_plugin, const_rule->target_interface);
                }
            }
        }
    }
    
    return 0;
}

