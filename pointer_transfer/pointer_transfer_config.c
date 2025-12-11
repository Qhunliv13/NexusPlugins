/**
 * @file pointer_transfer_config.c
 * @brief 指针传递插件配置解析实现 / Pointer Transfer Plugin Configuration Parsing Implementation / Zeigerübertragungs-Plugin-Konfigurationsparsing-Implementierung
 */

#include "pointer_transfer_config.h"
#include "pointer_transfer_context.h"
#include "pointer_transfer_utils.h"
#include "pointer_transfer_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/**
 * @brief 查询.nxpt文件加载状态 / Query .nxpt file load status / .nxpt-Datei-Ladestatus abfragen
 */
int is_nxpt_loaded(const char* plugin_name) {
    if (plugin_name == NULL) {
        return 0;
    }
    
    pointer_transfer_context_t* ctx = get_global_context();
    if (ctx->loaded_nxpt_files != NULL) {
        for (size_t i = 0; i < ctx->loaded_nxpt_count; i++) {
            if (ctx->loaded_nxpt_files[i].plugin_name != NULL &&
                strcmp(ctx->loaded_nxpt_files[i].plugin_name, plugin_name) == 0) {
                return ctx->loaded_nxpt_files[i].loaded;
            }
        }
    }
    
    return 0;
}

/**
 * @brief 标记.nxpt文件为已加载 / Mark .nxpt file as loaded / .nxpt-Datei als geladen markieren
 */
int mark_nxpt_loaded(const char* plugin_name, const char* nxpt_path) {
    if (plugin_name == NULL || nxpt_path == NULL) {
        return -1;
    }
    
    if (is_nxpt_loaded(plugin_name)) {
        return 0;
    }
    
    pointer_transfer_context_t* ctx = get_global_context();
    if (ctx->loaded_nxpt_count >= ctx->loaded_nxpt_capacity) {
        size_t new_capacity = ctx->loaded_nxpt_capacity == 0 ? 8 : ctx->loaded_nxpt_capacity * 2;
        loaded_nxpt_info_t* new_files = (loaded_nxpt_info_t*)realloc(ctx->loaded_nxpt_files, new_capacity * sizeof(loaded_nxpt_info_t));
        if (new_files == NULL) {
            return -1;
        }
        memset(new_files + ctx->loaded_nxpt_count, 0, (new_capacity - ctx->loaded_nxpt_count) * sizeof(loaded_nxpt_info_t));
        ctx->loaded_nxpt_files = new_files;
        ctx->loaded_nxpt_capacity = new_capacity;
    }
    
    loaded_nxpt_info_t* info = &ctx->loaded_nxpt_files[ctx->loaded_nxpt_count];
    info->plugin_name = allocate_string(plugin_name);
    info->nxpt_path = allocate_string(nxpt_path);
    info->loaded = 1;
    
    if (info->plugin_name == NULL || info->nxpt_path == NULL) {
        if (info->plugin_name != NULL) free(info->plugin_name);
        if (info->nxpt_path != NULL) free(info->nxpt_path);
        return -1;
    }
    
    ctx->loaded_nxpt_count++;
    return 0;
}

/**
 * @brief 解析入口插件配置 / Parse entry plugin configuration / Einstiegs-Plugin-Konfiguration parsen
 */
int parse_entry_plugin_config(const char* config_path) {
    if (config_path == NULL) {
        return -1;
    }
    
    FILE* fp = fopen(config_path, "r");
    if (fp == NULL) {
        internal_log_write("WARNING", "Failed to open entry plugin config file: %s", config_path);
        return -1;
    }
    
    internal_log_write("INFO", "Parsing entry plugin config file: %s", config_path);
    
    pointer_transfer_context_t* ctx = get_global_context();
    char* line_buffer = NULL;
    size_t line_buffer_size = 4096;
    char current_section[512] = {0};
    int in_entry_section = 0;
    
    line_buffer = (char*)malloc(line_buffer_size);
    if (line_buffer == NULL) {
        fclose(fp);
        return -1;
    }
    
    while (fgets(line_buffer, (int)line_buffer_size, fp) != NULL) {
        char* trimmed_line = line_buffer;
        size_t len = strlen(trimmed_line);
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
            }
            memcpy(current_section, trimmed_line + 1, section_len);
            current_section[section_len] = '\0';
            
            if (strcmp(current_section, "EntryPlugin") == 0) {
                in_entry_section = 1;
            } else {
                in_entry_section = 0;
            }
            continue;
        }
        
        if (in_entry_section) {
            char key[512];
            char value[2048];
            if (parse_key_value_simple(trimmed_line, key, sizeof(key), value, sizeof(value))) {
                trim_string(key);
                trim_string(value);
                
                if (strcmp(key, "PluginName") == 0) {
                    if (ctx->entry_plugin_name != NULL) free(ctx->entry_plugin_name);
                    ctx->entry_plugin_name = allocate_string(value);
                } else if (strcmp(key, "PluginPath") == 0) {
                    if (ctx->entry_plugin_path != NULL) free(ctx->entry_plugin_path);
                    ctx->entry_plugin_path = allocate_string(value);
                } else if (strcmp(key, "NxptPath") == 0) {
                    if (ctx->entry_nxpt_path != NULL) free(ctx->entry_nxpt_path);
                    ctx->entry_nxpt_path = allocate_string(value);
                } else if (strcmp(key, "AutoRunInterface") == 0) {
                    if (ctx->entry_auto_run_interface != NULL) free(ctx->entry_auto_run_interface);
                    ctx->entry_auto_run_interface = allocate_string(value);
                }
            }
        }
    }
    
    free(line_buffer);
    fclose(fp);
    
    if (ctx->entry_plugin_name != NULL && ctx->entry_nxpt_path != NULL) {
        internal_log_write("INFO", "Entry plugin config: name=%s, path=%s, nxpt=%s", 
            ctx->entry_plugin_name, 
            ctx->entry_plugin_path != NULL ? ctx->entry_plugin_path : "not specified",
            ctx->entry_nxpt_path);
        return 0;
    } else {
        internal_log_write("WARNING", "Entry plugin config incomplete");
        return -1;
    }
}

