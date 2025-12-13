/**
 * @file pointer_transfer_interface.c
 * @brief 指针传递插件接口调用（已拆分） / Pointer Transfer Plugin Interface Invocation (Split) / Zeigerübertragungs-Plugin-Schnittstellenaufruf (Aufgeteilt)
 * 
 * 此文件现在仅包含主调用逻辑，其他功能已拆分到以下文件：
 * - interface/pointer_transfer_interface_state.c: 接口状态管理
 * - interface/pointer_transfer_interface_param.c: 参数验证和处理
 * - interface/pointer_transfer_interface_cycle.c: 循环检测
 * - interface/pointer_transfer_interface_load.c: 插件加载和函数获取
 * - interface/pointer_transfer_interface_return.c: 返回值处理和函数调用
 * - interface/pointer_transfer_interface_rule.c: 返回值传递规则匹配
 * - interface/pointer_transfer_interface_chain.c: 调用链管理
 * - interface/pointer_transfer_interface_cleanup.c: 接口状态清理
 * - interface/pointer_transfer_interface_setgroup.c: SetGroup规则处理
 * - interface/pointer_transfer_interface_setgroup_exec.c: SetGroup规则执行
 * - interface/pointer_transfer_interface_validate.c: 接口调用验证
 * - interface/pointer_transfer_interface_rule_process.c: 返回值传递规则处理
 * - interface/pointer_transfer_interface_prepare.c: 接口调用准备
 * - interface/pointer_transfer_interface_result.c: 接口调用结果处理
 * 
 * This file now only contains main call logic, other functionality has been split into:
 * - interface/pointer_transfer_interface_state.c: Interface state management
 * - interface/pointer_transfer_interface_param.c: Parameter validation and processing
 * - interface/pointer_transfer_interface_cycle.c: Cycle detection
 * - interface/pointer_transfer_interface_load.c: Plugin loading and function retrieval
 * - interface/pointer_transfer_interface_return.c: Return value processing and function call
 * - interface/pointer_transfer_interface_rule.c: Return value transfer rule matching
 * - interface/pointer_transfer_interface_chain.c: Call chain management
 * - interface/pointer_transfer_interface_cleanup.c: Interface state cleanup
 * - interface/pointer_transfer_interface_setgroup.c: SetGroup rule processing
 * - interface/pointer_transfer_interface_setgroup_exec.c: SetGroup rule execution
 * - interface/pointer_transfer_interface_validate.c: Interface call validation
 * - interface/pointer_transfer_interface_rule_process.c: Return value transfer rule processing
 * - interface/pointer_transfer_interface_prepare.c: Interface call preparation
 * - interface/pointer_transfer_interface_result.c: Interface call result processing
 */

#include "pointer_transfer_interface.h"
#include "pointer_transfer_context.h"
#include "pointer_transfer_config.h"
#include "pointer_transfer_plugin_loader.h"
#include "pointer_transfer_utils.h"
#include "pointer_transfer_types.h"
#include "pointer_transfer_platform.h"
#include "pointer_transfer_currying.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <stdint.h>

/* call_function_generic已迁移至pointer_transfer_platform.c，替代函数为pt_platform_safe_call / call_function_generic migrated to pointer_transfer_platform.c, replaced by pt_platform_safe_call / call_function_generic nach pointer_transfer_platform.c migriert, ersetzt durch pt_platform_safe_call */
/* 接口状态管理函数已移至 interface/pointer_transfer_interface_state.c / Interface state management functions moved to interface/pointer_transfer_interface_state.c / Schnittstellenstatus-Verwaltungsfunktionen nach interface/pointer_transfer_interface_state.c verschoben */
 
 /**
  * @brief 调用目标插件接口（内部实现） / Call target plugin interface (internal implementation) / Ziel-Plugin-Schnittstelle aufrufen (interne Implementierung)
  */
 int call_target_plugin_interface_internal(const pointer_transfer_rule_t* rule, void* ptr, int recursion_depth, const char* call_chain[], size_t call_chain_size, int skip_param_cleanup) {
     if (rule == NULL || rule->target_plugin == NULL || rule->target_interface == NULL) {
         internal_log_write("ERROR", "Invalid parameters for call_target_plugin_interface");
         return -1;
     }
     
     check_recursion_depth(recursion_depth);
     
     if (detect_call_cycle(rule, call_chain, call_chain_size) != 0) {
         return -1;
     }
     
     pointer_transfer_context_t* ctx = get_global_context();
     
     target_interface_state_t* state = NULL;
     int actual_param_count = 0;
     pt_return_type_t return_type;
     size_t return_size;
     void* struct_buffer = NULL;
     
     if (prepare_interface_call(rule, ptr, &state, &actual_param_count, &return_type, &return_size, &struct_buffer) != 0) {
         return -1;
     }
     
     int64_t result_int = 0;
     double result_float = 0.0;
     if (execute_interface_call(state, rule, actual_param_count, return_type, return_size, struct_buffer,
                                &result_int, &result_float) != 0) {
         cleanup_interface_call_resources(return_type, struct_buffer, state, skip_param_cleanup);
         return -1;
     }
     
     if (ctx->rules != NULL) {
         process_return_value_transfer_rules(ctx, rule, return_type, return_size, result_int, result_float, struct_buffer,
                                             call_chain, call_chain_size, recursion_depth);
     }
     
     if (ctx->rules != NULL && rule != NULL && rule->target_plugin != NULL && rule->target_interface != NULL && state != NULL) {
         process_parameter_value_transfer_rules(ctx, rule, state);
     }
     
     cleanup_interface_call_resources(return_type, struct_buffer, state, skip_param_cleanup);
     
     return 0;
 }
 
/**
 * @brief 调用目标插件接口 / Call target plugin interface / Ziel-Plugin-Schnittstelle aufrufen
 */
int call_target_plugin_interface(const pointer_transfer_rule_t* rule, void* ptr) {
    const char* initial_call_chain[1] = {NULL};
    size_t initial_call_chain_size = 0;
    
    return call_target_plugin_interface_internal(rule, ptr, 0, initial_call_chain, initial_call_chain_size, 0);
}