/**
 * @file pointer_transfer_context_capacity.c
 * @brief 上下文容量扩展操作 / Context Capacity Expansion Operations / Kontext-Kapazitätserweiterungsoperationen
 */

#include "pointer_transfer_context.h"
#include "pointer_transfer_utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

/**
 * @brief 扩展规则数组容量 / Expand rules array capacity / Regel-Array-Kapazität erweitern
 * @return 成功返回0，失败返回非0 / Returns 0 on success, non-zero on failure / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
 */
int expand_rules_capacity(void) {
    pointer_transfer_context_t* ctx = get_global_context();
    if (ctx == NULL) {
        internal_log_write("ERROR", "expand_rules_capacity: global context is NULL");
        return -1;
    }
    
    size_t new_capacity = ctx->rule_capacity == 0 ? INITIAL_RULE_CAPACITY : ctx->rule_capacity * CAPACITY_GROWTH_FACTOR;
    
    /* 检查容量溢出 / Check capacity overflow / Kapazitätsüberlauf prüfen */
    if (new_capacity < ctx->rule_capacity || new_capacity > SIZE_MAX / sizeof(pointer_transfer_rule_t)) {
        internal_log_write("ERROR", "expand_rules_capacity: capacity overflow detected (current=%zu, new=%zu)", 
                          ctx->rule_capacity, new_capacity);
        return -1;
    }
    
    pointer_transfer_rule_t* new_rules = (pointer_transfer_rule_t*)realloc(ctx->rules, new_capacity * sizeof(pointer_transfer_rule_t));
    if (new_rules == NULL) {
        internal_log_write("ERROR", "expand_rules_capacity: failed to allocate memory for rules array (new_capacity=%zu)", new_capacity);
        return -1;
    }
    
    memset(new_rules + ctx->rule_count, 0, (new_capacity - ctx->rule_count) * sizeof(pointer_transfer_rule_t));
    size_t old_capacity = ctx->rule_capacity;
    ctx->rules = new_rules;
    ctx->rule_capacity = new_capacity;
    
    internal_log_write("INFO", "expand_rules_capacity: expanded to %zu (was %zu)", new_capacity, old_capacity);
    return 0;
}

/**
 * @brief 扩展已加载插件数组容量 / Expand loaded plugins array capacity / Geladenes Plugin-Array-Kapazität erweitern
 * @return 成功返回0，失败返回非0 / Returns 0 on success, non-zero on failure / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
 */
int expand_loaded_plugins_capacity(void) {
    pointer_transfer_context_t* ctx = get_global_context();
    if (ctx == NULL) {
        internal_log_write("ERROR", "expand_loaded_plugins_capacity: global context is NULL");
        return -1;
    }
    
    size_t new_capacity = ctx->loaded_plugin_capacity == 0 ? INITIAL_PLUGIN_CAPACITY : ctx->loaded_plugin_capacity * CAPACITY_GROWTH_FACTOR;
    
    /* 检查容量溢出 / Check capacity overflow / Kapazitätsüberlauf prüfen */
    if (new_capacity < ctx->loaded_plugin_capacity || new_capacity > SIZE_MAX / sizeof(loaded_plugin_info_t)) {
        internal_log_write("ERROR", "expand_loaded_plugins_capacity: capacity overflow detected (current=%zu, new=%zu)", 
                          ctx->loaded_plugin_capacity, new_capacity);
        return -1;
    }
    
    loaded_plugin_info_t* new_plugins = (loaded_plugin_info_t*)realloc(ctx->loaded_plugins, new_capacity * sizeof(loaded_plugin_info_t));
    if (new_plugins == NULL) {
        internal_log_write("ERROR", "expand_loaded_plugins_capacity: failed to allocate memory for plugins array (new_capacity=%zu)", new_capacity);
        return -1;
    }
    
    memset(new_plugins + ctx->loaded_plugin_count, 0, (new_capacity - ctx->loaded_plugin_count) * sizeof(loaded_plugin_info_t));
    size_t old_capacity = ctx->loaded_plugin_capacity;
    ctx->loaded_plugins = new_plugins;
    ctx->loaded_plugin_capacity = new_capacity;
    
    internal_log_write("INFO", "expand_loaded_plugins_capacity: expanded to %zu (was %zu)", new_capacity, old_capacity);
    return 0;
}

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

