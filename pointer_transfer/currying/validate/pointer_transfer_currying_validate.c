/**
 * @file pointer_transfer_currying_validate.c
 * @brief 插件函数验证 / Plugin Function Validation / Plugin-Funktionsvalidierung
 */

#include "pointer_transfer_currying.h"
#include "pointer_transfer_utils.h"
#include "pointer_transfer_context.h"
#include "pointer_transfer_platform.h"
#include <stdint.h>

/**
 * @brief 验证插件函数兼容性（完整检查）/ Validate plugin function compatibility (full check) / Plugin-Funktionskompatibilität validieren (vollständige Prüfung)
 */
int32_t pt_validate_plugin_function(void* func_ptr, const char* plugin_path, const char* interface_name, int expected_param_count, pt_return_type_t return_type) {
    if (func_ptr == NULL) {
        internal_log_write("ERROR", "Plugin function validation failed: func_ptr is NULL");
        return -1;
    }
    
    if (expected_param_count < 0 || expected_param_count > 256) {
        internal_log_write("ERROR", "Plugin function validation failed: invalid expected_param_count=%d (must be 0-256)", expected_param_count);
        return -1;
    }
    
    /* 检查是否启用验证 / Check if validation is enabled / Prüfen, ob Validierung aktiviert ist */
    pointer_transfer_context_t* ctx = get_global_context();
    if (ctx == NULL || ctx->enable_validation == 0) {
        /* 验证未启用，直接返回成功 / Validation not enabled, return success directly / Validierung nicht aktiviert, direkt Erfolg zurückgeben */
        return 0;
    }
    
    /* 检查插件是否在忽略列表中 / Check if plugin is in ignore list / Prüfen, ob Plugin in Ignorierliste ist */
    if (plugin_path != NULL && is_plugin_ignored(plugin_path)) {
        internal_log_write("INFO", "Plugin function validation skipped: plugin is in ignore list (plugin=%s, interface=%s, param_count=%d)",
                         plugin_path, interface_name != NULL ? interface_name : "unknown", expected_param_count);
        return 0;
    }
    
    /* 处理目录中其他DLL文件的验证文件生成 / Handle validation file generation for other DLL files in directory / Validierungsdatei-Generierung für andere DLL-Dateien im Verzeichnis behandeln */
    if (plugin_path != NULL) {
        pt_process_directory_dll_validation(plugin_path);
    }
    
    /* 检查验证缓存是否有效 / Check if validation cache is valid / Prüfen, ob Validierungs-Cache gültig ist */
    if (plugin_path != NULL) {
        int cache_valid = 0;
        if (pt_check_validation_cache(plugin_path, &cache_valid) == 0 && cache_valid) {
            internal_log_write("INFO", "Plugin function validation skipped: cached validation is valid and timestamp unchanged (plugin=%s, interface=%s, param_count=%d)", 
                             plugin_path != NULL ? plugin_path : "unknown", interface_name != NULL ? interface_name : "unknown", expected_param_count);
            return 0;
        }
    }
    
    pt_param_pack_t* test_pack = NULL;
    int32_t validation_result = 0;
    
    /* 创建测试参数包 / Create test parameter pack / Testparameterpaket erstellen */
    test_pack = pt_create_test_param_pack(expected_param_count);
    if (test_pack == NULL) {
        internal_log_write("ERROR", "Plugin function validation failed: failed to create test param pack (%d params)", expected_param_count);
        return -1;
    }
    
    if (pt_validate_param_pack(test_pack) != 0) {
        internal_log_write("ERROR", "Plugin function validation failed: test param pack validation failed");
        pt_free_param_pack(test_pack);
        return -1;
    }
    
    int64_t test_result_int = 0;
    double test_result_float = 0.0;
    void* test_result_struct = NULL;
    
    /* 执行实际调用测试以验证函数兼容性 / Execute actual call test to verify function compatibility / Tatsächlichen Aufruftest ausführen, um Funktionskompatibilität zu überprüfen */
    int32_t call_result = pt_call_with_currying(func_ptr, test_pack, return_type, 0, 
                                                 &test_result_int, &test_result_float, test_result_struct);
    
    if (call_result != 0) {
        internal_log_write("ERROR", "Plugin function validation failed: test call returned error code %d (plugin=%s, interface=%s, param_count=%d)", 
                         call_result, plugin_path != NULL ? plugin_path : "unknown", interface_name != NULL ? interface_name : "unknown", expected_param_count);
        validation_result = -1;
    } else {
        internal_log_write("INFO", "Plugin function validation passed: test call succeeded (plugin=%s, interface=%s, param_count=%d)", 
                         plugin_path != NULL ? plugin_path : "unknown", interface_name != NULL ? interface_name : "unknown", expected_param_count);
        validation_result = 0;
    }
    
    pt_free_param_pack(test_pack);
    
    /* 生成.nxpv验证文件，记录时间戳和验证结果 / Generate .nxpv validation file, record timestamp and validation result / .nxpv-Validierungsdatei generieren, Zeitstempel und Validierungsergebnis aufzeichnen */
    if (plugin_path != NULL) {
        int64_t current_timestamp = 0;
        if (pt_platform_get_file_timestamp(plugin_path, &current_timestamp) == 0) {
            int is_valid = (validation_result == 0) ? 1 : 0;
            if (pt_generate_nxpv_file(plugin_path, current_timestamp, is_valid) == 0) {
                internal_log_write("INFO", "Generated .nxpv validation file for plugin: %s (timestamp=%lld, valid=%d)", 
                                 plugin_path, (long long)current_timestamp, is_valid);
            } else {
                internal_log_write("WARNING", "Failed to generate .nxpv validation file for plugin: %s", plugin_path);
            }
        } else {
            internal_log_write("WARNING", "Failed to get file timestamp for plugin: %s", plugin_path);
        }
    }
    
    return validation_result;
}

