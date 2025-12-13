/**
 * @file pointer_transfer_interface_validate.c
 * @brief 接口调用验证 / Interface Call Validation / Schnittstellenaufruf-Validierung
 */

#include "pointer_transfer_interface.h"
#include "pointer_transfer_context.h"
#include "pointer_transfer_types.h"
#include "pointer_transfer_currying.h"
#include "pointer_transfer_utils.h"
#include <stdlib.h>

/**
 * @brief 验证可变参数接口的最小参数要求 / Validate minimum parameter requirement for variadic interface / Mindestparameteranforderung für variablen Parameter-Interface validieren
 * @param state 接口状态 / Interface state / Schnittstellenstatus
 * @param actual_param_count 实际参数数量 / Actual parameter count / Tatsächliche Parameteranzahl
 * @param plugin_name 插件名称 / Plugin name / Plugin-Name
 * @param interface_name 接口名称 / Interface name / Schnittstellenname
 * @return 验证通过返回0，失败返回-1 / Returns 0 if validation passes, -1 on failure / Gibt 0 zurück, wenn Validierung erfolgreich, -1 bei Fehler
 */
int validate_variadic_min_param_requirement(target_interface_state_t* state, int actual_param_count,
                                             const char* plugin_name, const char* interface_name) {
    if (state == NULL || plugin_name == NULL || interface_name == NULL) {
        return -1;
    }
    
    if (state->is_variadic && actual_param_count < state->min_param_count) {
        internal_log_write("ERROR", "Cannot call %s.%s: actual_param_count=%d is less than min_param_count=%d", 
                     plugin_name, interface_name, actual_param_count, state->min_param_count);
        return -1;
    }
    
    return 0;
}

/**
 * @brief 验证插件函数 / Validate plugin function / Plugin-Funktion validieren
 * @param state 接口状态 / Interface state / Schnittstellenstatus
 * @param plugin_path 插件路径 / Plugin path / Plugin-Pfad
 * @param interface_name 接口名称 / Interface name / Schnittstellenname
 * @param actual_param_count 实际参数数量 / Actual parameter count / Tatsächliche Parameteranzahl
 * @param return_type 返回值类型 / Return type / Rückgabetyp
 * @return 验证通过返回0，失败返回-1 / Returns 0 if validation passes, -1 on failure / Gibt 0 zurück, wenn Validierung erfolgreich, -1 bei Fehler
 */
int validate_plugin_function(target_interface_state_t* state, const char* plugin_path, const char* interface_name,
                              int actual_param_count, pt_return_type_t return_type) {
    if (state == NULL || plugin_path == NULL || interface_name == NULL) {
        return -1;
    }
    
    if (state->validation_done) {
        return 0;
    }
    
    int32_t validation_result = pt_validate_plugin_function(state->func_ptr, plugin_path, 
                                                             interface_name, actual_param_count, return_type);
    if (validation_result != 0) {
        internal_log_write("ERROR", "Plugin function validation failed: function validation returned error");
        return -1;
    }
    
    state->validation_done = 1;
    internal_log_write("INFO", "Plugin function validation passed: compatible with currying API");
    
    return 0;
}

