/**
 * @file pointer_transfer_interface_result.c
 * @brief 接口调用结果处理 / Interface Call Result Processing / Schnittstellenaufruf-Ergebnis-Verarbeitung
 */

#include "pointer_transfer_interface.h"
#include "pointer_transfer_context.h"
#include "pointer_transfer_types.h"
#include "pointer_transfer_utils.h"
#include <stdlib.h>
#include <string.h>

/**
 * @brief 处理返回值传递规则 / Process return value transfer rules / Rückgabewert-Übertragungsregeln verarbeiten
 * @param ctx 上下文 / Context / Kontext
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
int process_return_value_transfer_rules(pointer_transfer_context_t* ctx, const pointer_transfer_rule_t* rule,
                                         pt_return_type_t return_type, size_t return_size,
                                         int64_t result_int, double result_float, void* struct_buffer,
                                         const char* call_chain[], size_t call_chain_size, int recursion_depth) {
    if (ctx == NULL || ctx->rules == NULL || rule == NULL) {
        return -1;
    }
    
    size_t matched_rules[256];
    size_t matched_count = collect_matching_return_value_rules(ctx, rule->target_plugin, rule->target_interface,
                                                                matched_rules, sizeof(matched_rules) / sizeof(matched_rules[0]));
    
    if (matched_count == 0) {
        return 0;
    }
    
    int processed[256] = {0};
    
    for (size_t match_idx = 0; match_idx < matched_count; match_idx++) {
        if (processed[match_idx]) {
            continue;
        }
        
        size_t i = matched_rules[match_idx];
        pointer_transfer_rule_t* active_rule = &ctx->rules[i];
        
        if (active_rule->set_group != NULL && strlen(active_rule->set_group) > 0 &&
            active_rule->target_plugin != NULL && active_rule->target_interface != NULL) {
            process_setgroup_rule_group(ctx, active_rule, matched_rules, matched_count, processed, rule,
                                        return_type, return_size, result_int, result_float, struct_buffer,
                                        call_chain, call_chain_size, recursion_depth);
        } else {
            if (process_non_setgroup_rule(active_rule, rule, return_type, return_size,
                                          result_int, result_float, struct_buffer,
                                          call_chain, call_chain_size, recursion_depth, i) == 1) {
                break;
            }
        }
    }
    
    return 0;
}

/**
 * @brief 清理接口调用资源 / Cleanup interface call resources / Schnittstellenaufruf-Ressourcen bereinigen
 * @param return_type 返回值类型 / Return type / Rückgabetyp
 * @param struct_buffer 结构体缓冲区 / Struct buffer / Strukturpuffer
 * @param state 接口状态 / Interface state / Schnittstellenstatus
 * @param skip_param_cleanup 是否跳过参数清理 / Whether to skip parameter cleanup / Ob Parameterbereinigung übersprungen werden soll
 */
void cleanup_interface_call_resources(pt_return_type_t return_type, void* struct_buffer,
                                      target_interface_state_t* state, int skip_param_cleanup) {
    if (return_type == PT_RETURN_TYPE_STRUCT_VAL && struct_buffer != NULL) {
        free(struct_buffer);
    }
    
    if (state != NULL && !skip_param_cleanup) {
        cleanup_interface_state_parameters(state);
    }
}