/**
 * @brief 加载传递规则配置文件 / Load transfer rules configuration file / Übertragungsregel-Konfigurationsdatei laden
 */
int load_transfer_rules(const char* config_path) {
    if (config_path == NULL) {
        return -1;
    }
    
    FILE* fp = fopen(config_path, "r");
    if (fp == NULL) {
        internal_log_write("WARNING", "Failed to open transfer rules file: %s", config_path);
        return -1;
    }
    
    internal_log_write("INFO", "Opening transfer rules file: %s", config_path);
    
    pointer_transfer_context_t* ctx = get_global_context();
    char* line_buffer = NULL;
    size_t line_buffer_size = 4096;
    char current_section[512] = {0};
    int current_rule_index = -1;
    size_t line_count = 0;
    
    line_buffer = (char*)malloc(line_buffer_size);
    if (line_buffer == NULL) {
        fclose(fp);
        return -1;
    }
    
    /* 动态数组用于临时存储规则 / Dynamic array for temporary rule storage / Dynamisches Array zur temporären Regelspeicherung */
    size_t temp_rules_capacity = 0;
    size_t temp_rules_count = 0;
    pointer_transfer_rule_t* temp_rules = NULL;
    
    /* 一次扫描解析规则配置 / Single pass parsing of rule configuration / Einmaliges Parsen der Regelkonfiguration */
    while (fgets(line_buffer, (int)line_buffer_size, fp) != NULL) {
        line_count++;
        char* trimmed_line = line_buffer;
        size_t len = strlen(trimmed_line);
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
            }
            memcpy(current_section, trimmed_line + 1, section_len);
            current_section[section_len] = '\0';
            
            if (strncmp(current_section, "TransferRule_", 13) == 0) {
                char* endptr = NULL;
                long parsed_index = strtol(current_section + 13, &endptr, 10);
                if (endptr != NULL && *endptr == '\0' && parsed_index >= 0 && parsed_index <= INT_MAX) {
                    current_rule_index = (int)parsed_index;
                    
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
                            return -1;
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
                            return -1;
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
                    temp_rules_count++;
                } else {
                    current_rule_index = -1;
                }
            } else {
                current_rule_index = -1;
            }
            continue;
        }
        
        if (current_rule_index >= 0 && temp_rules_count > 0) {
            char key[512];
            char value[2048];
            if (parse_key_value_simple(trimmed_line, key, sizeof(key), value, sizeof(value))) {
                trim_string(key);
                trim_string(value);
                
                pointer_transfer_rule_t* rule = &temp_rules[temp_rules_count - 1];
                
                if (strcmp(key, "SourcePlugin") == 0) {
                    rule->source_plugin = allocate_string(value);
                } else if (strcmp(key, "SourceInterface") == 0) {
                    rule->source_interface = allocate_string(value);
                } else if (strcmp(key, "SourceParamIndex") == 0) {
                    char* endptr = NULL;
                    long parsed_val = strtol(value, &endptr, 10);
                    if (endptr != NULL && *endptr == '\0' && parsed_val >= INT_MIN && parsed_val <= INT_MAX) {
                        rule->source_param_index = (int)parsed_val;
                    }
                } else if (strcmp(key, "TargetPlugin") == 0) {
                    rule->target_plugin = allocate_string(value);
                } else if (strcmp(key, "TargetPluginPath") == 0) {
                    rule->target_plugin_path = allocate_string(value);
                } else if (strcmp(key, "TargetInterface") == 0) {
                    rule->target_interface = allocate_string(value);
                } else if (strcmp(key, "TargetParamIndex") == 0) {
                    char* endptr = NULL;
                    long parsed_val = strtol(value, &endptr, 10);
                    if (endptr != NULL && *endptr == '\0' && parsed_val >= INT_MIN && parsed_val <= INT_MAX) {
                        rule->target_param_index = (int)parsed_val;
                    }
                } else if (strcmp(key, "TargetParamValue") == 0) {
                    rule->target_param_value = allocate_string(value);
                } else if (strcmp(key, "Description") == 0) {
                    rule->description = allocate_string(value);
                } else if (strcmp(key, "MulticastGroup") == 0) {
                    rule->multicast_group = allocate_string(value);
                } else if (strcmp(key, "TransferMode") == 0) {
                    if (strcmp(value, "broadcast") == 0 || strcmp(value, "Broadcast") == 0) {
                        rule->transfer_mode = TRANSFER_MODE_BROADCAST;
                    } else if (strcmp(value, "multicast") == 0 || strcmp(value, "Multicast") == 0) {
                        rule->transfer_mode = TRANSFER_MODE_MULTICAST;
                    } else {
                        rule->transfer_mode = TRANSFER_MODE_UNICAST;
                    }
                } else if (strcmp(key, "Enabled") == 0) {
                    rule->enabled = (strcmp(value, "true") == 0 || strcmp(value, "1") == 0) ? 1 : 0;
                } else if (strcmp(key, "Condition") == 0) {
                    rule->condition = allocate_string(value);
                }
            }
        }
    }
    
    free(line_buffer);
    fclose(fp);
    
    size_t start_rule_index = ctx->rule_count;
    /* 检查size_t加法溢出 / Check size_t addition overflow / size_t-Additionsüberlauf prüfen */
    if (start_rule_index > SIZE_MAX - temp_rules_count) {
        internal_log_write("ERROR", "Rule count overflow detected: start_rule_index=%zu, temp_rules_count=%zu, sum would exceed SIZE_MAX", 
                     start_rule_index, temp_rules_count);
        if (temp_rules != NULL) {
            for (size_t i = 0; i < temp_rules_count; i++) {
                free_single_rule(&temp_rules[i]);
            }
            free(temp_rules);
        }
        return -1;
    }
    size_t new_rule_count = start_rule_index + temp_rules_count;
    
    if (new_rule_count > ctx->rule_capacity) {
        size_t needed = new_rule_count;
        while (ctx->rule_capacity < needed) {
            if (expand_rules_capacity() != 0) {
                for (size_t i = 0; i < temp_rules_count; i++) {
                    free_single_rule(&temp_rules[i]);
                }
                free(temp_rules);
                return -1;
            }
        }
    }
    
    for (size_t i = 0; i < temp_rules_count; i++) {
        size_t new_index = start_rule_index + i;
        if (new_index < ctx->rule_capacity) {
            pointer_transfer_rule_t* src_rule = &temp_rules[i];
            pointer_transfer_rule_t* dst_rule = &ctx->rules[new_index];
            memset(dst_rule, 0, sizeof(pointer_transfer_rule_t));
            
            dst_rule->source_param_index = src_rule->source_param_index;
            dst_rule->target_param_index = src_rule->target_param_index;
            dst_rule->transfer_mode = src_rule->transfer_mode;
            dst_rule->enabled = src_rule->enabled;
            
            if (src_rule->source_plugin != NULL) {
                dst_rule->source_plugin = allocate_string(src_rule->source_plugin);
            }
            if (src_rule->source_interface != NULL) {
                dst_rule->source_interface = allocate_string(src_rule->source_interface);
            }
            if (src_rule->target_plugin != NULL) {
                dst_rule->target_plugin = allocate_string(src_rule->target_plugin);
            }
            if (src_rule->target_plugin_path != NULL) {
                dst_rule->target_plugin_path = allocate_string(src_rule->target_plugin_path);
            }
            if (src_rule->target_interface != NULL) {
                dst_rule->target_interface = allocate_string(src_rule->target_interface);
            }
            if (src_rule->target_param_value != NULL) {
                dst_rule->target_param_value = allocate_string(src_rule->target_param_value);
            }
            if (src_rule->description != NULL) {
                dst_rule->description = allocate_string(src_rule->description);
            }
            if (src_rule->multicast_group != NULL) {
                dst_rule->multicast_group = allocate_string(src_rule->multicast_group);
            }
            if (src_rule->condition != NULL) {
                dst_rule->condition = allocate_string(src_rule->condition);
            }
        }
    }
    
    for (size_t i = 0; i < temp_rules_count; i++) {
        free_single_rule(&temp_rules[i]);
    }
    free(temp_rules);
    
    ctx->rule_count = new_rule_count;
    
    /* TODO: 待实现规则索引构建功能
     * 在此处构建rule_index索引表，索引键格式: "source_plugin.source_interface.source_param_index"
     * 实现后可优化后续规则查找性能，将O(n)线性查找优化为O(1)或O(log n)
     * 建议实现方案:
     *   1. 哈希表方案: 使用字符串哈希，O(1)查找
     *   2. 排序数组+二分查找: 按key排序，O(log n)查找
     *   3. 多级索引: 按plugin -> interface -> param_index分层索引
     */
    
    internal_log_write("INFO", "Parsed %zu lines, found %zu rules, total rules: %zu", line_count, temp_rules_count, ctx->rule_count);
    
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
    return 0;
}

