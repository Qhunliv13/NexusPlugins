/**
 * @file pointer_transfer_interface_setgroup.c
 * @brief SetGroup规则处理 / SetGroup Rule Processing / SetGroup-Regel-Verarbeitung
 */

#include "pointer_transfer_interface.h"
#include "pointer_transfer_context.h"
#include "pointer_transfer_types.h"
#include "pointer_transfer_platform.h"
#include "pointer_transfer_utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

/**
 * @brief 收集同一SetGroup中的所有规则 / Collect all rules in same SetGroup / Alle Regeln in derselben SetGroup sammeln
 * @param ctx 上下文 / Context / Kontext
 * @param matched_rules 匹配的规则索引数组 / Matched rule indices array / Array mit passenden Regelindizes
 * @param matched_count 匹配的规则数量 / Matched rule count / Anzahl der passenden Regeln
 * @param processed 已处理标记数组 / Processed flags array / Array mit Verarbeitungsmarkierungen
 * @param active_rule 当前活动规则 / Current active rule / Aktuelle aktive Regel
 * @param group_rules_out 输出组规则索引数组 / Output group rule indices array / Ausgabe-Array mit Gruppenregelindizes
 * @param max_group_rules 最大组规则数量 / Maximum group rules count / Maximale Gruppenregelanzahl
 * @return 组规则数量 / Group rule count / Gruppenregelanzahl
 */
size_t collect_setgroup_rules(pointer_transfer_context_t* ctx, const size_t* matched_rules, size_t matched_count,
                               int* processed, const pointer_transfer_rule_t* active_rule,
                               size_t* group_rules_out, size_t max_group_rules) {
    if (ctx == NULL || matched_rules == NULL || processed == NULL || active_rule == NULL || 
        group_rules_out == NULL || max_group_rules == 0) {
        return 0;
    }
    
    size_t group_count = 0;
    
    for (size_t j = 0; j < matched_count && group_count < max_group_rules; j++) {
        if (processed[j]) {
            continue;
        }
        size_t rule_idx = matched_rules[j];
        pointer_transfer_rule_t* group_rule = &ctx->rules[rule_idx];
        if (group_rule->set_group != NULL && 
            strcmp(group_rule->set_group, active_rule->set_group) == 0 &&
            group_rule->target_plugin != NULL && group_rule->target_interface != NULL &&
            strcmp(group_rule->target_plugin, active_rule->target_plugin) == 0 &&
            strcmp(group_rule->target_interface, active_rule->target_interface) == 0) {
            group_rules_out[group_count++] = rule_idx;
            processed[j] = 1;
        }
    }
    
    return group_count;
}

/**
 * @brief 按target_param_index排序规则 / Sort rules by target_param_index / Regeln nach target_param_index sortieren
 * @param ctx 上下文 / Context / Kontext
 * @param group_rules 组规则索引数组 / Group rule indices array / Array mit Gruppenregelindizes
 * @param group_count 组规则数量 / Group rule count / Gruppenregelanzahl
 */
void sort_setgroup_rules_by_param_index(pointer_transfer_context_t* ctx, size_t* group_rules, size_t group_count) {
    if (ctx == NULL || ctx->rules == NULL || group_rules == NULL) {
        return;
    }
    
    for (size_t a = 0; a < group_count; a++) {
        for (size_t b = a + 1; b < group_count; b++) {
            pointer_transfer_rule_t* rule_a = &ctx->rules[group_rules[a]];
            pointer_transfer_rule_t* rule_b = &ctx->rules[group_rules[b]];
            if (rule_a->target_param_index > rule_b->target_param_index) {
                size_t temp = group_rules[a];
                group_rules[a] = group_rules[b];
                group_rules[b] = temp;
            }
        }
    }
}

