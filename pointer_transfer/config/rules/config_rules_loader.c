/**
 * @file config_rules_loader.c
 * @brief 规则配置加载器 / Rules Configuration Loader / Regeln-Konfigurationslader
 */

#include "config_rules_loader.h"
#include "../common/config_errors.h"
#include "../common/config_parser_common.h"
#include "config_rules_scanner.h"
#include "config_rules_parser.h"
#include "config_rules_merger.h"
#include "pointer_transfer_config.h"
#include "pointer_transfer_context.h"
#include "pointer_transfer_utils.h"
#include "pointer_transfer_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>

/**
 * @brief 加载传递规则配置文件 / Load transfer rules configuration file / Übertragungsregel-Konfigurationsdatei laden
 */
int load_transfer_rules(const char* config_path) {
    if (config_path == NULL) {
        return CONFIG_ERR_INVALID_PARAM;
    }
    
    FILE* fp = fopen(config_path, "r");
    if (fp == NULL) {
        internal_log_write("WARNING", "Failed to open transfer rules file: %s", config_path);
        return CONFIG_ERR_FILE_OPEN;
    }
    
    pointer_transfer_context_t* ctx = get_global_context();
    if (ctx == NULL) {
        internal_log_write("ERROR", "load_transfer_rules: global context is NULL");
        fclose(fp);
        return CONFIG_ERR_INVALID_PARAM;
    }
    
    /* 首先快速扫描配置文件，查找DisableInfoLog配置 / First quickly scan config file to find DisableInfoLog configuration / Zuerst Konfigurationsdatei schnell scannen, um DisableInfoLog-Konfiguration zu finden */
    scan_config_for_settings(fp);
    
    internal_log_write("INFO", "Opening transfer rules file: %s", config_path);
    
    char* line_buffer = NULL;
    size_t line_buffer_size = 4096;
    char current_section[512] = {0};
    int current_rule_index = -1;
    int in_entry_section = 0;
    size_t line_count = 0;
    
    line_buffer = (char*)malloc(line_buffer_size);
    if (line_buffer == NULL) {
        fclose(fp);
        return CONFIG_ERR_MEMORY;
    }
    
    /* 动态数组用于临时存储规则 / Dynamic array for temporary rule storage / Dynamisches Array zur temporären Regelspeicherung */
    size_t temp_rules_capacity = 0;
    size_t temp_rules_count = 0;
    pointer_transfer_rule_t* temp_rules = NULL;
    
    /* 用于验证段索引连续性和一致性 / For validating section index continuity and consistency / Zur Validierung der Abschnittsindex-Kontinuität und -Konsistenz */
    int max_seen_index = -1;
    int expected_next_index = 0;
    
    /* 一次扫描解析规则配置 / Single pass parsing of rule configuration / Einmaliges Parsen der Regelkonfiguration */
    while (fgets(line_buffer, (int)line_buffer_size, fp) != NULL) {
        line_count++;
        char* trimmed_line = line_buffer;
        size_t len = strlen(trimmed_line);
        
        /* 检查行是否被截断 / Check if line was truncated / Prüfen, ob Zeile abgeschnitten wurde */
        if (len == line_buffer_size - 1 && line_buffer[len - 1] != '\n') {
            internal_log_write("WARNING", "load_transfer_rules: line %zu exceeds buffer size (%zu), may be truncated", 
                line_count, line_buffer_size);
        }
        
        if (len > 0 && trimmed_line[len - 1] == '\n') {
            trimmed_line[len - 1] = '\0';
            len--;
        }
        if (len > 0 && trimmed_line[len - 1] == '\r') {
            trimmed_line[len - 1] = '\0';
            len--;
        }
        trim_string(trimmed_line);
        len = strlen(trimmed_line);
        
        if (trimmed_line[0] == '#' || trimmed_line[0] == '\0') {
            continue;
        }
        
        if (len >= 2 && trimmed_line[0] == '[' && trimmed_line[len - 1] == ']') {
            size_t section_len = len - 2;
            if (section_len >= sizeof(current_section)) {
                section_len = sizeof(current_section) - 1;
                internal_log_write("WARNING", "load_transfer_rules: line %zu section name exceeds buffer size (%zu), truncated", 
                    line_count, sizeof(current_section));
            }
            memcpy(current_section, trimmed_line + 1, section_len);
            current_section[section_len] = '\0';
            
            /* 检查是否是EntryPlugin段 / Check if it's EntryPlugin section / Prüfen, ob es EntryPlugin-Abschnitt ist */
            if (strcmp(current_section, "EntryPlugin") == 0) {
                in_entry_section = 1;
                current_rule_index = -1;
            } else {
                in_entry_section = 0;
            }
            
            if (strncmp(current_section, "TransferRule_", 13) == 0) {
                char* endptr = NULL;
                long parsed_index = strtol(current_section + 13, &endptr, 10);
                if (endptr != NULL && *endptr == '\0' && parsed_index >= 0 && parsed_index <= INT_MAX) {
                    current_rule_index = (int)parsed_index;
                    
                    /* 验证段索引连续性和一致性 / Validate section index continuity and consistency / Abschnittsindex-Kontinuität und -Konsistenz validieren */
                    if (current_rule_index < expected_next_index) {
                        internal_log_write("WARNING", "load_transfer_rules: line %zu section index %d is less than expected next index %d, may indicate duplicate or out-of-order sections", 
                            line_count, current_rule_index, expected_next_index);
                    } else if (current_rule_index > expected_next_index) {
                        internal_log_write("WARNING", "load_transfer_rules: line %zu section index %d skips expected index %d, missing sections detected", 
                            line_count, current_rule_index, expected_next_index);
                    }
                    
                    if (current_rule_index > max_seen_index) {
                        max_seen_index = current_rule_index;
                    }
                    expected_next_index = current_rule_index + 1;
                    
                    /* 扩展数组以容纳新规则 / Expand array to accommodate new rule / Array erweitern, um neue Regel aufzunehmen */
                    if (temp_rules_count >= temp_rules_capacity) {
                        size_t new_capacity = temp_rules_capacity == 0 ? 8 : temp_rules_capacity * 2;
                        if (new_capacity > SIZE_MAX / sizeof(pointer_transfer_rule_t)) {
                            internal_log_write("ERROR", "Rule array capacity overflow detected");
                            free(line_buffer);
                            fclose(fp);
                            if (temp_rules != NULL) {
                                for (size_t i = 0; i < temp_rules_count; i++) {
                                    free_single_rule(&temp_rules[i]);
                                }
                                free(temp_rules);
                            }
                            return CONFIG_ERR_OVERFLOW;
                        }
                        
                        pointer_transfer_rule_t* new_rules = (pointer_transfer_rule_t*)realloc(temp_rules, new_capacity * sizeof(pointer_transfer_rule_t));
                        if (new_rules == NULL) {
                            free(line_buffer);
                            fclose(fp);
                            if (temp_rules != NULL) {
                                for (size_t i = 0; i < temp_rules_count; i++) {
                                    free_single_rule(&temp_rules[i]);
                                }
                                free(temp_rules);
                            }
                            return CONFIG_ERR_MEMORY;
                        }
                        memset(new_rules + temp_rules_count, 0, (new_capacity - temp_rules_count) * sizeof(pointer_transfer_rule_t));
                        temp_rules = new_rules;
                        temp_rules_capacity = new_capacity;
                    }
                    
                    /* 初始化新规则 / Initialize new rule / Neue Regel initialisieren */
                    temp_rules[temp_rules_count].source_param_index = -1;
                    temp_rules[temp_rules_count].target_param_index = -1;
                    temp_rules[temp_rules_count].transfer_mode = TRANSFER_MODE_UNICAST;
                    temp_rules[temp_rules_count].enabled = 1;
                    temp_rules[temp_rules_count].cache_self = 0;
                    temp_rules[temp_rules_count].set_group = NULL;
                    temp_rules_count++;
                } else {
                    current_rule_index = -1;
                }
            } else {
                current_rule_index = -1;
            }
            continue;
        }
        
        /* 如果在EntryPlugin段中，解析DisableInfoLog配置 / If in EntryPlugin section, parse DisableInfoLog configuration / Wenn im EntryPlugin-Abschnitt, DisableInfoLog-Konfiguration parsen */
        if (in_entry_section) {
            char key[512];
            char value[2048];
            if (parse_key_value_simple(trimmed_line, key, sizeof(key), value, sizeof(value))) {
                trim_string(key);
                trim_string(value);
                
                if (strcmp(key, "DisableInfoLog") == 0) {
                    ctx->disable_info_log = parse_boolean_value(value);
                    internal_log_write("INFO", "DisableInfoLog configuration: %d (%s)", 
                                      ctx->disable_info_log, ctx->disable_info_log ? "INFO logs disabled" : "INFO logs enabled");
                }
            }
            continue;
        }
        
        if (current_rule_index >= 0 && temp_rules_count > 0) {
            char key[512];
            char value[2048];
            if (parse_key_value_simple(trimmed_line, key, sizeof(key), value, sizeof(value))) {
                trim_string(key);
                trim_string(value);
                
                /* 检查键值长度 / Check key/value length / Schlüssel-/Wertlänge prüfen */
                size_t key_len = strlen(key);
                size_t value_len = strlen(value);
                if (key_len >= sizeof(key) - 1) {
                    internal_log_write("WARNING", "load_transfer_rules: line %zu key exceeds buffer size (%zu), truncated", 
                        line_count, sizeof(key));
                }
                if (value_len >= sizeof(value) - 1) {
                    internal_log_write("WARNING", "load_transfer_rules: line %zu value exceeds buffer size (%zu), truncated", 
                        line_count, sizeof(value));
                }
                
                pointer_transfer_rule_t* rule = &temp_rules[temp_rules_count - 1];
                parse_rule_key_value(rule, key, value);
                
                if (strcmp(key, "SetGroup") == 0) {
                    internal_log_write("INFO", "Parsed SetGroup=%s for rule index %d (temp_rules_count=%zu)", 
                        value, current_rule_index, temp_rules_count);
                }
            }
        }
    }
    
    free(line_buffer);
    fclose(fp);
    
    /* 合并规则到上下文 / Merge rules to context / Regeln in Kontext zusammenführen */
    int merge_result = merge_rules_to_context(temp_rules, temp_rules_count, max_seen_index);
    
    if (temp_rules != NULL) {
        for (size_t i = 0; i < temp_rules_count; i++) {
            free_single_rule(&temp_rules[i]);
        }
        free(temp_rules);
    }
    
    if (merge_result != CONFIG_ERR_SUCCESS) {
        return merge_result;
    }
    
    internal_log_write("INFO", "Parsed %zu lines, found %zu rules, total rules: %zu", line_count, temp_rules_count, ctx->rule_count);
    
    size_t start_rule_index = ctx->rule_count - temp_rules_count;
    for (size_t i = start_rule_index; i < ctx->rule_count; i++) {
        pointer_transfer_rule_t* rule = &ctx->rules[i];
        if (rule->transfer_mode == 0) {
            rule->transfer_mode = TRANSFER_MODE_UNICAST;
        }
        if (rule->source_plugin != NULL && rule->target_plugin != NULL) {
            const char* mode_str = "UNKNOWN";
            if (rule->transfer_mode == TRANSFER_MODE_UNICAST) {
                mode_str = "UNICAST";
            } else if (rule->transfer_mode == TRANSFER_MODE_BROADCAST) {
                mode_str = "BROADCAST";
            } else if (rule->transfer_mode == TRANSFER_MODE_MULTICAST) {
                mode_str = "MULTICAST";
            }
            internal_log_write("INFO", "Rule %zu: %s.%s[%d] -> %s.%s[%d], mode=%s (%d)", 
                i, rule->source_plugin, rule->source_interface, rule->source_param_index,
                rule->target_plugin, rule->target_interface, rule->target_param_index,
                mode_str, (int)rule->transfer_mode);
        }
    }
    
    internal_log_write("INFO", "Loaded %zu transfer rules from %s", temp_rules_count, config_path);
    return CONFIG_ERR_SUCCESS;
}

