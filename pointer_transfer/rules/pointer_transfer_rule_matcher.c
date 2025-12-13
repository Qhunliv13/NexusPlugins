/**
 * @file pointer_transfer_rule_matcher.c
 * @brief 规则匹配和应用逻辑 / Rule Matching and Application Logic / Regelabgleich und Anwendungslogik
 */

#include "pointer_transfer_rule_matcher.h"
#include "pointer_transfer_context.h"
#include "pointer_transfer_utils.h"
#include "pointer_transfer_interface.h"
#include "pointer_transfer_types.h"
#include <string.h>

/**
 * @brief 应用BROADCAST和MULTICAST规则 / Apply BROADCAST and MULTICAST rules / BROADCAST- und MULTICAST-Regeln anwenden
 */
static size_t apply_broadcast_multicast_rules(const char* source_plugin_name, const char* source_interface_name, 
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
 * @brief 应用UNICAST规则 / Apply UNICAST rules / UNICAST-Regeln anwenden
 */
static size_t apply_unicast_rules(const char* source_plugin_name, const char* source_interface_name, 
                                   int source_param_index, void* ptr, size_t start_index, size_t end_index,
                                   size_t* success_count) {
    pointer_transfer_context_t* ctx = get_global_context();
    size_t matched_count = 0;
    
    for (size_t i = start_index; i <= end_index && i < ctx->rule_count; i++) {
        pointer_transfer_rule_t* rule = &ctx->rules[i];
        if (!rule->enabled) {
            continue;
        }
        
        if (rule->source_plugin != NULL && rule->source_interface != NULL) {
            if (strcmp(rule->source_plugin, source_plugin_name) == 0 &&
                strcmp(rule->source_interface, source_interface_name) == 0 &&
                rule->source_param_index == source_param_index) {
                
                if (rule->transfer_mode == TRANSFER_MODE_UNICAST) {
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
                    
                    /* 检查是否存在匹配完全相同目标位置的其他规则 / Check if other rules match the exact same target location / Prüfen, ob andere Regeln die exakt gleiche Zielposition abgleichen */
                    int has_exact_duplicate = 0;
                    for (size_t j = i + 1; j <= end_index && j < ctx->rule_count; j++) {
                        pointer_transfer_rule_t* next_rule = &ctx->rules[j];
                        if (!next_rule->enabled || next_rule->source_plugin == NULL || next_rule->source_interface == NULL) {
                            continue;
                        }
                        if (strcmp(next_rule->source_plugin, source_plugin_name) == 0 &&
                            strcmp(next_rule->source_interface, source_interface_name) == 0 &&
                            next_rule->source_param_index == source_param_index &&
                            next_rule->target_plugin != NULL && next_rule->target_interface != NULL &&
                            rule->target_plugin != NULL && rule->target_interface != NULL &&
                            strcmp(next_rule->target_plugin, rule->target_plugin) == 0 &&
                            strcmp(next_rule->target_interface, rule->target_interface) == 0 &&
                            next_rule->target_param_index == rule->target_param_index) {
                            has_exact_duplicate = 1;
                            break;
                        }
                    }
                    /* 仅在找到完全相同的目标位置时停止 / Only stop when exact duplicate target is found / Nur stoppen, wenn exakt doppeltes Ziel gefunden wird */
                    if (has_exact_duplicate) {
                        break;
                    }
                }
            }
        }
    }
    
    return matched_count;
}

/**
 * @brief 应用匹配的规则（使用索引） / Apply matched rules (using index) / Passende Regeln anwenden (mit Index)
 */
size_t apply_matched_rules_indexed(const char* source_plugin_name, const char* source_interface_name, 
                                    int source_param_index, void* ptr, size_t start_index, size_t end_index,
                                    size_t* success_count) {
    size_t matched_count = 0;
    
    /* BROADCAST和MULTICAST规则 / BROADCAST and MULTICAST rules / BROADCAST- und MULTICAST-Regeln */
    matched_count += apply_broadcast_multicast_rules(source_plugin_name, source_interface_name, 
                                                      source_param_index, ptr, start_index, end_index, success_count);
    
    /* UNICAST规则 / UNICAST rules / UNICAST-Regeln */
    matched_count += apply_unicast_rules(source_plugin_name, source_interface_name, 
                                          source_param_index, ptr, start_index, end_index, success_count);
    
    return matched_count;
}

/**
 * @brief 应用匹配的规则（线性查找） / Apply matched rules (linear search) / Passende Regeln anwenden (lineare Suche)
 */
size_t apply_matched_rules_linear(const char* source_plugin_name, const char* source_interface_name, 
                                   int source_param_index, void* ptr, size_t* success_count) {
    pointer_transfer_context_t* ctx = get_global_context();
    size_t matched_count = 0;
    
    /* BROADCAST和MULTICAST规则 / BROADCAST and MULTICAST rules / BROADCAST- und MULTICAST-Regeln */
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
    
    /* UNICAST规则 / UNICAST rules / UNICAST-Regeln */
    for (size_t i = 0; i < ctx->rule_count; i++) {
        pointer_transfer_rule_t* rule = &ctx->rules[i];
        if (!rule->enabled) {
            continue;
        }
        
        if (rule->source_plugin != NULL && rule->source_interface != NULL) {
            if (strcmp(rule->source_plugin, source_plugin_name) == 0 &&
                strcmp(rule->source_interface, source_interface_name) == 0 &&
                rule->source_param_index == source_param_index) {
                
                if (rule->transfer_mode == TRANSFER_MODE_UNICAST) {
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
                    
                    /* 检查是否存在匹配完全相同目标位置的其他规则 / Check if other rules match the exact same target location / Prüfen, ob andere Regeln die exakt gleiche Zielposition abgleichen */
                    int has_exact_duplicate = 0;
                    for (size_t j = i + 1; j < ctx->rule_count; j++) {
                        pointer_transfer_rule_t* next_rule = &ctx->rules[j];
                        if (!next_rule->enabled || next_rule->source_plugin == NULL || next_rule->source_interface == NULL) {
                            continue;
                        }
                        if (strcmp(next_rule->source_plugin, source_plugin_name) == 0 &&
                            strcmp(next_rule->source_interface, source_interface_name) == 0 &&
                            next_rule->source_param_index == source_param_index &&
                            next_rule->target_plugin != NULL && next_rule->target_interface != NULL &&
                            rule->target_plugin != NULL && rule->target_interface != NULL &&
                            strcmp(next_rule->target_plugin, rule->target_plugin) == 0 &&
                            strcmp(next_rule->target_interface, rule->target_interface) == 0 &&
                            next_rule->target_param_index == rule->target_param_index) {
                            has_exact_duplicate = 1;
                            break;
                        }
                    }
                    /* 仅在找到完全相同的目标位置时停止 / Only stop when exact duplicate target is found / Nur stoppen, wenn exakt doppeltes Ziel gefunden wird */
                    if (has_exact_duplicate) {
                        break;
                    }
                }
            }
        }
    }
    
    return matched_count;
}

