/**
 * @file pointer_transfer_interface_rule.c
 * @brief 返回值传递规则匹配 / Return Value Transfer Rule Matching / Rückgabewert-Übertragungsregel-Abgleich
 */

#include "pointer_transfer_interface.h"
#include "pointer_transfer_context.h"
#include "pointer_transfer_types.h"
#include "pointer_transfer_utils.h"
#include <string.h>

/**
 * @brief 收集匹配的返回值传递规则 / Collect matching return value transfer rules / Passende Rückgabewert-Übertragungsregeln sammeln
 * @param ctx 上下文 / Context / Kontext
 * @param source_plugin 源插件名称 / Source plugin name / Quell-Plugin-Name
 * @param source_interface 源接口名称 / Source interface name / Quell-Schnittstellenname
 * @param matched_rules 输出匹配的规则索引数组 / Output matched rule indices array / Ausgabe-Array mit passenden Regelindizes
 * @param max_matched 最大匹配数量 / Maximum match count / Maximale Übereinstimmungsanzahl
 * @return 匹配的规则数量 / Number of matched rules / Anzahl der passenden Regeln
 */
size_t collect_matching_return_value_rules(pointer_transfer_context_t* ctx, const char* source_plugin, 
                                            const char* source_interface, size_t* matched_rules, size_t max_matched) {
    if (ctx == NULL || ctx->rules == NULL || source_plugin == NULL || source_interface == NULL || 
        matched_rules == NULL || max_matched == 0) {
        return 0;
    }
    
    size_t matched_count = 0;
    
    for (size_t i = 0; i < ctx->rule_count && matched_count < max_matched; i++) {
        pointer_transfer_rule_t* active_rule = &ctx->rules[i];
        if (!active_rule->enabled || active_rule->source_plugin == NULL || active_rule->source_interface == NULL) {
            continue;
        }
        
        if (active_rule->source_param_index == -1 &&
            strcmp(active_rule->source_plugin, source_plugin) == 0 &&
            strcmp(active_rule->source_interface, source_interface) == 0) {
            matched_rules[matched_count++] = i;
        }
    }
    
    return matched_count;
}

/**
 * @brief 检查是否有完全相同的目标位置 / Check if exact duplicate target location exists / Prüfen, ob exakt doppelte Zielposition existiert
 * @param ctx 上下文 / Context / Kontext
 * @param rule 当前规则 / Current rule / Aktuelle Regel
 * @param source_plugin 源插件名称 / Source plugin name / Quell-Plugin-Name
 * @param source_interface 源接口名称 / Source interface name / Quell-Schnittstellenname
 * @param start_index 开始搜索的索引 / Start search index / Start-Suchindex
 * @return 找到返回1，否则返回0 / Returns 1 if found, 0 otherwise / Gibt 1 zurück, wenn gefunden, sonst 0
 */
int check_exact_duplicate_target(const pointer_transfer_context_t* ctx, const pointer_transfer_rule_t* rule,
                                  const char* source_plugin, const char* source_interface, size_t start_index) {
    if (ctx == NULL || ctx->rules == NULL || rule == NULL || source_plugin == NULL || source_interface == NULL) {
        return 0;
    }
    
    for (size_t j = start_index; j < ctx->rule_count; j++) {
        pointer_transfer_rule_t* next_rule = &ctx->rules[j];
        if (!next_rule->enabled || next_rule->source_plugin == NULL || next_rule->source_interface == NULL) {
            continue;
        }
        
        if (next_rule->source_param_index == -1 &&
            strcmp(next_rule->source_plugin, source_plugin) == 0 &&
            strcmp(next_rule->source_interface, source_interface) == 0 &&
            next_rule->target_plugin != NULL && next_rule->target_interface != NULL &&
            rule->target_plugin != NULL && rule->target_interface != NULL &&
            strcmp(next_rule->target_plugin, rule->target_plugin) == 0 &&
            strcmp(next_rule->target_interface, rule->target_interface) == 0 &&
            next_rule->target_param_index == rule->target_param_index) {
            return 1;
        }
    }
    
    return 0;
}

/**
 * @brief 处理参数值传递规则 / Process parameter value transfer rules / Parameterwert-Übertragungsregeln verarbeiten
 * @param ctx 上下文 / Context / Kontext
 * @param rule 当前规则 / Current rule / Aktuelle Regel
 * @param state 接口状态 / Interface state / Schnittstellenstatus
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int process_parameter_value_transfer_rules(pointer_transfer_context_t* ctx, const pointer_transfer_rule_t* rule,
                                            target_interface_state_t* state) {
    if (ctx == NULL || ctx->rules == NULL || rule == NULL || state == NULL) {
        return -1;
    }
    
    if (rule->target_plugin == NULL || rule->target_interface == NULL) {
        return -1;
    }
    
    for (size_t i = 0; i < ctx->rule_count; i++) {
        pointer_transfer_rule_t* param_rule = &ctx->rules[i];
        if (!param_rule->enabled || param_rule->source_plugin == NULL || param_rule->source_interface == NULL) {
            continue;
        }
        
        if (strcmp(param_rule->source_plugin, rule->target_plugin) == 0 &&
            strcmp(param_rule->source_interface, rule->target_interface) == 0 &&
            param_rule->source_param_index >= 0 &&
            param_rule->source_param_index < state->param_count &&
            state->param_ready != NULL && state->param_values != NULL &&
            state->param_ready[param_rule->source_param_index] &&
            state->param_values[param_rule->source_param_index] != NULL) {
            
            void* param_value = state->param_values[param_rule->source_param_index];
            internal_log_write("INFO", "Triggering rule %zu: %s.%s[%d] -> %s.%s[%d] (getting parameter value)", 
                          i, param_rule->source_plugin, param_rule->source_interface, param_rule->source_param_index,
                          param_rule->target_plugin != NULL ? param_rule->target_plugin : "unknown",
                          param_rule->target_interface != NULL ? param_rule->target_interface : "unknown",
                          param_rule->target_param_index);
            
            int call_result = call_target_plugin_interface(param_rule, param_value);
            if (call_result == 0) {
                internal_log_write("INFO", "Successfully triggered rule %zu for parameter %d", i, param_rule->source_param_index);
            } else {
                internal_log_write("WARNING", "Failed to trigger rule %zu for parameter %d (error=%d)", i, param_rule->source_param_index, call_result);
            }
        }
    }
    
    return 0;
}

