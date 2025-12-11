/**
 * @file pointer_transfer_context.c
 * @brief 指针传递插件上下文操作 / Pointer Transfer Plugin Context Operations / Zeigerübertragungs-Plugin-Kontextoperationen
 */

#include "pointer_transfer_context.h"
#include "pointer_transfer_utils.h"
#include "pointer_transfer_platform.h"
#include "pointer_transfer_types.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

/* 哈希表常量 / Hash table constants / Hash-Tabellen-Konstanten */
#define HASH_TABLE_INITIAL_SIZE 16
#define HASH_TABLE_LOAD_FACTOR 0.75

/* 前向声明 / Forward declarations / Vorwärtsdeklarationen */
static void free_hash_table(rule_hash_table_t* hash_table);
static uint64_t hash_string(const char* str);
static uint64_t calculate_rule_hash_key(const char* source_plugin, const char* source_interface, int source_param_index);

/* 全局上下文变量 / Global context variable / Globale Kontextvariable */
static pointer_transfer_context_t g_context = {
    NULL, NXLD_PARAM_TYPE_UNKNOWN, NULL, 0,
    NULL, 0, 0, {NULL, 0, 0}, NULL, 0, 0,
    NULL, 0, 0, NULL, 0, 0,
    NULL, NULL, 0, 0,
    NULL, 0, 0,
    NULL, NULL, NULL, NULL
};

/**
 * @brief 获取全局上下文指针 / Get global context pointer / Globalen Kontextzeiger abrufen
 * @return 全局上下文指针 / Global context pointer / Globaler Kontextzeiger
 */
pointer_transfer_context_t* get_global_context(void) {
    return &g_context;
}

/**
 * @brief 扩展规则数组容量 / Expand rules array capacity / Regel-Array-Kapazität erweitern
 * @return 成功返回0，失败返回非0 / Returns 0 on success, non-zero on failure / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
 */
int expand_rules_capacity(void) {
    pointer_transfer_context_t* ctx = get_global_context();
    size_t new_capacity = ctx->rule_capacity == 0 ? INITIAL_RULE_CAPACITY : ctx->rule_capacity * CAPACITY_GROWTH_FACTOR;
    pointer_transfer_rule_t* new_rules = (pointer_transfer_rule_t*)realloc(ctx->rules, new_capacity * sizeof(pointer_transfer_rule_t));
    if (new_rules == NULL) {
        return -1;
    }
    memset(new_rules + ctx->rule_count, 0, (new_capacity - ctx->rule_count) * sizeof(pointer_transfer_rule_t));
    ctx->rules = new_rules;
    ctx->rule_capacity = new_capacity;
    return 0;
}

/**
 * @brief 扩展已加载插件数组容量 / Expand loaded plugins array capacity / Geladenes Plugin-Array-Kapazität erweitern
 * @return 成功返回0，失败返回非0 / Returns 0 on success, non-zero on failure / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
 */
int expand_loaded_plugins_capacity(void) {
    pointer_transfer_context_t* ctx = get_global_context();
    size_t new_capacity = ctx->loaded_plugin_capacity == 0 ? INITIAL_PLUGIN_CAPACITY : ctx->loaded_plugin_capacity * CAPACITY_GROWTH_FACTOR;
    loaded_plugin_info_t* new_plugins = (loaded_plugin_info_t*)realloc(ctx->loaded_plugins, new_capacity * sizeof(loaded_plugin_info_t));
    if (new_plugins == NULL) {
        return -1;
    }
    memset(new_plugins + ctx->loaded_plugin_count, 0, (new_capacity - ctx->loaded_plugin_count) * sizeof(loaded_plugin_info_t));
    ctx->loaded_plugins = new_plugins;
    ctx->loaded_plugin_capacity = new_capacity;
    return 0;
}

/**
 * @brief 扩展接口状态数组容量 / Expand interface states array capacity / Schnittstellen-Status-Array-Kapazität erweitern
 * @return 成功返回0，失败返回非0 / Returns 0 on success, non-zero on failure / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
 */
