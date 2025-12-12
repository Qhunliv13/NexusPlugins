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
#include <stdint.h>

/* 错误码定义 / Error code definitions / Fehlercode-Definitionen */
#define CONFIG_ERR_SUCCESS           0   /**< 成功 / Success / Erfolg */
#define CONFIG_ERR_INVALID_PARAM    -1   /**< 无效参数 / Invalid parameter / Ungültiger Parameter */
#define CONFIG_ERR_FILE_OPEN        -2   /**< 文件打开失败 / File open failed / Dateiöffnung fehlgeschlagen */
#define CONFIG_ERR_MEMORY           -3   /**< 内存分配失败 / Memory allocation failed / Speicherzuweisung fehlgeschlagen */
#define CONFIG_ERR_INCOMPLETE       -4   /**< 配置不完整 / Configuration incomplete / Konfiguration unvollständig */
#define CONFIG_ERR_OVERFLOW         -5   /**< 溢出错误 / Overflow error / Überlauffehler */
#define CONFIG_ERR_INDEX_MISMATCH   -6   /**< 索引不匹配 / Index mismatch / Index-Fehlanpassung */

/* 哈希表常量 / Hash table constants / Hash-Tabellen-Konstanten */
#define NXPT_HASH_TABLE_INITIAL_SIZE 8
#define NXPT_HASH_TABLE_LOAD_FACTOR 0.75

/* 前向声明 / Forward declarations / Vorwärtsdeklarationen */
static uint64_t hash_plugin_name(const char* plugin_name);
static int ensure_nxpt_hash_table_capacity(nxpt_hash_table_t* hash_table);
static int insert_into_nxpt_hash_table(nxpt_hash_table_t* hash_table, uint64_t hash_key, size_t array_index);
static void remove_from_nxpt_hash_table(nxpt_hash_table_t* hash_table, size_t array_index, const char* plugin_name);

/**
 * @brief 计算插件名称的哈希值（FNV-1a算法）/ Calculate hash value of plugin name (FNV-1a algorithm) / Hash-Wert des Plugin-Namens berechnen (FNV-1a-Algorithmus)
 */
static uint64_t hash_plugin_name(const char* plugin_name) {
    if (plugin_name == NULL) {
        return 0;
    }
    
    uint64_t hash = 14695981039346656037ULL; /* FNV偏移基数 / FNV offset basis / FNV-Offset-Basis */
    const char* p = plugin_name;
    
    while (*p != '\0') {
        hash ^= (uint64_t)(unsigned char)(*p);
        hash *= 1099511628211ULL; /* FNV质数 / FNV prime / FNV-Primzahl */
        p++;
    }
    
    return hash;
}

/**
 * @brief 确保哈希表有足够容量 / Ensure hash table has sufficient capacity / Sicherstellen, dass Hash-Tabelle ausreichend Kapazität hat
 */
static int ensure_nxpt_hash_table_capacity(nxpt_hash_table_t* hash_table) {
    if (hash_table == NULL) {
        return -1;
    }
    
    /* 如果哈希表未初始化或需要扩容 / If hash table is not initialized or needs expansion / Wenn Hash-Tabelle nicht initialisiert ist oder erweitert werden muss */
    if (hash_table->bucket_count == 0 || 
        (hash_table->entry_count > 0 && 
         (double)hash_table->entry_count / hash_table->bucket_count > NXPT_HASH_TABLE_LOAD_FACTOR)) {
        size_t new_bucket_count = hash_table->bucket_count == 0 ? NXPT_HASH_TABLE_INITIAL_SIZE : hash_table->bucket_count * 2;
        
        /* 检查溢出 / Check overflow / Überlauf prüfen */
        if (new_bucket_count > SIZE_MAX / sizeof(nxpt_hash_node_t*)) {
            return -1;
        }
        
        nxpt_hash_node_t** new_buckets = (nxpt_hash_node_t**)calloc(new_bucket_count, sizeof(nxpt_hash_node_t*));
        if (new_buckets == NULL) {
            return -1;
        }
        
        /* 重新哈希所有现有条目 / Rehash all existing entries / Alle vorhandenen Einträge neu hashen */
        if (hash_table->buckets != NULL && hash_table->entry_count > 0) {
            for (size_t i = 0; i < hash_table->bucket_count; i++) {
                nxpt_hash_node_t* node = hash_table->buckets[i];
                while (node != NULL) {
                    nxpt_hash_node_t* next = node->next;
                    size_t new_bucket = node->hash_key % new_bucket_count;
                    node->next = new_buckets[new_bucket];
                    new_buckets[new_bucket] = node;
                    node = next;
                }
            }
            free(hash_table->buckets);
        }
        
        hash_table->buckets = new_buckets;
        hash_table->bucket_count = new_bucket_count;
    }
    
    return 0;
}

