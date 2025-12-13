/**
 * @file pointer_transfer_config_entry.c
 * @brief 入口插件配置解析 / Entry Plugin Configuration Parsing / Einstiegs-Plugin-Konfigurationsparsing
 */

#include "pointer_transfer_config.h"
#include "pointer_transfer_context.h"
#include "pointer_transfer_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/* 错误码定义 / Error code definitions / Fehlercode-Definitionen */
#define CONFIG_ERR_SUCCESS           0   /**< 成功 / Success / Erfolg */
#define CONFIG_ERR_INVALID_PARAM    -1   /**< 无效参数 / Invalid parameter / Ungültiger Parameter */
#define CONFIG_ERR_FILE_OPEN        -2   /**< 文件打开失败 / File open failed / Dateiöffnung fehlgeschlagen */
#define CONFIG_ERR_MEMORY           -3   /**< 内存分配失败 / Memory allocation failed / Speicherzuweisung fehlgeschlagen */
#define CONFIG_ERR_INCOMPLETE       -4   /**< 配置不完整 / Configuration incomplete / Konfiguration unvollständig */

/**
 * @brief 解析布尔值配置 / Parse boolean configuration / Boolesche Konfiguration parsen
 */
static int parse_boolean_value(const char* value) {
    if (strcmp(value, "1") == 0 || strcmp(value, "true") == 0 || 
        strcmp(value, "True") == 0 || strcmp(value, "TRUE") == 0 ||
        strcmp(value, "yes") == 0 || strcmp(value, "Yes") == 0 || 
        strcmp(value, "YES") == 0 ||
        strcmp(value, "on") == 0 || strcmp(value, "On") == 0 || 
        strcmp(value, "ON") == 0) {
        return 1;
    } else if (strcmp(value, "0") == 0 || strcmp(value, "false") == 0 || 
               strcmp(value, "False") == 0 || strcmp(value, "FALSE") == 0 ||
               strcmp(value, "no") == 0 || strcmp(value, "No") == 0 || 
               strcmp(value, "NO") == 0 ||
               strcmp(value, "off") == 0 || strcmp(value, "Off") == 0 || 
               strcmp(value, "OFF") == 0) {
        return 0;
    } else {
        /* 尝试解析为整数 / Try to parse as integer / Als Ganzzahl parsen versuchen */
        char* endptr = NULL;
        long int_val = strtol(value, &endptr, 10);
        if (endptr != NULL && *endptr == '\0') {
            return (int_val != 0) ? 1 : 0;
        }
        return 0;
    }
}

/**
 * @brief 解析入口插件配置 / Parse entry plugin configuration / Einstiegs-Plugin-Konfiguration parsen
 */
