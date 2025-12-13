/**
 * @file pointer_transfer_context_capacity_rules.c
 * @brief 规则数组容量扩展操作 / Rules Array Capacity Expansion Operations / Regel-Array-Kapazitätserweiterungsoperationen
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

