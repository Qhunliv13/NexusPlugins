/**
 * @file pointer_transfer_interface_rule_process.c
 * @brief 返回值传递规则处理 / Return Value Transfer Rule Processing / Rückgabewert-Übertragungsregel-Verarbeitung
 */

#include "pointer_transfer_interface.h"
#include "pointer_transfer_context.h"
#include "pointer_transfer_types.h"
#include "pointer_transfer_utils.h"
#include <stdlib.h>
#include <string.h>

/**
 * @brief 处理SetGroup规则组 / Process SetGroup rule group / SetGroup-Regelgruppe verarbeiten
 * @param ctx 上下文 / Context / Kontext
 * @param active_rule 活动规则 / Active rule / Aktive Regel
 * @param matched_rules 匹配的规则索引数组 / Matched rule indices array / Array mit passenden Regelindizes
 * @param matched_count 匹配的规则数量 / Matched rule count / Anzahl der passenden Regeln
 * @param processed 已处理标记数组 / Processed flags array / Array mit Verarbeitungsmarkierungen
 * @param rule 当前规则 / Current rule / Aktuelle Regel
 * @param return_type 返回值类型 / Return type / Rückgabetyp
 * @param return_size 返回值大小 / Return size / Rückgabegröße
 * @param result_int 整数结果 / Integer result / Ganzzahlergebnis
 * @param result_float 浮点数结果 / Float result / Gleitkommaergebnis
 * @param struct_buffer 结构体缓冲区 / Struct buffer / Strukturpuffer
 * @param call_chain 调用链 / Call chain / Aufrufkette
 * @param call_chain_size 调用链大小 / Call chain size / Aufrufketten-Größe
 * @param recursion_depth 递归深度 / Recursion depth / Rekursionstiefe
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int process_setgroup_rule_group(pointer_transfer_context_t* ctx, const pointer_transfer_rule_t* active_rule,
                                 const size_t* matched_rules, size_t matched_count, int* processed,
                                 const pointer_transfer_rule_t* rule, pt_return_type_t return_type, size_t return_size,
                                 int64_t result_int, double result_float, void* struct_buffer,
                                 const char* call_chain[], size_t call_chain_size, int recursion_depth) {
    if (ctx == NULL || ctx->rules == NULL || active_rule == NULL || matched_rules == NULL || 
        processed == NULL || rule == NULL) {
        return -1;
    }
    
    size_t group_rules[64];
    size_t group_count = collect_setgroup_rules(ctx, matched_rules, matched_count, processed, active_rule,
                                                 group_rules, sizeof(group_rules) / sizeof(group_rules[0]));
    
    sort_setgroup_rules_by_param_index(ctx, group_rules, group_count);
    
    for (size_t group_idx = 0; group_idx < group_count; group_idx++) {
        size_t rule_idx = group_rules[group_idx];
        pointer_transfer_rule_t* group_rule = &ctx->rules[rule_idx];
        
        int64_t group_result_int = result_int;
        double group_result_float = result_float;
        pt_return_type_t group_return_type = return_type;
        size_t group_return_size = return_size;
        void* group_struct_buffer = NULL;
        
        if (recall_source_interface_for_setgroup(group_rule, &group_result_int, &group_result_float,
                                                  &group_return_type, &group_return_size, &group_struct_buffer) == 0) {
            internal_log_write("INFO", "Re-called source interface %s.%s for SetGroup rule %zu, new result = %lld", 
                          group_rule->source_plugin, group_rule->source_interface, rule_idx, (long long)group_result_int);
        } else {
            internal_log_write("WARNING", "Failed to re-call source interface %s.%s for SetGroup rule %zu, using original return value", 
                          group_rule->source_plugin, group_rule->source_interface, rule_idx);
        }
        
        int should_check_group = 0;
        int is_min_param_index = 1;
        check_setgroup_set_group_status(ctx, group_rule, rule_idx, &should_check_group, &is_min_param_index);
        
        if (!check_setgroup_parameter_readiness(group_rule)) {
            if (should_check_group && !is_min_param_index) {
                internal_log_write("INFO", "Set group check: skipping rule %zu (target state not exists, subsequent rule will set parameter, current rule is not minimum param index)", rule_idx);
            }
            continue;
        }
        
        execute_setgroup_rule(group_rule, rule, group_result_int, group_result_float,
                              group_return_type, group_return_size, group_struct_buffer,
                              call_chain, call_chain_size, recursion_depth, rule_idx);
        
        if (group_struct_buffer != NULL) {
            free(group_struct_buffer);
            group_struct_buffer = NULL;
        }
        
        if (group_rule->transfer_mode == TRANSFER_MODE_UNICAST) {
            if (check_more_rules_in_setgroup(ctx, group_rules, group_count, group_idx, group_rule)) {
                continue;
            }
            
            if (check_exact_duplicate_target(ctx, group_rule, rule->target_plugin, rule->target_interface, rule_idx + 1)) {
                break;
            }
        }
    }
    
    if (group_count > 0 && active_rule->target_plugin != NULL && active_rule->target_interface != NULL) {
        cleanup_setgroup_target_interface_parameters(ctx, active_rule->target_plugin, active_rule->target_interface);
    }
    
    return 0;
}

/**
 * @brief 处理非SetGroup规则 / Process non-SetGroup rule / Nicht-SetGroup-Regel verarbeiten
 * @param active_rule 活动规则 / Active rule / Aktive Regel
 * @param rule 当前规则 / Current rule / Aktuelle Regel
 * @param return_type 返回值类型 / Return type / Rückgabetyp
 * @param return_size 返回值大小 / Return size / Rückgabegröße
 * @param result_int 整数结果 / Integer result / Ganzzahlergebnis
 * @param result_float 浮点数结果 / Float result / Gleitkommaergebnis
 * @param struct_buffer 结构体缓冲区 / Struct buffer / Strukturpuffer
 * @param call_chain 调用链 / Call chain / Aufrufkette
 * @param call_chain_size 调用链大小 / Call chain size / Aufrufketten-Größe
 * @param recursion_depth 递归深度 / Recursion depth / Rekursionstiefe
 * @param rule_idx 规则索引 / Rule index / Regelindex
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int process_non_setgroup_rule(const pointer_transfer_rule_t* active_rule, const pointer_transfer_rule_t* rule,
                               pt_return_type_t return_type, size_t return_size,
                               int64_t result_int, double result_float, void* struct_buffer,
                               const char* call_chain[], size_t call_chain_size, int recursion_depth, size_t rule_idx) {
    if (active_rule == NULL || rule == NULL) {
        return -1;
    }
    
    execute_non_setgroup_rule(active_rule, rule, return_type, return_size,
                              result_int, result_float, struct_buffer,
                              call_chain, call_chain_size, recursion_depth, rule_idx);
    
    pointer_transfer_context_t* ctx = get_global_context();
    if (active_rule->transfer_mode == TRANSFER_MODE_UNICAST) {
        if (check_exact_duplicate_target(ctx, active_rule, rule->target_plugin, rule->target_interface, rule_idx + 1)) {
            return 1;
        }
    }
    
    return 0;
}

