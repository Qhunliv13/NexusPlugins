/**
 * @file pointer_transfer_interface_state_find.c
 * @brief 接口状态查找 / Interface State Finding / Schnittstellenstatus-Suche
 */

#include "pointer_transfer_interface.h"
#include "pointer_transfer_context.h"
#include <string.h>

/**
 * @brief 查找目标接口状态（不创建）/ Find target interface state (without creating) / Ziel-Schnittstellenstatus suchen (ohne Erstellung)
 */
target_interface_state_t* find_interface_state(const char* plugin_name, const char* interface_name) {
    if (plugin_name == NULL || interface_name == NULL) {
        return NULL;
    }
    
    pointer_transfer_context_t* ctx = get_global_context();
    if (ctx->interface_states != NULL) {
        for (size_t i = 0; i < ctx->interface_state_count; i++) {
            target_interface_state_t* state = &ctx->interface_states[i];
            if (state->plugin_name != NULL && state->interface_name != NULL &&
                strcmp(state->plugin_name, plugin_name) == 0 &&
                strcmp(state->interface_name, interface_name) == 0) {
                return state;
            }
        }
    }
    
    return NULL;
}