/**
 * @brief 插入条目到哈希表 / Insert entry into hash table / Eintrag in Hash-Tabelle einfügen
 */
static int insert_into_nxpt_hash_table(nxpt_hash_table_t* hash_table, uint64_t hash_key, size_t array_index) {
    if (hash_table == NULL) {
        return -1;
    }
    
    if (ensure_nxpt_hash_table_capacity(hash_table) != 0) {
        return -1;
    }
    
    size_t bucket = hash_key % hash_table->bucket_count;
    
    /* 创建新节点 / Create new node / Neuen Knoten erstellen */
    nxpt_hash_node_t* new_node = (nxpt_hash_node_t*)malloc(sizeof(nxpt_hash_node_t));
    if (new_node == NULL) {
        return -1;
    }
    
    new_node->hash_key = hash_key;
    new_node->array_index = array_index;
    new_node->next = hash_table->buckets[bucket];
    hash_table->buckets[bucket] = new_node;
    hash_table->entry_count++;
    
    return 0;
}

/**
 * @brief 从哈希表中移除条目 / Remove entry from hash table / Eintrag aus Hash-Tabelle entfernen
 */
static void remove_from_nxpt_hash_table(nxpt_hash_table_t* hash_table, size_t array_index, const char* plugin_name) {
    if (hash_table == NULL || hash_table->buckets == NULL || plugin_name == NULL) {
        return;
    }
    
    uint64_t hash_key = hash_plugin_name(plugin_name);
    size_t bucket = hash_key % hash_table->bucket_count;
    
    nxpt_hash_node_t* prev = NULL;
    nxpt_hash_node_t* node = hash_table->buckets[bucket];
    
    while (node != NULL) {
        if (node->array_index == array_index && node->hash_key == hash_key) {
            if (prev == NULL) {
                hash_table->buckets[bucket] = node->next;
            } else {
                prev->next = node->next;
            }
            free(node);
            hash_table->entry_count--;
            return;
        }
        prev = node;
        node = node->next;
    }
}

/**
 * @brief 清理已加载.nxpt文件哈希表 / Cleanup loaded .nxpt files hash table / Geladene .nxpt-Dateien-Hash-Tabelle bereinigen
 */
void free_nxpt_hash_table_internal(void) {
    pointer_transfer_context_t* ctx = get_global_context();
    if (ctx == NULL) {
        return;
    }
    
    nxpt_hash_table_t* hash_table = &ctx->nxpt_hash_table;
    if (hash_table->buckets == NULL) {
        return;
    }
    
    for (size_t i = 0; i < hash_table->bucket_count; i++) {
        nxpt_hash_node_t* node = hash_table->buckets[i];
        while (node != NULL) {
            nxpt_hash_node_t* next = node->next;
            free(node);
            node = next;
        }
    }
    
    free(hash_table->buckets);
    hash_table->buckets = NULL;
    hash_table->bucket_count = 0;
    hash_table->entry_count = 0;
}

/**
 * @brief 查询.nxpt文件加载状态（使用哈希表优化）/ Query .nxpt file load status (optimized with hash table) / .nxpt-Datei-Ladestatus abfragen (mit Hash-Tabelle optimiert)
 */