int expand_interface_states_capacity(void) {
    pointer_transfer_context_t* ctx = get_global_context();
    size_t new_capacity = ctx->interface_state_capacity == 0 ? INITIAL_INTERFACE_STATE_CAPACITY : ctx->interface_state_capacity * CAPACITY_GROWTH_FACTOR;
    target_interface_state_t* new_states = (target_interface_state_t*)realloc(ctx->interface_states, new_capacity * sizeof(target_interface_state_t));
    if (new_states == NULL) {
        return -1;
    }
    memset(new_states + ctx->interface_state_count, 0, (new_capacity - ctx->interface_state_count) * sizeof(target_interface_state_t));
    ctx->interface_states = new_states;
    ctx->interface_state_capacity = new_capacity;
    return 0;
}

/**
 * @brief 释放传递规则内存 / Free transfer rules memory / Übertragungsregel-Speicher freigeben
 */
void free_transfer_rules(void) {
    pointer_transfer_context_t* ctx = get_global_context();
    if (ctx->rules == NULL) {
        return;
    }
    
    for (size_t i = 0; i < ctx->rule_count; i++) {
        free_single_rule(&ctx->rules[i]);
    }
    
    free(ctx->rules);
    ctx->rules = NULL;
    ctx->rule_count = 0;
    ctx->rule_capacity = 0;
    
    /* 释放哈希表 / Free hash table / Hash-Tabelle freigeben */
    free_hash_table(&ctx->rule_hash_table);
    
    if (ctx->path_cache != NULL) {
        for (size_t i = 0; i < ctx->path_cache_count; i++) {
            if (ctx->path_cache[i].plugin_name != NULL) {
                free(ctx->path_cache[i].plugin_name);
            }
            if (ctx->path_cache[i].plugin_path != NULL) {
                free(ctx->path_cache[i].plugin_path);
            }
        }
        free(ctx->path_cache);
        ctx->path_cache = NULL;
        ctx->path_cache_count = 0;
        ctx->path_cache_capacity = 0;
    }
}

/**
 * @brief 初始化上下文 / Initialize context / Kontext initialisieren
 */
void init_context(void) {
    pointer_transfer_context_t* ctx = get_global_context();
    memset(ctx, 0, sizeof(pointer_transfer_context_t));
    ctx->stored_type = NXLD_PARAM_TYPE_UNKNOWN;
}

/**
 * @brief 字符串哈希函数（FNV-1a算法）/ String hash function (FNV-1a algorithm) / Zeichenfolgen-Hash-Funktion (FNV-1a-Algorithmus)
 */
static uint64_t hash_string(const char* str) {
    if (str == NULL) {
        return 0;
    }
    
    uint64_t hash = 14695981039346656037ULL; /* FNV offset basis */
    const char* p = str;
    
    while (*p != '\0') {
        hash ^= (uint64_t)(unsigned char)(*p);
        hash *= 1099511628211ULL; /* FNV prime */
        p++;
    }
    
    return hash;
}

/**
 * @brief 计算规则哈希键 / Calculate rule hash key / Regel-Hash-Schlüssel berechnen
 */
static uint64_t calculate_rule_hash_key(const char* source_plugin, const char* source_interface, int source_param_index) {
    char key_buffer[512];
    int written = snprintf(key_buffer, sizeof(key_buffer), "%s.%s.%d", 
                          source_plugin != NULL ? source_plugin : "",
                          source_interface != NULL ? source_interface : "",
                          source_param_index);
    if (written < 0 || (size_t)written >= sizeof(key_buffer)) {
        return 0;
    }
    return hash_string(key_buffer);
}

/**
 * @brief 释放哈希表 / Free hash table / Hash-Tabelle freigeben
 */
