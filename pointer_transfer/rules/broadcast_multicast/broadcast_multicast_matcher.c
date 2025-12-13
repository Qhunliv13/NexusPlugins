/**
 * @file broadcast_multicast_matcher.c
 * @brief BROADCAST和MULTICAST规则匹配器 / BROADCAST and MULTICAST Rule Matcher / BROADCAST- und MULTICAST-Regelabgleich
 */

#include "broadcast_multicast_matcher.h"
#include "../../pointer_transfer_context.h"
#include "../../pointer_transfer_utils.h"
#include "../../pointer_transfer_interface.h"
#include "../../pointer_transfer_types.h"
#include <string.h>

/**
 * @brief 应用BROADCAST和MULTICAST规则（使用索引） / Apply BROADCAST and MULTICAST rules (using index) / BROADCAST- und MULTICAST-Regeln anwenden (mit Index)
 */
size_t apply_broadcast_multicast_rules_indexed(const char* source_plugin_name, const char* source_interface_name, 
                                                int source_param_index, void* ptr, size_t start_index, size_t end_index,
                                                size_t* success_count) {
    pointer_transfer_context_t* ctx = get_global_context();
    size_t matched_count = 0;
    
    for (size_t i = start_index; i <= end_index && i < ctx->rule_count; i++) {
        pointer_transfer_rule_t* rule = &ctx->rules[i];
        if (!rule->enabled) {
            continue;
        }
        
        if (rule->source_plugin != NULL && rule->source_interface != NULL &&
            strcmp(rule->source_plugin, source_plugin_name) == 0 &&
            strcmp(rule->source_interface, source_interface_name) == 0 &&
            rule->source_param_index == source_param_index) {
            
            if (rule->transfer_mode == TRANSFER_MODE_BROADCAST || rule->transfer_mode == TRANSFER_MODE_MULTICAST) {
                if (rule->transfer_mode == TRANSFER_MODE_MULTICAST) {
                    if (rule->multicast_group == NULL || strlen(rule->multicast_group) == 0) {
                        continue;
                    }
                }
                
                if (!check_condition(rule->condition, ptr)) {
                    internal_log_write("INFO", "Transfer rule %zu condition not met, skipping - condition: %s", 
                                i, rule->condition != NULL ? rule->condition : "none");
                    continue;
                }
                
                matched_count++;
                internal_log_write("INFO", "Applying transfer rule %zu (mode=%d) - %s.%s[%d] to %s.%s[%d]", 
                            i, (int)rule->transfer_mode,
                            source_plugin_name, source_interface_name, source_param_index,
                            rule->target_plugin != NULL ? rule->target_plugin : "unknown",
                            rule->target_interface != NULL ? rule->target_interface : "unknown",
                            rule->target_param_index);
                
                int call_result = call_target_plugin_interface(rule, ptr);
                if (call_result == 0) {
                    (*success_count)++;
                    internal_log_write("INFO", "Successfully called target plugin interface");
                } else {
                    internal_log_write("WARNING", "Failed to call target plugin interface (error=%d)", call_result);
                }
            }
        }
    }
    
    return matched_count;
}

/**
 * @brief 应用BROADCAST和MULTICAST规则（线性查找） / Apply BROADCAST and MULTICAST rules (linear search) / BROADCAST- und MULTICAST-Regeln anwenden (lineare Suche)
 */
size_t apply_broadcast_multicast_rules_linear(const char* source_plugin_name, const char* source_interface_name, 
                                               int source_param_index, void* ptr, size_t* success_count) {
    pointer_transfer_context_t* ctx = get_global_context();
    size_t matched_count = 0;
    
    for (size_t i = 0; i < ctx->rule_count; i++) {
        pointer_transfer_rule_t* rule = &ctx->rules[i];
        if (!rule->enabled) {
            continue;
        }
        
        if (rule->source_plugin != NULL && rule->source_interface != NULL) {
            if (strcmp(rule->source_plugin, source_plugin_name) == 0 &&
                strcmp(rule->source_interface, source_interface_name) == 0 &&
                rule->source_param_index == source_param_index) {
                
                if (rule->transfer_mode == TRANSFER_MODE_BROADCAST || rule->transfer_mode == TRANSFER_MODE_MULTICAST) {
                    if (rule->transfer_mode == TRANSFER_MODE_MULTICAST) {
                        if (rule->multicast_group == NULL || strlen(rule->multicast_group) == 0) {
                            continue;
                        }
                    }
                    
                    if (!check_condition(rule->condition, ptr)) {
                        internal_log_write("INFO", "Transfer rule %zu condition not met, skipping - condition: %s", 
                                    i, rule->condition != NULL ? rule->condition : "none");
                        continue;
                    }
                    
                    matched_count++;
                    internal_log_write("INFO", "Applying transfer rule %zu (mode=%d) - %s.%s[%d] to %s.%s[%d]", 
                                i, (int)rule->transfer_mode,
                                source_plugin_name, source_interface_name, source_param_index,
                                rule->target_plugin != NULL ? rule->target_plugin : "unknown",
                                rule->target_interface != NULL ? rule->target_interface : "unknown",
                                rule->target_param_index);
                    
                    int call_result = call_target_plugin_interface(rule, ptr);
                    if (call_result == 0) {
                        (*success_count)++;
                        internal_log_write("INFO", "Successfully called target plugin interface");
                    } else {
                        internal_log_write("WARNING", "Failed to call target plugin interface (error=%d)", call_result);
                    }
                }
            }
        }
    }
    
    return matched_count;
}

