/**
 * @file config_rules_scanner.c
 * @brief 规则配置扫描器 / Rules Configuration Scanner / Regeln-Konfigurationsscanner
 */

#include "config_rules_scanner.h"
#include "../common/config_errors.h"
#include "../common/config_parser_common.h"
#include "pointer_transfer_context.h"
#include "pointer_transfer_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief 扫描配置文件查找设置 / Scan config file for settings / Konfigurationsdatei nach Einstellungen scannen
 */
void scan_config_for_settings(FILE* fp) {
    pointer_transfer_context_t* ctx = get_global_context();
    if (ctx == NULL) {
        return;
    }
    
    char* scan_buffer = (char*)malloc(4096);
    if (scan_buffer == NULL) {
        return;
    }
    
    int in_entry_section_scan = 0;
    while (fgets(scan_buffer, 4096, fp) != NULL) {
        char* trimmed = scan_buffer;
        size_t len = strlen(trimmed);
        if (len > 0 && trimmed[len - 1] == '\n') {
            trimmed[len - 1] = '\0';
            len--;
        }
        if (len > 0 && trimmed[len - 1] == '\r') {
            trimmed[len - 1] = '\0';
            len--;
        }
        trim_string(trimmed);
        
        if (trimmed[0] == '#' || trimmed[0] == '\0') {
            continue;
        }
        
        if (len >= 2 && trimmed[0] == '[' && trimmed[len - 1] == ']') {
            char section[512];
            size_t section_len = len - 2;
            if (section_len >= sizeof(section)) {
                section_len = sizeof(section) - 1;
            }
            memcpy(section, trimmed + 1, section_len);
            section[section_len] = '\0';
            in_entry_section_scan = (strcmp(section, "EntryPlugin") == 0) ? 1 : 0;
            continue;
        }
        
        if (in_entry_section_scan) {
            char key[512];
            char value[2048];
            if (parse_key_value_simple(trimmed, key, sizeof(key), value, sizeof(value))) {
                trim_string(key);
                trim_string(value);
                if (strcmp(key, "DisableInfoLog") == 0) {
                    ctx->disable_info_log = parse_boolean_value(value);
                } else if (strcmp(key, "EnableValidation") == 0) {
                    ctx->enable_validation = parse_boolean_value(value);
                }
            }
        }
    }
    
    free(scan_buffer);
    rewind(fp);
}

