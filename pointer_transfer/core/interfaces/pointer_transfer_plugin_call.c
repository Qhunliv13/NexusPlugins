/**
 * @file pointer_transfer_plugin_call.c
 * @brief 插件调用接口实现 / Plugin Call Interface Implementation / Plugin-Aufruf-Schnittstellenimplementierung
 */

#include "pointer_transfer_plugin.h"
#include "pointer_transfer_types.h"
#include "pointer_transfer_context.h"
#include "pointer_transfer_utils.h"
#include "rules/core/pointer_transfer_rule_matcher.h"
#include <stdlib.h>
#include <string.h>

/**
 * @brief 调用目标插件接口 / Call target plugin interface / Ziel-Plugin-Schnittstelle aufrufen
 * @param source_plugin_name 源插件名称 / Source plugin name / Quell-Plugin-Name
 * @param source_interface_name 源接口名称 / Source interface name / Quell-Schnittstellenname
 * @param param_index 参数索引 / Parameter index / Parameterindex
 * @param param_value 参数值 / Parameter value / Parameterwert
 * @return 成功返回0，失败返回非0 / Returns 0 on success, non-zero on failure / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
 */
POINTER_TRANSFER_PLUGIN_EXPORT int POINTER_TRANSFER_PLUGIN_CALL CallPlugin(const char* source_plugin_name, const char* source_interface_name, int param_index, void* param_value) {
    if (source_plugin_name == NULL || source_interface_name == NULL) {
        internal_log_write("WARNING", "CallPlugin: invalid parameters");
        return -1;
    }
    
    internal_log_write("INFO", "CallPlugin: called with source_plugin=%s, source_interface=%s, param_index=%d", 
                  source_plugin_name, source_interface_name, param_index);
    
    pointer_transfer_context_t* ctx = get_global_context();
    
    /* 如果param_index >= 0，尝试从已调用接口的参数状态中获取值 / If param_index >= 0, try to get value from parameter state of already called interface / Wenn param_index >= 0, versuche Wert aus Parameterstatus der bereits aufgerufenen Schnittstelle zu erhalten */
    void* actual_param_value = param_value;
    if (param_index >= 0) {
        target_interface_state_t* source_state = find_interface_state(source_plugin_name, source_interface_name);
        if (source_state != NULL && source_state->param_ready != NULL && 
            param_index < source_state->param_count && source_state->param_ready[param_index] &&
            source_state->param_values != NULL && source_state->param_values[param_index] != NULL) {
            actual_param_value = source_state->param_values[param_index];
            internal_log_write("INFO", "CallPlugin: got parameter %d value from interface state for %s.%s", 
                          param_index, source_plugin_name, source_interface_name);
        } else {
            internal_log_write("WARNING", "CallPlugin: failed to get parameter %d value from interface state for %s.%s, using provided param_value", 
                          param_index, source_plugin_name, source_interface_name);
        }
    }
    
    size_t matched_count = 0;
    size_t success_count = 0;
    
    if (ctx->rule_count > 0 && ctx->rules != NULL) {
        /* 使用索引定位匹配规则 / Use index to locate matching rules / Index verwenden, um übereinstimmende Regeln zu lokalisieren */
        size_t start_index = 0;
        size_t end_index = 0;
        int use_index = find_rule_index_range(source_plugin_name, source_interface_name, param_index, &start_index, &end_index);
        
        if (use_index) {
            matched_count = apply_matched_rules_indexed(source_plugin_name, source_interface_name, 
                                                         param_index, actual_param_value, start_index, end_index, &success_count);
        } else {
            matched_count = apply_matched_rules_linear(source_plugin_name, source_interface_name, 
                                                        param_index, actual_param_value, &success_count);
        }
    }
    
    if (matched_count == 0) {
        internal_log_write("WARNING", "CallPlugin: no matching rule found for %s.%s[%d]. Transfer rules must be configured in .nxpt file", source_plugin_name, source_interface_name, param_index);
        return -1;
    }
    
    internal_log_write("INFO", "CallPlugin: processed %zu rules, %zu successful", matched_count, success_count);
    return (success_count > 0) ? 0 : -1;
}

