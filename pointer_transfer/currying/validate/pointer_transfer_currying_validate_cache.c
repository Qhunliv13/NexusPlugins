/**
 * @file pointer_transfer_currying_validate_cache.c
 * @brief 验证缓存管理 / Validation Cache Management / Validierungs-Cache-Verwaltung
 */

#include "pointer_transfer_currying.h"
#include "pointer_transfer_utils.h"
#include "pointer_transfer_context.h"
#include "pointer_transfer_platform.h"
#include <string.h>
#include <stdint.h>

/**
 * @brief 检查验证缓存是否有效 / Check if validation cache is valid / Prüfen, ob Validierungs-Cache gültig ist
 */
int32_t pt_check_validation_cache(const char* plugin_path, int* cache_valid) {
    if (plugin_path == NULL || cache_valid == NULL) {
        return -1;
    }
    
    *cache_valid = 0;
    
    char nxpv_path[1024];
    if (pt_build_nxpv_path(plugin_path, nxpv_path, sizeof(nxpv_path)) != 0) {
        return -1;
    }
    
    int64_t cached_timestamp = 0;
    int cached_valid = 0;
    
    /* 读取缓存的验证信息 / Read cached validation info / Gecachte Validierungsinformationen lesen */
    if (pt_read_validation_from_nxpv(nxpv_path, &cached_timestamp, &cached_valid) == 0) {
        /* 获取当前DLL文件的修改时间戳 / Get current DLL file modification timestamp / Aktuellen DLL-Dateiänderungszeitstempel abrufen */
        int64_t current_timestamp = 0;
        if (pt_platform_get_file_timestamp(plugin_path, &current_timestamp) == 0) {
            /* 如果时间戳匹配且验证结果为合规，则缓存有效 / If timestamp matches and validation result is valid, cache is valid / Wenn Zeitstempel übereinstimmt und Validierungsergebnis gültig ist, ist Cache gültig */
            if (current_timestamp == cached_timestamp && cached_valid == 1) {
                *cache_valid = 1;
                return 0;
            }
        }
    }
    
    return 0;
}

/**
 * @brief 处理目录中其他DLL文件的验证文件生成 / Handle validation file generation for other DLL files in directory / Validierungsdatei-Generierung für andere DLL-Dateien im Verzeichnis behandeln
 */
int32_t pt_process_directory_dll_validation(const char* plugin_path) {
    if (plugin_path == NULL) {
        return -1;
    }
    
    /* 检查插件是否在忽略列表中 / Check if plugin is in ignore list / Prüfen, ob Plugin in Ignorierliste ist */
    if (is_plugin_ignored(plugin_path)) {
        return 0;
    }
    
    /* 递归查找同目录下的所有DLL文件并为它们生成.nxpv文件（仅在启用验证时）/ Recursively find all DLL files in the same directory and generate .nxpv files for them (only when validation is enabled) / Alle DLL-Dateien im selben Verzeichnis rekursiv finden und .nxpv-Dateien für sie generieren (nur wenn Validierung aktiviert ist) */
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
            if (pt_build_nxpv_path(dll_files[i], nxpv_path, sizeof(nxpv_path)) == 0) {
                int64_t cached_timestamp = 0;
                int cached_valid = 0;
                
                int64_t dll_timestamp = 0;
                if (pt_platform_get_file_timestamp(dll_files[i], &dll_timestamp) == 0) {
                    /* 如果.nxpv文件不存在或时间戳不匹配，生成新的验证文件 / If .nxpv file doesn't exist or timestamp doesn't match, generate new validation file / Wenn .nxpv-Datei nicht existiert oder Zeitstempel nicht übereinstimmt, neue Validierungsdatei generieren */
                    if (pt_read_validation_from_nxpv(nxpv_path, &cached_timestamp, &cached_valid) != 0 || 
                        cached_timestamp != dll_timestamp) {
                        /* 为DLL文件生成.nxpv文件，标记为未验证（Valid=0），等待实际验证 / Generate .nxpv file for DLL file, mark as unvalidated (Valid=0), pending actual validation / .nxpv-Datei für DLL-Datei generieren, als nicht validiert markieren (Valid=0), wartet auf tatsächliche Validierung */
                        if (pt_generate_nxpv_file(dll_files[i], dll_timestamp, 0) == 0) {
                            internal_log_write("INFO", "Generated .nxpv validation file for DLL: %s (timestamp=%lld, valid=0, not validated)", 
                                             dll_files[i], (long long)dll_timestamp);
                        }
                    }
                }
            }
        }
    }
    
    return 0;
}

