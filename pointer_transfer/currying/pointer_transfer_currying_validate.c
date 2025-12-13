/**
 * @file pointer_transfer_currying_validate.c
 * @brief 插件函数验证和验证文件操作 / Plugin Function Validation and Validation File Operations / Plugin-Funktionsvalidierung und Validierungsdatei-Operationen
 */

#include "pointer_transfer_currying.h"
#include "pointer_transfer_utils.h"
#include "pointer_transfer_context.h"
#include "pointer_transfer_platform.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

/**
 * @brief 构建.nxpv文件路径 / Build .nxpv file path / .nxpv-Dateipfad erstellen
 */
static int build_nxpv_path(const char* plugin_path, char* nxpv_path, size_t buffer_size) {
    if (plugin_path == NULL || nxpv_path == NULL || buffer_size == 0) {
        return -1;
    }
    
    const char* ext_pos = strrchr(plugin_path, '.');
    
    size_t base_len;
    if (ext_pos != NULL) {
        base_len = ext_pos - plugin_path;
    } else {
        base_len = strlen(plugin_path);
    }
    
    if (base_len + 6 >= buffer_size) {
        return -1;
    }
    
    memcpy(nxpv_path, plugin_path, base_len);
    memcpy(nxpv_path + base_len, ".nxpv", 6);
    
    return 0;
}

/**
 * @brief 从.nxpv文件读取验证信息 / Read validation info from .nxpv file / Validierungsinformationen aus .nxpv-Datei lesen
 */
static int32_t read_validation_from_nxpv(const char* nxpv_path, int64_t* timestamp, int* is_valid) {
    if (nxpv_path == NULL || timestamp == NULL || is_valid == NULL) {
        return -1;
    }
    
    FILE* fp = fopen(nxpv_path, "r");
    if (fp == NULL) {
        return -1;
    }
    
    char line[256];
    *timestamp = 0;
    *is_valid = 0;
    
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strncmp(line, "Timestamp=", 10) == 0) {
            *timestamp = strtoll(line + 10, NULL, 10);
        } else if (strncmp(line, "Valid=", 6) == 0) {
            *is_valid = (strcmp(line + 6, "1\n") == 0 || strcmp(line + 6, "1\r\n") == 0) ? 1 : 0;
        }
    }
    
    fclose(fp);
    return 0;
}

/**
 * @brief 生成.nxpv验证文件 / Generate .nxpv validation file / .nxpv-Validierungsdatei generieren
 */
