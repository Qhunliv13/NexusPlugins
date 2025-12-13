/**
 * @file pointer_transfer_context_capacity_states.c
 * @brief 接口状态数组容量扩展操作 / Interface States Array Capacity Expansion Operations / Schnittstellen-Status-Array-Kapazitätserweiterungsoperationen
 */

#include "pointer_transfer_context.h"
#include "pointer_transfer_utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

/**
 * @brief 扩展接口状态数组容量 / Expand interface states array capacity / Schnittstellen-Status-Array-Kapazität erweitern
 * @return 成功返回0，失败返回非0 / Returns 0 on success, non-zero on failure / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
 */
int expand_interface_states_capacity(void) {
    pointer_transfer_context_t* ctx = get_global_context();
    if (ctx == NULL) {
        internal_log_write("ERROR", "expand_interface_states_capacity: global context is NULL");
        return -1;
    }
    
    size_t new_capacity = ctx->interface_state_capacity == 0 ? INITIAL_INTERFACE_STATE_CAPACITY : ctx->interface_state_capacity * CAPACITY_GROWTH_FACTOR;
    
    /* 检查容量溢出 / Check capacity overflow / Kapazitätsüberlauf prüfen */
    if (new_capacity < ctx->interface_state_capacity || new_capacity > SIZE_MAX / sizeof(target_interface_state_t)) {
        internal_log_write("ERROR", "expand_interface_states_capacity: capacity overflow detected (current=%zu, new=%zu)", 
                          ctx->interface_state_capacity, new_capacity);
        return -1;
    }
    
    target_interface_state_t* new_states = (target_interface_state_t*)realloc(ctx->interface_states, new_capacity * sizeof(target_interface_state_t));
    if (new_states == NULL) {
        internal_log_write("ERROR", "expand_interface_states_capacity: failed to allocate memory for interface states array (new_capacity=%zu)", new_capacity);
        return -1;
    }
    
    memset(new_states + ctx->interface_state_count, 0, (new_capacity - ctx->interface_state_count) * sizeof(target_interface_state_t));
    size_t old_capacity = ctx->interface_state_capacity;
    ctx->interface_states = new_states;
    ctx->interface_state_capacity = new_capacity;
    
    internal_log_write("INFO", "expand_interface_states_capacity: expanded to %zu (was %zu)", new_capacity, old_capacity);
    return 0;
}