int is_nxpt_loaded(const char* plugin_name) {
    if (plugin_name == NULL) {
        return 0;
    }
    
    pointer_transfer_context_t* ctx = get_global_context();
    if (ctx == NULL) {
        return 0;
    }
    
    /* 如果数组为空，直接返回 / If array is empty, return directly / Wenn Array leer ist, direkt zurückgeben */
    if (ctx->loaded_nxpt_files == NULL || ctx->loaded_nxpt_count == 0) {
        return 0;
    }
    
    /* 使用哈希表查找 / Use hash table lookup / Hash-Tabelle-Suche verwenden */
    nxpt_hash_table_t* hash_table = &ctx->nxpt_hash_table;
    if (hash_table->buckets != NULL && hash_table->bucket_count > 0) {
        uint64_t hash_key = hash_plugin_name(plugin_name);
        size_t bucket = hash_key % hash_table->bucket_count;
        
        nxpt_hash_node_t* node = hash_table->buckets[bucket];
        while (node != NULL) {
            if (node->hash_key == hash_key && 
                node->array_index < ctx->loaded_nxpt_count &&
                ctx->loaded_nxpt_files[node->array_index].plugin_name != NULL &&
                strcmp(ctx->loaded_nxpt_files[node->array_index].plugin_name, plugin_name) == 0) {
                return ctx->loaded_nxpt_files[node->array_index].loaded;
            }
            node = node->next;
        }
    } else {
        /* 哈希表未初始化，回退到线性搜索 / Hash table not initialized, fall back to linear search / Hash-Tabelle nicht initialisiert, auf lineare Suche zurückgreifen */
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
 * @return 成功返回CONFIG_ERR_SUCCESS，失败返回相应错误码 / Returns CONFIG_ERR_SUCCESS on success, error code on failure / Gibt CONFIG_ERR_SUCCESS bei Erfolg zurück, Fehlercode bei Fehler
 */
int mark_nxpt_loaded(const char* plugin_name, const char* nxpt_path) {
    if (plugin_name == NULL || nxpt_path == NULL) {
        internal_log_write("ERROR", "mark_nxpt_loaded: invalid parameters (plugin_name=%p, nxpt_path=%p)", 
            (void*)plugin_name, (void*)nxpt_path);
        return CONFIG_ERR_INVALID_PARAM;
    }
    
    if (is_nxpt_loaded(plugin_name)) {
        return CONFIG_ERR_SUCCESS;
    }
    
    pointer_transfer_context_t* ctx = get_global_context();
    if (ctx == NULL) {
        internal_log_write("ERROR", "mark_nxpt_loaded: global context is NULL");
        return CONFIG_ERR_INVALID_PARAM;
    }
    
    /* 检查是否需要扩容 / Check if capacity expansion is needed / Prüfen, ob Kapazitätserweiterung erforderlich ist */
    if (ctx->loaded_nxpt_count >= ctx->loaded_nxpt_capacity) {
        size_t new_capacity = ctx->loaded_nxpt_capacity == 0 ? 8 : ctx->loaded_nxpt_capacity * 2;
        
        /* 检查溢出 / Check overflow / Überlauf prüfen */
        if (new_capacity > SIZE_MAX / sizeof(loaded_nxpt_info_t)) {
            internal_log_write("ERROR", "mark_nxpt_loaded: capacity overflow detected (new_capacity=%zu)", new_capacity);
            return CONFIG_ERR_OVERFLOW;
        }
        
        loaded_nxpt_info_t* new_files = (loaded_nxpt_info_t*)realloc(ctx->loaded_nxpt_files, new_capacity * sizeof(loaded_nxpt_info_t));
        if (new_files == NULL) {
            internal_log_write("ERROR", "mark_nxpt_loaded: failed to allocate memory for nxpt files array (new_capacity=%zu)", new_capacity);
            return CONFIG_ERR_MEMORY;
        }
        memset(new_files + ctx->loaded_nxpt_count, 0, (new_capacity - ctx->loaded_nxpt_count) * sizeof(loaded_nxpt_info_t));
        ctx->loaded_nxpt_files = new_files;
        ctx->loaded_nxpt_capacity = new_capacity;
    }
    
    /* 分配字符串并检查失败 / Allocate strings and check for failures / Zeichenfolgen zuweisen und auf Fehler prüfen */
    loaded_nxpt_info_t* info = &ctx->loaded_nxpt_files[ctx->loaded_nxpt_count];
    info->plugin_name = NULL;
    info->nxpt_path = NULL;
    info->loaded = 0;
    
    info->plugin_name = allocate_string(plugin_name);
    if (info->plugin_name == NULL) {
        internal_log_write("ERROR", "mark_nxpt_loaded: failed to allocate memory for plugin_name");
        return CONFIG_ERR_MEMORY;
    }
    
    info->nxpt_path = allocate_string(nxpt_path);
    if (info->nxpt_path == NULL) {
        internal_log_write("ERROR", "mark_nxpt_loaded: failed to allocate memory for nxpt_path");
        free(info->plugin_name);
        info->plugin_name = NULL;
        return CONFIG_ERR_MEMORY;
    }
    
    info->loaded = 1;
    size_t new_index = ctx->loaded_nxpt_count;
    ctx->loaded_nxpt_count++;
    
    /* 将新条目插入哈希表 / Insert new entry into hash table / Neuen Eintrag in Hash-Tabelle einfügen */
    uint64_t hash_key = hash_plugin_name(plugin_name);
    if (insert_into_nxpt_hash_table(&ctx->nxpt_hash_table, hash_key, new_index) != 0) {
        /* 哈希表插入失败不影响核心功能逻辑，仅记录警告信息 / Hash table insertion failure does not affect core functionality logic, only log warning message / Hash-Tabelle-Einfügung-Fehler beeinflusst Kernfunktionalitätslogik nicht, nur Warnmeldung protokollieren */
        internal_log_write("WARNING", "mark_nxpt_loaded: failed to insert into hash table, falling back to linear search");
    }
    
    return CONFIG_ERR_SUCCESS;
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
                    int disable_info = 0;
                    if (strcmp(value, "1") == 0 || strcmp(value, "true") == 0 || 
                        strcmp(value, "True") == 0 || strcmp(value, "TRUE") == 0 ||
                        strcmp(value, "yes") == 0 || strcmp(value, "Yes") == 0 || 
                        strcmp(value, "YES") == 0 ||
                        strcmp(value, "on") == 0 || strcmp(value, "On") == 0 || 
                        strcmp(value, "ON") == 0) {
                        disable_info = 1;
                    } else if (strcmp(value, "0") == 0 || strcmp(value, "false") == 0 || 
                               strcmp(value, "False") == 0 || strcmp(value, "FALSE") == 0 ||
                               strcmp(value, "no") == 0 || strcmp(value, "No") == 0 || 
                               strcmp(value, "NO") == 0 ||
                               strcmp(value, "off") == 0 || strcmp(value, "Off") == 0 || 
                               strcmp(value, "OFF") == 0) {
                        disable_info = 0;
                    } else {
                        /* 尝试解析为整数 / Try to parse as integer / Als Ganzzahl parsen versuchen */
                        char* endptr = NULL;
                        long int_val = strtol(value, &endptr, 10);
                        if (endptr != NULL && *endptr == '\0') {
                            disable_info = (int_val != 0) ? 1 : 0;
                        } else {
                            internal_log_write("WARNING", "parse_entry_plugin_config: invalid DisableInfoLog value '%s', using default (0=enable)", value);
                            disable_info = 0;
                        }
                    }
                    ctx->disable_info_log = disable_info;
                    /* 使用INFO级别输出配置确认信息 / Use INFO level to output configuration confirmation info / INFO-Ebene verwenden, um Konfigurationsbestätigungsinformationen auszugeben */
                    internal_log_write("INFO", "DisableInfoLog configuration: %d (%s)", 
                                      disable_info, disable_info ? "INFO logs disabled" : "INFO logs enabled");
                } else if (strcmp(key, "EnableValidation") == 0) {
                    int enable_validation = 0;
                    if (strcmp(value, "1") == 0 || strcmp(value, "true") == 0 || 
                        strcmp(value, "True") == 0 || strcmp(value, "TRUE") == 0 ||
                        strcmp(value, "yes") == 0 || strcmp(value, "Yes") == 0 || 
                        strcmp(value, "YES") == 0 ||
                        strcmp(value, "on") == 0 || strcmp(value, "On") == 0 || 
                        strcmp(value, "ON") == 0) {
                        enable_validation = 1;
                    } else if (strcmp(value, "0") == 0 || strcmp(value, "false") == 0 || 
                               strcmp(value, "False") == 0 || strcmp(value, "FALSE") == 0 ||
                               strcmp(value, "no") == 0 || strcmp(value, "No") == 0 || 
                               strcmp(value, "NO") == 0 ||
                               strcmp(value, "off") == 0 || strcmp(value, "Off") == 0 || 
                               strcmp(value, "OFF") == 0) {
                        enable_validation = 0;
                    } else {
                        /* 尝试解析为整数 / Try to parse as integer / Als Ganzzahl parsen versuchen */
                        char* endptr = NULL;
                        long int_val = strtol(value, &endptr, 10);
                        if (endptr != NULL && *endptr == '\0') {
                            enable_validation = (int_val != 0) ? 1 : 0;
                        } else {
                            internal_log_write("WARNING", "parse_entry_plugin_config: invalid EnableValidation value '%s', using default (0=disable)", value);
                            enable_validation = 0;
                        }
                    }
                    ctx->enable_validation = enable_validation;
                    /* 使用INFO级别输出配置确认信息 / Use INFO level to output configuration confirmation info / INFO-Ebene verwenden, um Konfigurationsbestätigungsinformationen auszugeben */
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
    long file_pos = ftell(fp);
    char* scan_buffer = (char*)malloc(4096);
    if (scan_buffer != NULL) {
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
                        int disable_info = 0;
                        if (strcmp(value, "1") == 0 || strcmp(value, "true") == 0 || 
                            strcmp(value, "True") == 0 || strcmp(value, "TRUE") == 0 ||
                            strcmp(value, "yes") == 0 || strcmp(value, "Yes") == 0 || 
                            strcmp(value, "YES") == 0 ||
                            strcmp(value, "on") == 0 || strcmp(value, "On") == 0 || 
                            strcmp(value, "ON") == 0) {
                            disable_info = 1;
                        } else if (strcmp(value, "0") == 0 || strcmp(value, "false") == 0 || 
                                   strcmp(value, "False") == 0 || strcmp(value, "FALSE") == 0 ||
                                   strcmp(value, "no") == 0 || strcmp(value, "No") == 0 || 
                                   strcmp(value, "NO") == 0 ||
                                   strcmp(value, "off") == 0 || strcmp(value, "Off") == 0 || 
                                   strcmp(value, "OFF") == 0) {
                            disable_info = 0;
                        } else {
                            char* endptr = NULL;
                            long int_val = strtol(value, &endptr, 10);
                            if (endptr != NULL && *endptr == '\0') {
                                disable_info = (int_val != 0) ? 1 : 0;
                            }
                        }
                        ctx->disable_info_log = disable_info;
                    } else if (strcmp(key, "EnableValidation") == 0) {
                        int enable_validation = 0;
                        if (strcmp(value, "1") == 0 || strcmp(value, "true") == 0 || 
                            strcmp(value, "True") == 0 || strcmp(value, "TRUE") == 0 ||
                            strcmp(value, "yes") == 0 || strcmp(value, "Yes") == 0 || 
                            strcmp(value, "YES") == 0 ||
                            strcmp(value, "on") == 0 || strcmp(value, "On") == 0 || 
                            strcmp(value, "ON") == 0) {
                            enable_validation = 1;
                        } else if (strcmp(value, "0") == 0 || strcmp(value, "false") == 0 || 
                                   strcmp(value, "False") == 0 || strcmp(value, "FALSE") == 0 ||
                                   strcmp(value, "no") == 0 || strcmp(value, "No") == 0 || 
                                   strcmp(value, "NO") == 0 ||
                                   strcmp(value, "off") == 0 || strcmp(value, "Off") == 0 || 
                                   strcmp(value, "OFF") == 0) {
                            enable_validation = 0;
                        } else {
                            char* endptr = NULL;
                            long int_val = strtol(value, &endptr, 10);
                            if (endptr != NULL && *endptr == '\0') {
                                enable_validation = (int_val != 0) ? 1 : 0;
                            }
                        }
                        ctx->enable_validation = enable_validation;
                    }
                }
            }
        }
        free(scan_buffer);
        rewind(fp); /* 重置文件指针到开头 / Reset file pointer to beginning / Dateizeiger auf Anfang zurücksetzen */
    }
    
    internal_log_write("INFO", "Opening transfer rules file: %s", config_path);
    
    char* line_buffer = NULL;
    size_t line_buffer_size = 4096;
    char current_section[512] = {0};
    int current_rule_index = -1;
    int in_entry_section = 0;  /* 标记是否在EntryPlugin段中 / Flag indicating if in EntryPlugin section / Flagge, die angibt, ob im EntryPlugin-Abschnitt */
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
                    int disable_info = 0;
                    if (strcmp(value, "1") == 0 || strcmp(value, "true") == 0 || 
                        strcmp(value, "True") == 0 || strcmp(value, "TRUE") == 0 ||
                        strcmp(value, "yes") == 0 || strcmp(value, "Yes") == 0 || 
                        strcmp(value, "YES") == 0 ||
                        strcmp(value, "on") == 0 || strcmp(value, "On") == 0 || 
                        strcmp(value, "ON") == 0) {
                        disable_info = 1;
                    } else if (strcmp(value, "0") == 0 || strcmp(value, "false") == 0 || 
                               strcmp(value, "False") == 0 || strcmp(value, "FALSE") == 0 ||
                               strcmp(value, "no") == 0 || strcmp(value, "No") == 0 || 
                               strcmp(value, "NO") == 0 ||
                               strcmp(value, "off") == 0 || strcmp(value, "Off") == 0 || 
                               strcmp(value, "OFF") == 0) {
                        disable_info = 0;
                    } else {
                        char* endptr = NULL;
                        long int_val = strtol(value, &endptr, 10);
                        if (endptr != NULL && *endptr == '\0') {
                            disable_info = (int_val != 0) ? 1 : 0;
                        } else {
                            internal_log_write("WARNING", "load_transfer_rules: invalid DisableInfoLog value '%s', using default (0=enable)", value);
                            disable_info = 0;
                        }
                    }
                    ctx->disable_info_log = disable_info;
                    /* 使用INFO级别输出配置确认信息 / Use INFO level to output configuration confirmation info / INFO-Ebene verwenden, um Konfigurationsbestätigungsinformationen auszugeben */
                    internal_log_write("INFO", "DisableInfoLog configuration: %d (%s)", 
                                      disable_info, disable_info ? "INFO logs disabled" : "INFO logs enabled");
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
                } else if (strcmp(key, "CacheSelf") == 0) {
                    rule->cache_self = (strcmp(value, "true") == 0 || strcmp(value, "1") == 0) ? 1 : 0;
                } else if (strcmp(key, "SetGroup") == 0) {
                    rule->set_group = allocate_string(value);
                    internal_log_write("INFO", "Parsed SetGroup=%s for rule index %d (temp_rules_count=%zu)", 
                        value, current_rule_index, temp_rules_count);
                }
            }
        }
    }
    
    free(line_buffer);
    fclose(fp);
    
    /* 验证段索引一致性：检查解析的规则数量是否与索引范围一致 / Validate section index consistency: check if parsed rule count matches index range / Abschnittsindex-Konsistenz validieren: Prüfen, ob die Anzahl der analysierten Regeln mit dem Indexbereich übereinstimmt */
    if (max_seen_index >= 0 && (size_t)(max_seen_index + 1) != temp_rules_count) {
        internal_log_write("WARNING", "load_transfer_rules: section index inconsistency detected - max index=%d, but only %zu rules parsed. Expected %zu rules for indices 0-%d", 
            max_seen_index, temp_rules_count, (size_t)(max_seen_index + 1), max_seen_index);
    }
    
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
        return CONFIG_ERR_OVERFLOW;
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
                return CONFIG_ERR_MEMORY;
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
            if (src_rule->set_group != NULL) {
                dst_rule->set_group = allocate_string(src_rule->set_group);
            }
            dst_rule->cache_self = src_rule->cache_self;
        }
    }
    
    for (size_t i = 0; i < temp_rules_count; i++) {
        free_single_rule(&temp_rules[i]);
    }
    free(temp_rules);
    
    ctx->rule_count = new_rule_count;
    
    /* 构建规则索引 / Build rule index / Regelindex erstellen */
    if (build_rule_index() != 0) {
        internal_log_write("WARNING", "Failed to build rule index, falling back to linear search");
    }
    
    /* 构建规则缓存 / Build rule cache / Regel-Cache erstellen */
    build_rule_cache();
    
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
    return CONFIG_ERR_SUCCESS;
}