static int32_t generate_nxpv_file(const char* plugin_path, int64_t timestamp, int is_valid) {
    if (plugin_path == NULL) {
        return -1;
    }
    
    char nxpv_path[1024];
    if (build_nxpv_path(plugin_path, nxpv_path, sizeof(nxpv_path)) != 0) {
        return -1;
    }
    
    FILE* fp = fopen(nxpv_path, "w");
    if (fp == NULL) {
        return -1;
    }
    
    fprintf(fp, "Timestamp=%lld\n", (long long)timestamp);
    fprintf(fp, "Valid=%d\n", is_valid);
    
    fclose(fp);
    return 0;
}

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
    
    /* 递归查找同目录下的所有DLL文件并为它们生成.nxpv文件（仅在启用验证时）/ Recursively find all DLL files in the same directory and generate .nxpv files for them (only when validation is enabled) / Alle DLL-Dateien im selben Verzeichnis rekursiv finden und .nxpv-Dateien für sie generieren (nur wenn Validierung aktiviert ist) */
    if (plugin_path != NULL) {
        char dll_files[256][1024];
        int file_count = 0;
        if (pt_platform_find_all_dll_files(plugin_path, dll_files, 256, &file_count) == 0 && file_count > 0) {
            internal_log_write("INFO", "Found %d DLL files in directory, generating .nxpv files", file_count);
            for (int i = 0; i < file_count; i++) {
                /* 跳过当前插件文件（后续处理）/ Skip current plugin file (processed subsequently) / Aktuelle Plugin-Datei überspringen (wird anschließend verarbeitet) */
                if (strcmp(dll_files[i], plugin_path) == 0) {
                    continue;
                }
                
                /* 检查DLL文件是否在忽略列表中 / Check if DLL file is in ignore list / Prüfen, ob DLL-Datei in Ignorierliste ist */
                if (is_plugin_ignored(dll_files[i])) {
                    internal_log_write("INFO", "Skipping ignored plugin DLL: %s", dll_files[i]);
                    continue;
                }
                
                /* 检查.nxpv文件是否已存在且时间戳匹配 / Check if .nxpv file already exists and timestamp matches / Prüfen, ob .nxpv-Datei bereits existiert und Zeitstempel übereinstimmt */
                char nxpv_path[1024];
                if (build_nxpv_path(dll_files[i], nxpv_path, sizeof(nxpv_path)) == 0) {
                    int64_t cached_timestamp = 0;
                    int cached_valid = 0;
                    
                    int64_t dll_timestamp = 0;
                    if (pt_platform_get_file_timestamp(dll_files[i], &dll_timestamp) == 0) {
                        /* 如果.nxpv文件不存在或时间戳不匹配，生成新的验证文件 / If .nxpv file doesn't exist or timestamp doesn't match, generate new validation file / Wenn .nxpv-Datei nicht existiert oder Zeitstempel nicht übereinstimmt, neue Validierungsdatei generieren */
                        if (read_validation_from_nxpv(nxpv_path, &cached_timestamp, &cached_valid) != 0 || 
                            cached_timestamp != dll_timestamp) {
                            /* 为DLL文件生成.nxpv文件，标记为未验证（Valid=0），等待实际验证 / Generate .nxpv file for DLL file, mark as unvalidated (Valid=0), pending actual validation / .nxpv-Datei für DLL-Datei generieren, als nicht validiert markieren (Valid=0), wartet auf tatsächliche Validierung */
                            if (generate_nxpv_file(dll_files[i], dll_timestamp, 0) == 0) {
                                internal_log_write("INFO", "Generated .nxpv validation file for DLL: %s (timestamp=%lld, valid=0, not validated)", 
                                                 dll_files[i], (long long)dll_timestamp);
                            }
                        }
                    }
                }
            }
        }
    }
    
    /* 检查.nxpv文件是否存在，如果存在且时间戳匹配且验证结果为合规，则跳过验证 / Check if .nxpv file exists, if exists and timestamp matches and validation result is valid, skip validation / Prüfen, ob .nxpv-Datei existiert, wenn vorhanden und Zeitstempel übereinstimmt und Validierungsergebnis gültig ist, Validierung überspringen */
    if (plugin_path != NULL) {
        char nxpv_path[1024];
        if (build_nxpv_path(plugin_path, nxpv_path, sizeof(nxpv_path)) == 0) {
            int64_t cached_timestamp = 0;
            int cached_valid = 0;
            
            /* 读取缓存的验证信息 / Read cached validation info / Gecachte Validierungsinformationen lesen */
            if (read_validation_from_nxpv(nxpv_path, &cached_timestamp, &cached_valid) == 0) {
                /* 获取当前DLL文件的修改时间戳 / Get current DLL file modification timestamp / Aktuellen DLL-Dateiänderungszeitstempel abrufen */
                int64_t current_timestamp = 0;
                if (pt_platform_get_file_timestamp(plugin_path, &current_timestamp) == 0) {
                    /* 如果时间戳匹配且验证结果为合规，则跳过验证 / If timestamp matches and validation result is valid, skip validation / Wenn Zeitstempel übereinstimmt und Validierungsergebnis gültig ist, Validierung überspringen */
                    if (current_timestamp == cached_timestamp && cached_valid == 1) {
                        internal_log_write("INFO", "Plugin function validation skipped: cached validation is valid and timestamp unchanged (plugin=%s, interface=%s, param_count=%d)", 
                                         plugin_path != NULL ? plugin_path : "unknown", interface_name != NULL ? interface_name : "unknown", expected_param_count);
                        return 0;
                    }
                }
            }
        }
    }
    
    pt_param_pack_t* test_pack = NULL;
    int32_t validation_result = 0;
    
    /* 完整检查：为所有参数数量创建测试参数包并执行实际调用测试 / Full check: create test param pack for all parameter counts and execute actual call test / Vollständige Prüfung: Testparameterpaket für alle Parameteranzahlen erstellen und tatsächlichen Aufruftest ausführen */
    if (expected_param_count == 0) {
        test_pack = pt_create_param_pack(0, NULL, NULL, NULL);
        if (test_pack == NULL) {
            internal_log_write("ERROR", "Plugin function validation failed: failed to create test param pack (0 params)");
            return -1;
        }
    } else {
        /* 动态分配测试参数数组，支持任意数量的参数 / Dynamically allocate test parameter arrays to support arbitrary parameter counts / Testparameter-Arrays dynamisch zuweisen, um beliebige Parameteranzahlen zu unterstützen */
        nxld_param_type_t* types = (nxld_param_type_t*)malloc(expected_param_count * sizeof(nxld_param_type_t));
        int32_t* values = (int32_t*)malloc(expected_param_count * sizeof(int32_t));
        void** value_ptrs = (void**)malloc(expected_param_count * sizeof(void*));
        size_t* sizes = (size_t*)malloc(expected_param_count * sizeof(size_t));
        
        if (types == NULL || values == NULL || value_ptrs == NULL || sizes == NULL) {
            internal_log_write("ERROR", "Plugin function validation failed: failed to allocate memory for test parameters (%d params)", expected_param_count);
            if (types != NULL) free(types);
            if (values != NULL) free(values);
            if (value_ptrs != NULL) free(value_ptrs);
            if (sizes != NULL) free(sizes);
            return -1;
        }
        
        /* 初始化测试参数：使用INT32类型和递增的测试值 / Initialize test parameters: use INT32 type and incrementing test values / Testparameter initialisieren: INT32-Typ und inkrementelle Testwerte verwenden */
        for (int i = 0; i < expected_param_count; i++) {
            types[i] = NXLD_PARAM_TYPE_INT32;
            values[i] = i + 1;
            value_ptrs[i] = &values[i];
            sizes[i] = sizeof(int32_t);
        }
        
        test_pack = pt_create_param_pack(expected_param_count, types, value_ptrs, sizes);
        
        /* 释放临时分配的内存 / Free temporarily allocated memory / Temporär zugewiesenen Speicher freigeben */
        free(types);
        free(values);
        free(value_ptrs);
        free(sizes);
        
        if (test_pack == NULL) {
            internal_log_write("ERROR", "Plugin function validation failed: failed to create test param pack (%d params)", expected_param_count);
            return -1;
        }
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
            if (generate_nxpv_file(plugin_path, current_timestamp, is_valid) == 0) {
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