/**
 * @brief 重新调用源接口获取新的返回值 / Re-call source interface to get new return value / Quellschnittstelle neu aufrufen, um neuen Rückgabewert zu erhalten
 * @param group_rule SetGroup规则 / SetGroup rule / SetGroup-Regel
 * @param result_int_out 输出整数结果 / Output integer result / Ausgabe-Ganzzahlergebnis
 * @param result_float_out 输出浮点数结果 / Output float result / Ausgabe-Gleitkommaergebnis
 * @param return_type_out 输出返回值类型 / Output return type / Ausgabe-Rückgabetyp
 * @param return_size_out 输出返回值大小 / Output return size / Ausgabe-Rückgabegröße
 * @param struct_buffer_out 输出结构体缓冲区 / Output struct buffer / Ausgabe-Strukturpuffer
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int recall_source_interface_for_setgroup(const pointer_transfer_rule_t* group_rule,
                                          int64_t* result_int_out, double* result_float_out,
                                          pt_return_type_t* return_type_out, size_t* return_size_out,
                                          void** struct_buffer_out) {
    if (group_rule == NULL || result_int_out == NULL || result_float_out == NULL ||
        return_type_out == NULL || return_size_out == NULL || struct_buffer_out == NULL) {
        return -1;
    }
    
    if (group_rule->source_plugin == NULL || group_rule->source_interface == NULL) {
        return -1;
    }
    
    target_interface_state_t* source_state = find_interface_state(group_rule->source_plugin, group_rule->source_interface);
    if (source_state == NULL || source_state->func_ptr == NULL) {
        return -1;
    }
    
    int source_actual_param_count = source_state->param_count;
    if (source_state->is_variadic) {
        source_actual_param_count = 0;
        if (source_state->param_ready != NULL) {
            for (int i = 0; i < source_state->param_count; i++) {
                if (source_state->param_ready[i]) {
                    source_actual_param_count = i + 1;
                } else {
                    break;
                }
            }
        }
    }
    
    pt_return_type_t source_return_type = source_state->return_type;
    size_t source_return_size = source_state->return_size;
    
    if (source_return_type == PT_RETURN_TYPE_STRUCT_PTR && source_return_size > 0) {
#ifdef _WIN32
        if (source_return_size > 8) {
            source_return_type = PT_RETURN_TYPE_STRUCT_VAL;
        }
#else
        if (source_return_size > 16) {
            source_return_type = PT_RETURN_TYPE_STRUCT_VAL;
        }
#endif
    }
    
    void* group_struct_buffer = NULL;
    if (source_return_type == PT_RETURN_TYPE_STRUCT_VAL && source_return_size > 0) {
        group_struct_buffer = malloc(source_return_size);
        if (group_struct_buffer != NULL) {
            memset(group_struct_buffer, 0, source_return_size);
        }
    }
    
    if (source_state->param_types != NULL && source_state->param_values != NULL) {
        int64_t temp_result_int = 0;
        double temp_result_float = 0.0;
        int32_t source_call_result = pt_platform_safe_call(source_state->func_ptr, source_actual_param_count,
                                                          (void*)source_state->param_types, source_state->param_values,
                                                          (void*)source_state->param_sizes,
                                                          source_return_type, source_return_size, &temp_result_int, &temp_result_float, group_struct_buffer);
        if (source_call_result == 0) {
            *result_int_out = temp_result_int;
            *result_float_out = temp_result_float;
            *return_type_out = source_return_type;
            *return_size_out = source_return_size;
            *struct_buffer_out = group_struct_buffer;
            return 0;
        } else {
            if (group_struct_buffer != NULL) {
                free(group_struct_buffer);
            }
            return -1;
        }
    }
    
    if (group_struct_buffer != NULL) {
        free(group_struct_buffer);
    }
    return -1;
}

/**
 * @brief 检查SetGroup参数就绪状态 / Check SetGroup parameter readiness / SetGroup-Parameterbereitschaft prüfen
 * @param group_rule SetGroup规则 / SetGroup rule / SetGroup-Regel
 * @return 可以应用返回1，否则返回0 / Returns 1 if can apply, 0 otherwise / Gibt 1 zurück, wenn anwendbar, sonst 0
 */
int check_setgroup_parameter_readiness(const pointer_transfer_rule_t* group_rule) {
    if (group_rule == NULL || group_rule->target_plugin == NULL || 
        group_rule->target_interface == NULL || group_rule->target_param_index < 0) {
        return 0;
    }
    
    target_interface_state_t* target_state = find_interface_state(group_rule->target_plugin, group_rule->target_interface);
    if (target_state == NULL || target_state->param_ready == NULL) {
        if (group_rule->target_param_index == 0) {
            return 1;
        }
        return 0;
    }
    
    int check_limit = group_rule->target_param_index;
    if (target_state->is_variadic) {
        for (int j = 0; j < check_limit && j < target_state->param_count; j++) {
            if (!target_state->param_ready[j]) {
                return 0;
            }
        }
        if (check_limit < target_state->min_param_count) {
            int ready_count_after_set = check_limit + 1;
            if (ready_count_after_set < target_state->min_param_count) {
                return 0;
            }
        }
    } else {
        for (int j = 0; j < check_limit && j < target_state->param_count; j++) {
            if (!target_state->param_ready[j]) {
                return 0;
            }
        }
        if (group_rule->target_param_index >= target_state->param_count) {
            return 0;
        }
    }
    
    return 1;
}

