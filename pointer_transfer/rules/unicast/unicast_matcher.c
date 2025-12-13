/**
 * @file unicast_matcher.c
 * @brief UNICAST规则匹配器 / UNICAST Rule Matcher / UNICAST-Regelabgleich
 */

#include "unicast_matcher.h"
#include "../../pointer_transfer_context.h"
#include "../../pointer_transfer_utils.h"
#include "../../pointer_transfer_interface.h"
#include "../../pointer_transfer_types.h"
#include <string.h>

/**
 * @brief 检查是否存在匹配完全相同目标位置的其他规则 / Check if other rules match the exact same target location / Prüfen, ob andere Regeln die exakt gleiche Zielposition abgleichen
 * @param ctx 上下文指针 / Context pointer / Kontext-Zeiger
 * @param source_plugin_name 源插件名称 / Source plugin name / Quell-Plugin-Name
 * @param source_interface_name 源接口名称 / Source interface name / Quell-Schnittstellenname
 * @param source_param_index 源参数索引 / Source parameter index / Quell-Parameterindex
 * @param current_rule 当前规则指针 / Current rule pointer / Aktuelle Regel-Zeiger
 * @param start_index 起始索引 / Start index / Startindex
 * @param end_index 结束索引 / End index / Endindex
 * @return 存在重复返回1，否则返回0 / Returns 1 if duplicate exists, 0 otherwise / Gibt 1 zurück, wenn Duplikat vorhanden ist, sonst 0
 */
static int check_exact_duplicate_target(pointer_transfer_context_t* ctx, const char* source_plugin_name, 
                                         const char* source_interface_name, int source_param_index,
                                         pointer_transfer_rule_t* current_rule, size_t start_index, size_t end_index) {
    for (size_t j = start_index + 1; j <= end_index && j < ctx->rule_count; j++) {
        pointer_transfer_rule_t* next_rule = &ctx->rules[j];
        if (!next_rule->enabled || next_rule->source_plugin == NULL || next_rule->source_interface == NULL) {
            continue;
        }
        if (strcmp(next_rule->source_plugin, source_plugin_name) == 0 &&
            strcmp(next_rule->source_interface, source_interface_name) == 0 &&
            next_rule->source_param_index == source_param_index &&
            next_rule->target_plugin != NULL && next_rule->target_interface != NULL &&
            current_rule->target_plugin != NULL && current_rule->target_interface != NULL &&
            strcmp(next_rule->target_plugin, current_rule->target_plugin) == 0 &&
            strcmp(next_rule->target_interface, current_rule->target_interface) == 0 &&
            next_rule->target_param_index == current_rule->target_param_index) {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief 应用UNICAST规则（使用索引） / Apply UNICAST rules (using index) / UNICAST-Regeln anwenden (mit Index)
 */
size_t apply_unicast_rules_indexed(const char* source_plugin_name, const char* source_interface_name, 
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
                    
                    if (check_exact_duplicate_target(ctx, source_plugin_name, source_interface_name, source_param_index, rule, i, end_index)) {
                        break;
                    }
                }
            }
        }
    }
    
    return matched_count;
}

/**
 * @brief 应用UNICAST规则（线性查找） / Apply UNICAST rules (linear search) / UNICAST-Regeln anwenden (lineare Suche)
 */
size_t apply_unicast_rules_linear(const char* source_plugin_name, const char* source_interface_name, 
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
                    
                    if (check_exact_duplicate_target(ctx, source_plugin_name, source_interface_name, source_param_index, rule, i, ctx->rule_count - 1)) {
                        break;
                    }
                }
            }
        }
    }
    
    return matched_count;
}

