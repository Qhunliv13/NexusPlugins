/**
 * @file config_rules_merger.c
 * @brief 规则合并器 / Rules Merger / Regeln-Zusammenführer
 */

#include "config_rules_merger.h"
#include "../common/config_errors.h"
#include "pointer_transfer_context.h"
#include "pointer_transfer_types.h"
#include "pointer_transfer_utils.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/**
 * @brief 合并规则到上下文 / Merge rules to context / Regeln in Kontext zusammenführen
 */
int merge_rules_to_context(pointer_transfer_rule_t* temp_rules, size_t temp_rules_count, 
                           int max_seen_index) {
    pointer_transfer_context_t* ctx = get_global_context();
    if (ctx == NULL) {
        return CONFIG_ERR_INVALID_PARAM;
    }
    
    /* 验证段索引一致性 / Validate section index consistency / Abschnittsindex-Konsistenz validieren */
    if (max_seen_index >= 0 && (size_t)(max_seen_index + 1) != temp_rules_count) {
        internal_log_write("WARNING", "load_transfer_rules: section index inconsistency detected - max index=%d, but only %zu rules parsed. Expected %zu rules for indices 0-%d", 
            max_seen_index, temp_rules_count, (size_t)(max_seen_index + 1), max_seen_index);
    }
    
    size_t start_rule_index = ctx->rule_count;
    /* 检查size_t加法溢出 / Check size_t addition overflow / size_t-Additionsüberlauf prüfen */
    if (start_rule_index > SIZE_MAX - temp_rules_count) {
        internal_log_write("ERROR", "Rule count overflow detected: start_rule_index=%zu, temp_rules_count=%zu, sum would exceed SIZE_MAX", 
                     start_rule_index, temp_rules_count);
        return CONFIG_ERR_OVERFLOW;
    }
    size_t new_rule_count = start_rule_index + temp_rules_count;
    
    if (new_rule_count > ctx->rule_capacity) {
        size_t needed = new_rule_count;
        while (ctx->rule_capacity < needed) {
            if (expand_rules_capacity() != 0) {
                return CONFIG_ERR_MEMORY;
            }
        }
    }
    
    for (size_t i = 0; i < temp_rules_count; i++) {
        size_t new_index = start_rule_index + i;
        if (new_index < ctx->rule_capacity) {
            pointer_transfer_rule_t* src_rule = &temp_rules[i];
            pointer_transfer_rule_t* dst_rule = &ctx->rules[new_index];
            memset(dst_rule, 0, sizeof(pointer_transfer_rule_t));
            
            dst_rule->source_param_index = src_rule->source_param_index;
            dst_rule->target_param_index = src_rule->target_param_index;
            dst_rule->transfer_mode = src_rule->transfer_mode;
            dst_rule->enabled = src_rule->enabled;
            
            if (src_rule->source_plugin != NULL) {
                dst_rule->source_plugin = allocate_string(src_rule->source_plugin);
            }
            if (src_rule->source_interface != NULL) {
                dst_rule->source_interface = allocate_string(src_rule->source_interface);
            }
            if (src_rule->target_plugin != NULL) {
                dst_rule->target_plugin = allocate_string(src_rule->target_plugin);
            }
            if (src_rule->target_plugin_path != NULL) {
                dst_rule->target_plugin_path = allocate_string(src_rule->target_plugin_path);
            }
            if (src_rule->target_interface != NULL) {
                dst_rule->target_interface = allocate_string(src_rule->target_interface);
            }
            if (src_rule->target_param_value != NULL) {
                dst_rule->target_param_value = allocate_string(src_rule->target_param_value);
            }
            if (src_rule->description != NULL) {
                dst_rule->description = allocate_string(src_rule->description);
            }
            if (src_rule->multicast_group != NULL) {
                dst_rule->multicast_group = allocate_string(src_rule->multicast_group);
            }
            if (src_rule->condition != NULL) {
                dst_rule->condition = allocate_string(src_rule->condition);
            }
            if (src_rule->set_group != NULL) {
                dst_rule->set_group = allocate_string(src_rule->set_group);
            }
            dst_rule->cache_self = src_rule->cache_self;
        }
    }
    
    ctx->rule_count = new_rule_count;
    
    /* 构建规则索引 / Build rule index / Regelindex erstellen */
    if (build_rule_index() != 0) {
        internal_log_write("WARNING", "Failed to build rule index, falling back to linear search");
    }
    
    /* 构建规则缓存 / Build rule cache / Regel-Cache erstellen */
    build_rule_cache();
    
    return CONFIG_ERR_SUCCESS;
}

