/**
 * @file pointer_transfer_currying_nxpv.c
 * @brief .nxpv验证文件操作 / .nxpv Validation File Operations / .nxpv-Validierungsdatei-Operationen
 */

#include "pointer_transfer_currying.h"
#include "pointer_transfer_utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

/**
 * @brief 构建.nxpv文件路径 / Build .nxpv file path / .nxpv-Dateipfad erstellen
 */
int pt_build_nxpv_path(const char* plugin_path, char* nxpv_path, size_t buffer_size) {
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
int32_t pt_read_validation_from_nxpv(const char* nxpv_path, int64_t* timestamp, int* is_valid) {
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
int32_t pt_generate_nxpv_file(const char* plugin_path, int64_t timestamp, int is_valid) {
    if (plugin_path == NULL) {
        return -1;
    }
    
    char nxpv_path[1024];
    if (pt_build_nxpv_path(plugin_path, nxpv_path, sizeof(nxpv_path)) != 0) {
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