/**
 * @brief 检查SetGroup中是否有更多规则要处理 / Check if there are more rules in SetGroup to process / Prüfen, ob noch weitere Regeln in SetGroup zu verarbeiten sind
 * @param ctx 上下文 / Context / Kontext
 * @param group_rules 组规则索引数组 / Group rule indices array / Array mit Gruppenregelindizes
 * @param group_count 组规则数量 / Group rule count / Gruppenregelanzahl
 * @param group_idx 当前组索引 / Current group index / Aktueller Gruppenindex
 * @param group_rule 当前组规则 / Current group rule / Aktuelle Gruppenregel
 * @return 有更多规则返回1，否则返回0 / Returns 1 if more rules, 0 otherwise / Gibt 1 zurück, wenn weitere Regeln, sonst 0
 */
int check_more_rules_in_setgroup(pointer_transfer_context_t* ctx, const size_t* group_rules, size_t group_count,
                                  size_t group_idx, const pointer_transfer_rule_t* group_rule) {
    if (ctx == NULL || ctx->rules == NULL || group_rules == NULL || group_rule == NULL) {
        return 0;
    }
    
    for (size_t j = group_idx + 1; j < group_count; j++) {
        size_t next_rule_idx = group_rules[j];
        pointer_transfer_rule_t* next_group_rule = &ctx->rules[next_rule_idx];
        if (next_group_rule->enabled && 
            next_group_rule->target_plugin != NULL && next_group_rule->target_interface != NULL &&
            group_rule->target_plugin != NULL && group_rule->target_interface != NULL &&
            strcmp(next_group_rule->target_plugin, group_rule->target_plugin) == 0 &&
            strcmp(next_group_rule->target_interface, group_rule->target_interface) == 0 &&
            next_group_rule->target_param_index == group_rule->target_param_index) {
            return 1;
        }
    }
    
    return 0;
}

/**
 * @brief 检查SetGroup设置组状态 / Check SetGroup set group status / SetGroup-Set-Gruppenstatus prüfen
 * @param ctx 上下文 / Context / Kontext
 * @param group_rule SetGroup规则 / SetGroup rule / SetGroup-Regel
 * @param rule_idx 规则索引 / Rule index / Regelindex
 * @param should_check_group_out 输出是否需要检查组 / Output whether need to check group / Ausgabe, ob Gruppe geprüft werden muss
 * @param is_min_param_index_out 输出是否是最小参数索引 / Output whether is minimum parameter index / Ausgabe, ob minimaler Parameterindex
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int check_setgroup_set_group_status(pointer_transfer_context_t* ctx, const pointer_transfer_rule_t* group_rule,
                                     size_t rule_idx, int* should_check_group_out, int* is_min_param_index_out) {
    if (ctx == NULL || ctx->rules == NULL || group_rule == NULL || 
        should_check_group_out == NULL || is_min_param_index_out == NULL) {
        return -1;
    }
    
    int should_check_group = 0;
    int is_min_param_index = 1;
    
    if (group_rule->set_group != NULL && 
        strlen(group_rule->set_group) > 0 &&
        group_rule->target_plugin != NULL && group_rule->target_interface != NULL &&
        group_rule->target_param_index >= 0) {
        internal_log_write("INFO", "Set group check: rule %zu has set_group=%s, target=%s.%s[%d]", 
            rule_idx, group_rule->set_group, group_rule->target_plugin, group_rule->target_interface, group_rule->target_param_index);
        
        for (size_t j = 0; j < ctx->rule_count; j++) {
            if (j == rule_idx) continue;
            pointer_transfer_rule_t* check_rule = &ctx->rules[j];
            if (check_rule->enabled && check_rule->set_group != NULL &&
                strcmp(check_rule->set_group, group_rule->set_group) == 0 &&
                check_rule->source_param_index == -1 &&
                check_rule->source_plugin != NULL && check_rule->source_interface != NULL &&
                group_rule->source_plugin != NULL && group_rule->source_interface != NULL &&
                strcmp(check_rule->source_plugin, group_rule->source_plugin) == 0 &&
                strcmp(check_rule->source_interface, group_rule->source_interface) == 0 &&
                check_rule->target_plugin != NULL && check_rule->target_interface != NULL &&
                strcmp(check_rule->target_plugin, group_rule->target_plugin) == 0 &&
                strcmp(check_rule->target_interface, group_rule->target_interface) == 0) {
                if (check_rule->target_param_index > group_rule->target_param_index) {
                    should_check_group = 1;
                    internal_log_write("INFO", "Set group check: rule %zu belongs to set group %s, found subsequent rule %zu that will set parameter %d", 
                        rule_idx, group_rule->set_group, j, check_rule->target_param_index);
                }
                if (check_rule->target_param_index < group_rule->target_param_index) {
                    is_min_param_index = 0;
                }
            }
        }
    } else if (group_rule->set_group == NULL) {
        internal_log_write("INFO", "Set group check: rule %zu has no set_group", rule_idx);
    }
    
    *should_check_group_out = should_check_group;
    *is_min_param_index_out = is_min_param_index;
    return 0;
}

