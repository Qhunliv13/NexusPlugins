/**
 * @file pointer_transfer_interface_cleanup.c
 * @brief 接口状态清理 / Interface State Cleanup / Schnittstellenstatus-Bereinigung
 */

#include "pointer_transfer_interface.h"
#include "pointer_transfer_context.h"
#include "pointer_transfer_utils.h"

/**
 * @brief 清理接口状态参数 / Cleanup interface state parameters / Schnittstellenstatus-Parameter bereinigen
 * @param state 接口状态 / Interface state / Schnittstellenstatus
 */
void cleanup_interface_state_parameters(target_interface_state_t* state) {
    if (state == NULL) {
        return;
    }
    
    state->in_use = 0;
    
    if (state->param_count > 0 && state->param_ready != NULL && state->param_values != NULL) {
        for (int i = 0; i < state->param_count; i++) {
            state->param_ready[i] = 0;
            state->param_values[i] = NULL;
        }
    }
}

/**
 * @brief 清理SetGroup目标接口的参数状态 / Cleanup parameter state of SetGroup target interface / Parameterstatus der SetGroup-Zielschnittstelle bereinigen
 * @param ctx 上下文 / Context / Kontext
 * @param plugin_name 插件名称 / Plugin name / Plugin-Name
 * @param interface_name 接口名称 / Interface name / Schnittstellenname
 */
void cleanup_setgroup_target_interface_parameters(pointer_transfer_context_t* ctx, const char* plugin_name, const char* interface_name) {
    if (ctx == NULL || plugin_name == NULL || interface_name == NULL) {
        return;
    }
    
    target_interface_state_t* target_state = find_interface_state(plugin_name, interface_name);
    if (target_state != NULL && target_state->param_count > 0 && 
        target_state->param_ready != NULL && target_state->param_values != NULL) {
        for (int i = 0; i < target_state->param_count; i++) {
            target_state->param_ready[i] = 0;
            target_state->param_values[i] = NULL;
        }
        internal_log_write("INFO", "Cleaned up parameter state for SetGroup target interface %s.%s", 
                      plugin_name, interface_name);
    }
}