static void free_hash_table(rule_hash_table_t* hash_table) {
    if (hash_table == NULL || hash_table->buckets == NULL) {
        return;
    }
    
    for (size_t i = 0; i < hash_table->bucket_count; i++) {
        rule_hash_node_t* node = hash_table->buckets[i];
        while (node != NULL) {
            rule_hash_node_t* next = node->next;
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
 * @brief 扩展哈希表 / Expand hash table / Hash-Tabelle erweitern
 */
static int expand_hash_table(rule_hash_table_t* hash_table) {
    if (hash_table == NULL) {
        return -1;
    }
    
    size_t old_bucket_count = hash_table->bucket_count;
    rule_hash_node_t** old_buckets = hash_table->buckets;
    
    size_t new_bucket_count = old_bucket_count == 0 ? HASH_TABLE_INITIAL_SIZE : old_bucket_count * 2;
    rule_hash_node_t** new_buckets = (rule_hash_node_t**)calloc(new_bucket_count, sizeof(rule_hash_node_t*));
    if (new_buckets == NULL) {
        return -1;
    }
    
    /* 重新哈希所有条目 / Rehash all entries / Alle Einträge neu hashen */
    for (size_t i = 0; i < old_bucket_count; i++) {
        rule_hash_node_t* node = old_buckets[i];
        while (node != NULL) {
            rule_hash_node_t* next = node->next;
            size_t new_bucket = node->hash_key % new_bucket_count;
            node->next = new_buckets[new_bucket];
            new_buckets[new_bucket] = node;
            node = next;
        }
    }
    
    free(old_buckets);
    hash_table->buckets = new_buckets;
    hash_table->bucket_count = new_bucket_count;
    
    return 0;
}

/**
 * @brief 向哈希表插入规则 / Insert rule into hash table / Regel in Hash-Tabelle einfügen
 */
static int insert_rule_into_hash_table(rule_hash_table_t* hash_table, uint64_t hash_key, size_t rule_index) {
    if (hash_table == NULL) {
        return -1;
    }
    
    /* 检查是否需要扩展 / Check if expansion needed / Prüfen, ob Erweiterung erforderlich */
    if (hash_table->bucket_count == 0 || 
        (hash_table->entry_count > 0 && 
         (double)hash_table->entry_count / hash_table->bucket_count > HASH_TABLE_LOAD_FACTOR)) {
        if (expand_hash_table(hash_table) != 0) {
            return -1;
        }
    }
    
    size_t bucket = hash_key % hash_table->bucket_count;
    
    /* 创建新节点 / Create new node / Neuen Knoten erstellen */
    rule_hash_node_t* new_node = (rule_hash_node_t*)malloc(sizeof(rule_hash_node_t));
    if (new_node == NULL) {
        return -1;
    }
    
    new_node->hash_key = hash_key;
    new_node->rule_index = rule_index;
    new_node->next = hash_table->buckets[bucket];
    hash_table->buckets[bucket] = new_node;
    hash_table->entry_count++;
    
    return 0;
}

/**
 * @brief 构建规则索引（哈希表）/ Build rule index (hash table) / Regelindex erstellen (Hash-Tabelle)
 * @return 成功返回0，失败返回非0 / Returns 0 on success, non-zero on failure / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
 */
int build_rule_index(void) {
    pointer_transfer_context_t* ctx = get_global_context();
    
    /* 释放旧哈希表 / Free old hash table / Alte Hash-Tabelle freigeben */
    free_hash_table(&ctx->rule_hash_table);
    
    if (ctx->rule_count == 0 || ctx->rules == NULL) {
        return 0;
    }
    
    /* 初始化哈希表 / Initialize hash table / Hash-Tabelle initialisieren */
    ctx->rule_hash_table.bucket_count = HASH_TABLE_INITIAL_SIZE;
    ctx->rule_hash_table.entry_count = 0;
    ctx->rule_hash_table.buckets = (rule_hash_node_t**)calloc(ctx->rule_hash_table.bucket_count, sizeof(rule_hash_node_t*));
    if (ctx->rule_hash_table.buckets == NULL) {
        return -1;
    }
    
    /* 构建索引项 / Build index entries / Indexeinträge erstellen */
    for (size_t i = 0; i < ctx->rule_count; i++) {
        pointer_transfer_rule_t* rule = &ctx->rules[i];
        if (!rule->enabled || rule->source_plugin == NULL || rule->source_interface == NULL) {
            continue;
        }
        
        uint64_t hash_key = calculate_rule_hash_key(rule->source_plugin, rule->source_interface, rule->source_param_index);
        if (hash_key != 0) {
            if (insert_rule_into_hash_table(&ctx->rule_hash_table, hash_key, i) != 0) {
                free_hash_table(&ctx->rule_hash_table);
                return -1;
            }
        }
    }
    
    internal_log_write("INFO", "Built rule hash table with %zu entries in %zu buckets", 
                      ctx->rule_hash_table.entry_count, ctx->rule_hash_table.bucket_count);
    return 0;
}

/**
 * @brief 查找规则索引范围（哈希表）/ Find rule index range (hash table) / Regelindex-Bereich suchen (Hash-Tabelle)
 * @param source_plugin 源插件名称 / Source plugin name / Quell-Plugin-Name
 * @param source_interface 源接口名称 / Source interface name / Quell-Schnittstellenname
 * @param source_param_index 源参数索引 / Source parameter index / Quell-Parameterindex
 * @param start_index 输出起始索引指针 / Output start index pointer / Ausgabe-Startindex-Zeiger
 * @param end_index 输出结束索引指针 / Output end index pointer / Ausgabe-Endindex-Zeiger
 * @return 找到返回1，未找到返回0 / Returns 1 if found, 0 if not found / Gibt 1 zurück wenn gefunden, 0 wenn nicht gefunden
 */
int find_rule_index_range(const char* source_plugin, const char* source_interface, int source_param_index, size_t* start_index, size_t* end_index) {
    if (source_plugin == NULL || source_interface == NULL || start_index == NULL || end_index == NULL) {
        return 0;
    }
    
    pointer_transfer_context_t* ctx = get_global_context();
    if (ctx->rule_hash_table.buckets == NULL || ctx->rule_hash_table.entry_count == 0) {
        return 0;
    }
    
    /* 计算哈希键 / Calculate hash key / Hash-Schlüssel berechnen */
    uint64_t hash_key = calculate_rule_hash_key(source_plugin, source_interface, source_param_index);
    if (hash_key == 0) {
        return 0;
    }
    
    size_t bucket = hash_key % ctx->rule_hash_table.bucket_count;
    rule_hash_node_t* node = ctx->rule_hash_table.buckets[bucket];
    
    /* 查找匹配的规则索引 / Find matching rule indices / Passende Regelindizes suchen */
    size_t min_index = SIZE_MAX;
    size_t max_index = 0;
    int found = 0;
    
    while (node != NULL) {
        if (node->hash_key == hash_key) {
            pointer_transfer_rule_t* rule = &ctx->rules[node->rule_index];
            if (rule->enabled && 
                rule->source_plugin != NULL && strcmp(rule->source_plugin, source_plugin) == 0 &&
                rule->source_interface != NULL && strcmp(rule->source_interface, source_interface) == 0 &&
                rule->source_param_index == source_param_index) {
                if (node->rule_index < min_index) {
                    min_index = node->rule_index;
                }
                if (node->rule_index > max_index) {
                    max_index = node->rule_index;
                }
                found = 1;
            }
        }
        node = node->next;
    }
    
    if (found) {
        *start_index = min_index;
        *end_index = max_index;
        return 1;
    }
    
    return 0;
}

/**
 * @brief 构建规则缓存 / Build rule cache / Regel-Cache erstellen
 * @return 成功返回0，失败返回非0 / Returns 0 on success, non-zero on failure / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
 */
int build_rule_cache(void) {
    pointer_transfer_context_t* ctx = get_global_context();
    
    /* 释放旧缓存 / Free old cache / Alten Cache freigeben */
    if (ctx->cached_rule_indices != NULL) {
        free(ctx->cached_rule_indices);
        ctx->cached_rule_indices = NULL;
        ctx->cached_rule_count = 0;
        ctx->cached_rule_capacity = 0;
    }
    
    if (ctx->rule_count == 0 || ctx->rules == NULL) {
        return 0;
    }
    
    /* 统计需要缓存的规则数量 / Count rules that need caching / Regeln zählen, die gecacht werden müssen */
    size_t cache_count = 0;
    for (size_t i = 0; i < ctx->rule_count; i++) {
        if (ctx->rules[i].cache_self && ctx->rules[i].enabled) {
            cache_count++;
        }
    }
    
    if (cache_count == 0) {
        return 0;
    }
    
    /* 分配缓存数组 / Allocate cache array / Cache-Array zuweisen */
    ctx->cached_rule_indices = (size_t*)malloc(cache_count * sizeof(size_t));
    if (ctx->cached_rule_indices == NULL) {
        return -1;
    }
    
    /* 填充缓存数组 / Fill cache array / Cache-Array füllen */
    size_t cache_index = 0;
    for (size_t i = 0; i < ctx->rule_count; i++) {
        if (ctx->rules[i].cache_self && ctx->rules[i].enabled) {
            ctx->cached_rule_indices[cache_index++] = i;
        }
    }
    
    ctx->cached_rule_count = cache_count;
    ctx->cached_rule_capacity = cache_count;
    
    internal_log_write("INFO", "Built rule cache with %zu cached rules", cache_count);
    return 0;
}

/**
 * @brief 获取缓存的规则数量 / Get cached rule count / Anzahl gecachter Regeln abrufen
 * @return 缓存的规则数量 / Cached rule count / Anzahl gecachter Regeln
 */
size_t get_cached_rule_count(void) {
    pointer_transfer_context_t* ctx = get_global_context();
    return ctx->cached_rule_count;
}

/**
 * @brief 获取缓存的规则索引数组 / Get cached rule indices array / Gecachte Regelindex-Array abrufen
 * @return 缓存的规则索引数组指针 / Cached rule indices array pointer / Zeiger auf gecachte Regelindex-Array
 */
const size_t* get_cached_rule_indices(void) {
    pointer_transfer_context_t* ctx = get_global_context();
    return ctx->cached_rule_indices;
}

/**
 * @brief 清理上下文 / Cleanup context / Kontext bereinigen
 */
void cleanup_context(void) {
    pointer_transfer_context_t* ctx = get_global_context();
    
    if (ctx->stored_type_name != NULL) {
        free(ctx->stored_type_name);
        ctx->stored_type_name = NULL;
    }
    if (ctx->plugin_dll_path != NULL) {
        free(ctx->plugin_dll_path);
        ctx->plugin_dll_path = NULL;
    }
    if (ctx->interface_states != NULL) {
        for (size_t i = 0; i < ctx->interface_state_count; i++) {
            target_interface_state_t* state = &ctx->interface_states[i];
            if (state->plugin_name != NULL) {
                free(state->plugin_name);
            }
            if (state->interface_name != NULL) {
                free(state->interface_name);
            }
            if (state->param_ready != NULL) {
                free(state->param_ready);
            }
            if (state->param_values != NULL) {
                free(state->param_values);
            }
            if (state->param_types != NULL) {
                free(state->param_types);
            }
            if (state->param_sizes != NULL) {
                free(state->param_sizes);
            }
            if (state->param_int_values != NULL) {
                free(state->param_int_values);
            }
            if (state->param_float_values != NULL) {
                free(state->param_float_values);
            }
        }
        free(ctx->interface_states);
        ctx->interface_states = NULL;
    }
    if (ctx->loaded_plugins != NULL) {
        for (size_t i = 0; i < ctx->loaded_plugin_count; i++) {
            if (ctx->loaded_plugins[i].plugin_name != NULL) {
                free(ctx->loaded_plugins[i].plugin_name);
            }
            if (ctx->loaded_plugins[i].plugin_path != NULL) {
                free(ctx->loaded_plugins[i].plugin_path);
            }
            if (ctx->loaded_plugins[i].handle != NULL) {
                pt_platform_close_library(ctx->loaded_plugins[i].handle);
            }
        }
        free(ctx->loaded_plugins);
        ctx->loaded_plugins = NULL;
    }
    free_transfer_rules();
    free_hash_table(&ctx->rule_hash_table);
    if (ctx->cached_rule_indices != NULL) {
        free(ctx->cached_rule_indices);
        ctx->cached_rule_indices = NULL;
        ctx->cached_rule_count = 0;
        ctx->cached_rule_capacity = 0;
    }
    if (ctx->entry_plugin_name != NULL) {
        free(ctx->entry_plugin_name);
        ctx->entry_plugin_name = NULL;
    }
    if (ctx->entry_plugin_path != NULL) {
        free(ctx->entry_plugin_path);
        ctx->entry_plugin_path = NULL;
    }
    if (ctx->entry_nxpt_path != NULL) {
        free(ctx->entry_nxpt_path);
        ctx->entry_nxpt_path = NULL;
    }
    if (ctx->entry_auto_run_interface != NULL) {
        free(ctx->entry_auto_run_interface);
        ctx->entry_auto_run_interface = NULL;
    }
    if (ctx->loaded_nxpt_files != NULL) {
        for (size_t i = 0; i < ctx->loaded_nxpt_count; i++) {
            if (ctx->loaded_nxpt_files[i].plugin_name != NULL) {
                free(ctx->loaded_nxpt_files[i].plugin_name);
            }
            if (ctx->loaded_nxpt_files[i].nxpt_path != NULL) {
                free(ctx->loaded_nxpt_files[i].nxpt_path);
            }
        }
        free(ctx->loaded_nxpt_files);
        ctx->loaded_nxpt_files = NULL;
    }
    memset(ctx, 0, sizeof(pointer_transfer_context_t));
}

