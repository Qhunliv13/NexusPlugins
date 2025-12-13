/**
 * @file pointer_transfer_interface_param.c
 * @brief 接口参数验证和处理 / Interface Parameter Validation and Processing / Schnittstellenparameter-Validierung und -Verarbeitung
 * 
 * 此文件现在作为参数处理的入口点，具体实现已拆分到：
 * - pointer_transfer_interface_param_validate.c: 参数验证
 * - pointer_transfer_interface_param_set.c: 参数设置
 * - pointer_transfer_interface_param_readiness.c: 参数就绪状态验证
 * - pointer_transfer_interface_param_count.c: 参数数量计算
 */

#include "pointer_transfer_interface.h"
#include "pointer_transfer_context.h"
#include "pointer_transfer_utils.h"
#include "pointer_transfer_types.h"
#include <stdio.h>
#include <string.h>
#include <limits.h>

/**
 * @brief 验证和设置参数值 / Validate and set parameter value / Parameterwert validieren und setzen
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int validate_and_set_parameter(const pointer_transfer_rule_t* rule, target_interface_state_t* state, void* ptr) {
    if (rule == NULL || state == NULL) {
        return -1;
    }
    
    if (validate_parameter_pointer(rule, state, ptr) != 0) {
        return -1;
    }
    
    if (validate_parameter_index(rule, state) != 0) {
        return -1;
    }
    
    if (validate_parameter_arrays(rule, state) != 0) {
        return -1;
    }
    
    if (state->param_count > 0 && rule->target_param_index < state->param_count) {
        if (rule->target_param_value != NULL && strlen(rule->target_param_value) > 0) {
            if (set_parameter_from_const_string(rule, state, ptr) != 0) {
                return -1;
            }
        } else {
            if (set_parameter_from_pointer(rule, state, ptr) != 0) {
                return -1;
            }
        }
    }
    
    if (apply_constant_value_rules(rule, state) != 0) {
        return -1;
    }
    
    return 0;
}

/**
 * @brief 验证参数就绪状态 / Validate parameter readiness / Parameterbereitschaft validieren
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int validate_parameter_readiness(const pointer_transfer_rule_t* rule, target_interface_state_t* state) {
    if (rule == NULL || state == NULL) {
        return -1;
    }
    
    if (state->is_variadic) {
        return validate_variadic_parameter_readiness(rule, state);
    } else {
        return validate_fixed_parameter_readiness(rule, state);
    }
}

/**
 * @brief 计算实际参数数量 / Calculate actual parameter count / Tatsächliche Parameteranzahl berechnen
 * @return 实际参数数量 / Actual parameter count / Tatsächliche Parameteranzahl
 */
int calculate_actual_param_count(const pointer_transfer_rule_t* rule, target_interface_state_t* state) {
    if (state == NULL) {
        return 0;
    }
    
    int actual_param_count = state->param_count;
    
    if (state->is_variadic) {
        int base_count = calculate_variadic_base_ready_count(state);
        actual_param_count = update_variadic_count_from_extra_rules(rule, state, base_count);
        validate_and_log_variadic_count(rule, state, actual_param_count);
    }
    
    return actual_param_count;
}

