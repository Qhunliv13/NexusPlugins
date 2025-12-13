/**
 * @file pointer_transfer_interface_setgroup_exec.c
 * @brief SetGroup规则执行 / SetGroup Rule Execution / SetGroup-Regel-Ausführung
 */

#include "pointer_transfer_interface.h"
#include "pointer_transfer_context.h"
#include "pointer_transfer_types.h"
#include "pointer_transfer_utils.h"
#include <string.h>
#include <stdio.h>

/**
 * @brief 执行SetGroup规则 / Execute SetGroup rule / SetGroup-Regel ausführen
 * @param group_rule SetGroup规则 / SetGroup rule / SetGroup-Regel
 * @param rule 当前规则 / Current rule / Aktuelle Regel
 * @param group_result_int 组整数结果 / Group integer result / Gruppen-Ganzzahlergebnis
 * @param group_result_float 组浮点数结果 / Group float result / Gruppen-Gleitkommaergebnis
 * @param group_return_type 组返回值类型 / Group return type / Gruppen-Rückgabetyp
 * @param group_return_size 组返回值大小 / Group return size / Gruppen-Rückgabegröße
 * @param group_struct_buffer 组结构体缓冲区 / Group struct buffer / Gruppen-Strukturpuffer
 * @param call_chain 调用链 / Call chain / Aufrufkette
 * @param call_chain_size 调用链大小 / Call chain size / Aufrufketten-Größe
 * @param recursion_depth 递归深度 / Recursion depth / Rekursionstiefe
 * @param rule_idx 规则索引 / Rule index / Regelindex
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int execute_setgroup_rule(const pointer_transfer_rule_t* group_rule, const pointer_transfer_rule_t* rule,
                          int64_t group_result_int, double group_result_float,
                          pt_return_type_t group_return_type, size_t group_return_size, void* group_struct_buffer,
                          const char* call_chain[], size_t call_chain_size, int recursion_depth, size_t rule_idx) {
    if (group_rule == NULL || rule == NULL) {
        return -1;
    }
    
    pointer_transfer_context_t* ctx = get_global_context();
    
    internal_log_write("INFO", "Found active call rule %zu: %s.%s -> %s.%s", 
                  rule_idx, group_rule->source_plugin, group_rule->source_interface,
                  group_rule->target_plugin != NULL ? group_rule->target_plugin : "unknown",
                  group_rule->target_interface != NULL ? group_rule->target_interface : "unknown");
    
    void* call_param = NULL;
    static int64_t static_result_int = 0;
    static double static_result_float = 0.0;
    
    if (group_rule->target_param_value != NULL && strlen(group_rule->target_param_value) > 0) {
        call_param = (void*)group_rule->target_param_value;
    } else {
        if (group_return_type == PT_RETURN_TYPE_FLOAT || group_return_type == PT_RETURN_TYPE_DOUBLE) {
            static_result_float = group_result_float;
            call_param = &static_result_float;
            ctx->stored_size = sizeof(double);
            ctx->stored_type = NXLD_PARAM_TYPE_DOUBLE;
        } else {
            static_result_int = group_result_int;
            call_param = &static_result_int;
            ctx->stored_size = sizeof(int64_t);
            ctx->stored_type = NXLD_PARAM_TYPE_INT64;
        }
        internal_log_write("INFO", "Using %s return value %lld for SetGroup transfer", 
                          (group_return_type == PT_RETURN_TYPE_FLOAT || group_return_type == PT_RETURN_TYPE_DOUBLE) ? "float" : "integer/pointer",
                          (long long)((group_return_type == PT_RETURN_TYPE_FLOAT || group_return_type == PT_RETURN_TYPE_DOUBLE) ? (int64_t)group_result_float : group_result_int));
    }
    
    const char* new_call_chain[64];
    size_t new_call_chain_size = 0;
    build_new_call_chain(call_chain, call_chain_size, rule->target_plugin, rule->target_interface,
                         new_call_chain, &new_call_chain_size);
    
    int active_call_result = call_target_plugin_interface_internal(group_rule, call_param, recursion_depth + 1, new_call_chain, new_call_chain_size, 1);
    if (active_call_result == 0) {
        internal_log_write("INFO", "Successfully executed active call rule %zu", rule_idx);
    } else {
        internal_log_write("WARNING", "Failed to execute active call rule %zu (error=%d)", rule_idx, active_call_result);
    }
    
    return active_call_result;
}

/**
 * @brief 执行非SetGroup规则 / Execute non-SetGroup rule / Nicht-SetGroup-Regel ausführen
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
int execute_non_setgroup_rule(const pointer_transfer_rule_t* active_rule, const pointer_transfer_rule_t* rule,
                               pt_return_type_t return_type, size_t return_size,
                               int64_t result_int, double result_float, void* struct_buffer,
                               const char* call_chain[], size_t call_chain_size, int recursion_depth, size_t rule_idx) {
    if (active_rule == NULL || rule == NULL) {
        return -1;
    }
    
    pointer_transfer_context_t* ctx = get_global_context();
    
    static int64_t static_result_int = 0;
    static double static_result_float = 0.0;
    
    void* call_param = NULL;
    if (active_rule->target_param_value != NULL && strlen(active_rule->target_param_value) > 0) {
        call_param = (void*)active_rule->target_param_value;
    } else {
        if (return_type == PT_RETURN_TYPE_FLOAT || return_type == PT_RETURN_TYPE_DOUBLE) {
            static_result_float = result_float;
            call_param = &static_result_float;
            ctx->stored_size = sizeof(double);
            ctx->stored_type = NXLD_PARAM_TYPE_DOUBLE;
            internal_log_write("INFO", "Using float return value %lf for transfer", result_float);
        } else if (return_type == PT_RETURN_TYPE_STRUCT_VAL && struct_buffer != NULL) {
            call_param = struct_buffer;
            ctx->stored_size = return_size;
            ctx->stored_type = NXLD_PARAM_TYPE_POINTER;
            internal_log_write("INFO", "Using struct return value (size=%zu) for transfer", return_size);
        } else if (return_type == PT_RETURN_TYPE_STRUCT_PTR) {
            call_param = (void*)(intptr_t)result_int;
            ctx->stored_size = sizeof(void*);
            ctx->stored_type = NXLD_PARAM_TYPE_STRING;
            internal_log_write("INFO", "Using pointer return value %p for transfer", (void*)(intptr_t)result_int);
        } else {
            static_result_int = result_int;
            call_param = &static_result_int;
            ctx->stored_size = sizeof(int64_t);
            ctx->stored_type = NXLD_PARAM_TYPE_INT64;
            internal_log_write("INFO", "Using integer/pointer return value %lld for transfer", (long long)result_int);
        }
    }
    
    const char* new_call_chain[64];
    size_t new_call_chain_size = 0;
    build_new_call_chain(call_chain, call_chain_size, rule->target_plugin, rule->target_interface,
                         new_call_chain, &new_call_chain_size);
    
    internal_log_write("INFO", "Found active call rule %zu (no SetGroup): %s.%s -> %s.%s", 
                  rule_idx, active_rule->source_plugin, active_rule->source_interface,
                  active_rule->target_plugin != NULL ? active_rule->target_plugin : "unknown",
                  active_rule->target_interface != NULL ? active_rule->target_interface : "unknown");
    
    int active_call_result = call_target_plugin_interface_internal(active_rule, call_param, recursion_depth + 1, new_call_chain, new_call_chain_size, 0);
    if (active_call_result == 0) {
        internal_log_write("INFO", "Successfully executed active call rule %zu (no SetGroup)", rule_idx);
    } else {
        internal_log_write("WARNING", "Failed to execute active call rule %zu (no SetGroup, error=%d)", rule_idx, active_call_result);
    }
    
    return active_call_result;
}

