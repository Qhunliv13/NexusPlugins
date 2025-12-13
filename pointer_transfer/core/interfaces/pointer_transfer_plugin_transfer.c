/**
 * @file pointer_transfer_plugin_transfer.c
 * @brief 指针传递接口实现 / Pointer Transfer Interface Implementation / Zeigerübertragungs-Schnittstellenimplementierung
 */

#include "pointer_transfer_plugin.h"
#include "pointer_transfer_types.h"
#include "pointer_transfer_context.h"
#include "pointer_transfer_utils.h"
#include "rules/core/pointer_transfer_rule_matcher.h"
#include <stdlib.h>
#include <string.h>

/**
 * @brief 传递指针 / Transfer pointer / Zeiger übertragen
 * @param source_plugin_name 源插件名称 / Source plugin name / Quell-Plugin-Name
 * @param source_interface_name 源接口名称 / Source interface name / Quell-Schnittstellenname
 * @param source_param_index 源参数索引 / Source parameter index / Quell-Parameterindex
 * @param ptr 指针 / Pointer / Zeiger
 * @param expected_type 数据类型 / Data type / Datentyp
 * @param type_name 类型名称 / Type name / Typname
 * @param data_size 数据大小 / Data size / Datengröße
 * @return 成功返回0，类型不匹配返回1，其他错误返回-1 / Returns 0 on success, 1 on type mismatch, -1 on other errors / Gibt 0 bei Erfolg zurück, 1 bei Typfehlanpassung, -1 bei anderen Fehlern
 */
POINTER_TRANSFER_PLUGIN_EXPORT int POINTER_TRANSFER_PLUGIN_CALL TransferPointer(const char* source_plugin_name, const char* source_interface_name, int source_param_index, void* ptr, nxld_param_type_t expected_type, const char* type_name, size_t data_size) {
    if (source_plugin_name == NULL) {
        internal_log_write("WARNING", "TransferPointer: received NULL source_plugin_name");
        return -1;
    }
    
    if (source_interface_name == NULL) {
        internal_log_write("WARNING", "TransferPointer: received NULL source_interface_name");
        return -1;
    }
    
    if (ptr == NULL) {
        internal_log_write("WARNING", "TransferPointer: received NULL pointer");
        return -1;
    }
    
    pointer_transfer_context_t* ctx = get_global_context();
    int type_mismatch = 0;
    
    if (ctx->stored_ptr != NULL && ctx->stored_ptr == ptr) {
        nxld_param_type_t stored_type = ctx->stored_type;
        
        if (!check_type_compatibility(stored_type, expected_type)) {
            const char* stored_type_str = get_type_name_string(stored_type);
            const char* expected_type_str = get_type_name_string(expected_type);
            const char* stored_type_name = ctx->stored_type_name != NULL ? ctx->stored_type_name : "unknown";
            const char* expected_type_name = type_name != NULL ? type_name : "unknown";
            
            internal_log_write("WARNING", "TransferPointer: type mismatch detected for pointer %p - stored: %s (%s), expected: %s (%s)", 
                           ptr, stored_type_str, stored_type_name, expected_type_str, expected_type_name);
            type_mismatch = 1;
        }
        
        if (data_size > 0 && ctx->stored_size > 0 && data_size != ctx->stored_size) {
            internal_log_write("WARNING", "TransferPointer: size mismatch detected for pointer %p - stored: %zu, expected: %zu", 
                           ptr, ctx->stored_size, data_size);
            type_mismatch = 1;
        }
    }
    
    ctx->stored_ptr = ptr;
    ctx->stored_type = expected_type;
    ctx->stored_size = data_size;
    
    if (ctx->stored_type_name != NULL) {
        free(ctx->stored_type_name);
        ctx->stored_type_name = NULL;
    }
    
    if (type_name != NULL && strlen(type_name) > 0) {
        size_t type_name_len = strlen(type_name);
        ctx->stored_type_name = (char*)malloc(type_name_len + 1);
        if (ctx->stored_type_name != NULL) {
            memcpy(ctx->stored_type_name, type_name, type_name_len);
            ctx->stored_type_name[type_name_len] = '\0';
        }
    }
    
    if (type_mismatch) {
        internal_log_write("INFO", "TransferPointer: pointer transferred with type mismatch warning - type: %s (%s), size: %zu", 
                      get_type_name_string(expected_type), type_name != NULL ? type_name : "unknown", data_size);
        return 1;
    }
    
    internal_log_write("INFO", "TransferPointer: pointer transferred successfully - source_plugin=%s, source_interface=%s, source_param_index=%d, type: %s (%s), size: %zu", 
                  source_plugin_name, source_interface_name, source_param_index, get_type_name_string(expected_type), type_name != NULL ? type_name : "unknown", data_size);
    
    if (ctx->rule_count > 0 && ctx->rules != NULL) {
        size_t matched_count = 0;
        size_t success_count = 0;
        
        /* 使用索引查找匹配规则 / Use index to find matching rules / Index verwenden, um übereinstimmende Regeln zu finden */
        size_t start_index = 0;
        size_t end_index = 0;
        int use_index = find_rule_index_range(source_plugin_name, source_interface_name, source_param_index, &start_index, &end_index);
        
        if (use_index) {
            matched_count = apply_matched_rules_indexed(source_plugin_name, source_interface_name, 
                                                         source_param_index, ptr, start_index, end_index, &success_count);
        } else {
            matched_count = apply_matched_rules_linear(source_plugin_name, source_interface_name, 
                                                        source_param_index, ptr, &success_count);
        }
        
        if (matched_count > 0) {
            internal_log_write("INFO", "Processed %zu rules, %zu successful", matched_count, success_count);
        }
    }
    
    return 0;
}