int parse_entry_plugin_config(const char* config_path) {
    if (config_path == NULL) {
        return CONFIG_ERR_INVALID_PARAM;
    }
    
    FILE* fp = fopen(config_path, "r");
    if (fp == NULL) {
        internal_log_write("WARNING", "Failed to open entry plugin config file: %s", config_path);
        return CONFIG_ERR_FILE_OPEN;
    }
    
    internal_log_write("INFO", "Parsing entry plugin config file: %s", config_path);
    
    pointer_transfer_context_t* ctx = get_global_context();
    if (ctx == NULL) {
        internal_log_write("ERROR", "parse_entry_plugin_config: global context is NULL");
        fclose(fp);
        return CONFIG_ERR_INVALID_PARAM;
    }
    
    char* line_buffer = NULL;
    size_t line_buffer_size = 4096;
    char current_section[512] = {0};
    int in_entry_section = 0;
    
    line_buffer = (char*)malloc(line_buffer_size);
    if (line_buffer == NULL) {
        fclose(fp);
        return CONFIG_ERR_MEMORY;
    }
    
    size_t line_number = 0;
    while (fgets(line_buffer, (int)line_buffer_size, fp) != NULL) {
        line_number++;
        char* trimmed_line = line_buffer;
        size_t len = strlen(trimmed_line);
        
        /* 检查行是否被截断 / Check if line was truncated / Prüfen, ob Zeile abgeschnitten wurde */
        if (len == line_buffer_size - 1 && line_buffer[len - 1] != '\n') {
            internal_log_write("WARNING", "parse_entry_plugin_config: line %zu exceeds buffer size (%zu), may be truncated", 
                line_number, line_buffer_size);
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
                internal_log_write("WARNING", "parse_entry_plugin_config: line %zu section name exceeds buffer size (%zu), truncated", 
                    line_number, sizeof(current_section));
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
                
                /* 检查键值长度 / Check key/value length / Schlüssel-/Wertlänge prüfen */
                size_t key_len = strlen(key);
                size_t value_len = strlen(value);
                if (key_len >= sizeof(key) - 1) {
                    internal_log_write("WARNING", "parse_entry_plugin_config: line %zu key exceeds buffer size (%zu), truncated", 
                        line_number, sizeof(key));
                }
                if (value_len >= sizeof(value) - 1) {
                    internal_log_write("WARNING", "parse_entry_plugin_config: line %zu value exceeds buffer size (%zu), truncated", 
                        line_number, sizeof(value));
                }
                
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
                } else if (strcmp(key, "DisableInfoLog") == 0) {
                    int disable_info = parse_boolean_value(value);
                    ctx->disable_info_log = disable_info;
                    internal_log_write("INFO", "DisableInfoLog configuration: %d (%s)", 
                                      disable_info, disable_info ? "INFO logs disabled" : "INFO logs enabled");
                } else if (strcmp(key, "EnableValidation") == 0) {
                    int enable_validation = parse_boolean_value(value);
                    ctx->enable_validation = enable_validation;
                    internal_log_write("INFO", "EnableValidation configuration: %d (%s)", 
                                      enable_validation, enable_validation ? "validation enabled" : "validation disabled");
                } else if (strcmp(key, "IgnorePlugins") == 0) {
                    /* 解析忽略插件列表（逗号分隔） / Parse ignored plugins list (comma-separated) / Liste der ignorierten Plugins parsen (kommagetrennt) */
                    if (value != NULL && strlen(value) > 0) {
                        char* value_copy = allocate_string(value);
                        if (value_copy != NULL) {
                            char* token = strtok(value_copy, ",");
                            while (token != NULL) {
                                /* 去除前后空白字符 / Trim whitespace / Leerzeichen entfernen */
                                trim_string(token);
                                if (strlen(token) > 0) {
                                    /* 添加到忽略列表 / Add to ignore list / Zur Ignorierliste hinzufügen */
                                    pointer_transfer_context_t* ctx = get_global_context();
                                    if (ctx != NULL) {
                                        /* 检查是否需要扩容 / Check if capacity expansion is needed / Prüfen, ob Kapazitätserweiterung erforderlich ist */
                                        if (ctx->ignore_plugin_count >= ctx->ignore_plugin_capacity) {
                                            size_t new_capacity = ctx->ignore_plugin_capacity == 0 ? 8 : ctx->ignore_plugin_capacity * 2;
                                            
                                            if (new_capacity < ctx->ignore_plugin_capacity || new_capacity > SIZE_MAX / sizeof(char*)) {
                                                internal_log_write("ERROR", "IgnorePlugins: capacity overflow detected (current=%zu, new=%zu)", 
                                                                  ctx->ignore_plugin_capacity, new_capacity);
                                                token = strtok(NULL, ",");
                                                continue;
                                            }
                                            
                                            char** new_plugins = (char**)realloc(ctx->ignore_plugins, new_capacity * sizeof(char*));
                                            if (new_plugins == NULL) {
                                                internal_log_write("ERROR", "IgnorePlugins: failed to allocate memory for ignore plugins array (new_capacity=%zu)", new_capacity);
                                                token = strtok(NULL, ",");
                                                continue;
                                            }
                                            
                                            memset(new_plugins + ctx->ignore_plugin_count, 0, (new_capacity - ctx->ignore_plugin_capacity) * sizeof(char*));
                                            ctx->ignore_plugins = new_plugins;
                                            ctx->ignore_plugin_capacity = new_capacity;
                                        }
                                        
                                        /* 检查是否已存在 / Check if already exists / Prüfen, ob bereits vorhanden */
                                        int already_exists = 0;
                                        for (size_t i = 0; i < ctx->ignore_plugin_count; i++) {
                                            if (ctx->ignore_plugins[i] != NULL && strcmp(ctx->ignore_plugins[i], token) == 0) {
                                                already_exists = 1;
                                                break;
                                            }
                                        }
                                        
                                        if (!already_exists) {
                                            char* token_copy = allocate_string(token);
                                            if (token_copy != NULL) {
                                                ctx->ignore_plugins[ctx->ignore_plugin_count] = token_copy;
                                                ctx->ignore_plugin_count++;
                                                internal_log_write("INFO", "Added ignored plugin path from config: %s", token);
                                            } else {
                                                internal_log_write("WARNING", "IgnorePlugins: failed to allocate memory for plugin path: %s", token);
                                            }
                                        }
                                    }
                                }
                                token = strtok(NULL, ",");
                            }
                            free(value_copy);
                        }
                    }
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
        return CONFIG_ERR_SUCCESS;
    } else {
        internal_log_write("WARNING", "Entry plugin config incomplete");
        return CONFIG_ERR_INCOMPLETE;
    }
}

