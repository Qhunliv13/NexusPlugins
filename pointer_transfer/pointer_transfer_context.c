/**
 * @file pointer_transfer_context.c
 * @brief 指针传递插件上下文操作 / Pointer Transfer Plugin Context Operations / Zeigerübertragungs-Plugin-Kontextoperationen
 */

#include "pointer_transfer_context.h"
#include "pointer_transfer_utils.h"
#include "pointer_transfer_platform.h"
#include "pointer_transfer_types.h"
#include "pointer_transfer_config.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#ifndef _WIN32
#include <strings.h>  /* 用于Linux下的strcasecmp函数 / for strcasecmp on Linux / für strcasecmp unter Linux */
#endif

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
    NULL, NULL, NULL, NULL,
    0,  /* disable_info_log = 0 (默认启用INFO级别日志) / disable_info_log = 0 (INFO level logging enabled by default) / disable_info_log = 0 (INFO-Level-Protokollierung standardmäßig aktiviert) */
    0,  /* enable_validation = 0 (默认禁用验证) / enable_validation = 0 (validation disabled by default) / enable_validation = 0 (Validierung standardmäßig deaktiviert) */
    NULL, 0, 0  /* ignore_plugins = NULL, ignore_plugin_count = 0, ignore_plugin_capacity = 0 (默认无忽略插件) / ignore_plugins = NULL, ignore_plugin_count = 0, ignore_plugin_capacity = 0 (no ignored plugins by default) / ignore_plugins = NULL, ignore_plugin_count = 0, ignore_plugin_capacity = 0 (standardmäßig keine ignorierten Plugins) */
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
    if (ctx == NULL) {
        internal_log_write("ERROR", "expand_rules_capacity: global context is NULL");
        return -1;
    }
    
    size_t new_capacity = ctx->rule_capacity == 0 ? INITIAL_RULE_CAPACITY : ctx->rule_capacity * CAPACITY_GROWTH_FACTOR;
    
    /* 检查容量溢出 / Check capacity overflow / Kapazitätsüberlauf prüfen */
    if (new_capacity < ctx->rule_capacity || new_capacity > SIZE_MAX / sizeof(pointer_transfer_rule_t)) {
        internal_log_write("ERROR", "expand_rules_capacity: capacity overflow detected (current=%zu, new=%zu)", 
                          ctx->rule_capacity, new_capacity);
        return -1;
    }
    
    pointer_transfer_rule_t* new_rules = (pointer_transfer_rule_t*)realloc(ctx->rules, new_capacity * sizeof(pointer_transfer_rule_t));
    if (new_rules == NULL) {
        internal_log_write("ERROR", "expand_rules_capacity: failed to allocate memory for rules array (new_capacity=%zu)", new_capacity);
        return -1;
    }
    
    memset(new_rules + ctx->rule_count, 0, (new_capacity - ctx->rule_count) * sizeof(pointer_transfer_rule_t));
    size_t old_capacity = ctx->rule_capacity;
    ctx->rules = new_rules;
    ctx->rule_capacity = new_capacity;
    
    internal_log_write("INFO", "expand_rules_capacity: expanded to %zu (was %zu)", new_capacity, old_capacity);
    return 0;
}

/**
 * @brief 扩展已加载插件数组容量 / Expand loaded plugins array capacity / Geladenes Plugin-Array-Kapazität erweitern
 * @return 成功返回0，失败返回非0 / Returns 0 on success, non-zero on failure / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
 */
int expand_loaded_plugins_capacity(void) {
    pointer_transfer_context_t* ctx = get_global_context();
    if (ctx == NULL) {
        internal_log_write("ERROR", "expand_loaded_plugins_capacity: global context is NULL");
        return -1;
    }
    
    size_t new_capacity = ctx->loaded_plugin_capacity == 0 ? INITIAL_PLUGIN_CAPACITY : ctx->loaded_plugin_capacity * CAPACITY_GROWTH_FACTOR;
    
    /* 检查容量溢出 / Check capacity overflow / Kapazitätsüberlauf prüfen */
    if (new_capacity < ctx->loaded_plugin_capacity || new_capacity > SIZE_MAX / sizeof(loaded_plugin_info_t)) {
        internal_log_write("ERROR", "expand_loaded_plugins_capacity: capacity overflow detected (current=%zu, new=%zu)", 
                          ctx->loaded_plugin_capacity, new_capacity);
        return -1;
    }
    
    loaded_plugin_info_t* new_plugins = (loaded_plugin_info_t*)realloc(ctx->loaded_plugins, new_capacity * sizeof(loaded_plugin_info_t));
    if (new_plugins == NULL) {
        internal_log_write("ERROR", "expand_loaded_plugins_capacity: failed to allocate memory for plugins array (new_capacity=%zu)", new_capacity);
        return -1;
    }
    
    memset(new_plugins + ctx->loaded_plugin_count, 0, (new_capacity - ctx->loaded_plugin_count) * sizeof(loaded_plugin_info_t));
    size_t old_capacity = ctx->loaded_plugin_capacity;
    ctx->loaded_plugins = new_plugins;
    ctx->loaded_plugin_capacity = new_capacity;
    
    internal_log_write("INFO", "expand_loaded_plugins_capacity: expanded to %zu (was %zu)", new_capacity, old_capacity);
    return 0;
}

/**
 * @brief 扩展接口状态数组容量 / Expand interface states array capacity / Schnittstellen-Status-Array-Kapazität erweitern
 * @return 成功返回0，失败返回非0 / Returns 0 on success, non-zero on failure / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
 */
int expand_interface_states_capacity(void) {
    pointer_transfer_context_t* ctx = get_global_context();
    if (ctx == NULL) {
        internal_log_write("ERROR", "expand_interface_states_capacity: global context is NULL");
        return -1;
    }
    
    size_t new_capacity = ctx->interface_state_capacity == 0 ? INITIAL_INTERFACE_STATE_CAPACITY : ctx->interface_state_capacity * CAPACITY_GROWTH_FACTOR;
    
    /* 检查容量溢出 / Check capacity overflow / Kapazitätsüberlauf prüfen */
    if (new_capacity < ctx->interface_state_capacity || new_capacity > SIZE_MAX / sizeof(target_interface_state_t)) {
        internal_log_write("ERROR", "expand_interface_states_capacity: capacity overflow detected (current=%zu, new=%zu)", 
                          ctx->interface_state_capacity, new_capacity);
        return -1;
    }
    
    target_interface_state_t* new_states = (target_interface_state_t*)realloc(ctx->interface_states, new_capacity * sizeof(target_interface_state_t));
    if (new_states == NULL) {
        internal_log_write("ERROR", "expand_interface_states_capacity: failed to allocate memory for interface states array (new_capacity=%zu)", new_capacity);
        return -1;
    }
    
    memset(new_states + ctx->interface_state_count, 0, (new_capacity - ctx->interface_state_count) * sizeof(target_interface_state_t));
    size_t old_capacity = ctx->interface_state_capacity;
    ctx->interface_states = new_states;
    ctx->interface_state_capacity = new_capacity;
    
    internal_log_write("INFO", "expand_interface_states_capacity: expanded to %zu (was %zu)", new_capacity, old_capacity);
    return 0;
}

/**
 * @brief 释放传递规则内存 / Free transfer rules memory / Übertragungsregel-Speicher freigeben
 */
void free_transfer_rules(void) {
    pointer_transfer_context_t* ctx = get_global_context();
    if (ctx == NULL) {
        return;
    }
    
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
                ctx->path_cache[i].plugin_name = NULL;
            }
            if (ctx->path_cache[i].plugin_path != NULL) {
                free(ctx->path_cache[i].plugin_path);
                ctx->path_cache[i].plugin_path = NULL;
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
    if (ctx == NULL) {
        return;
    }
    
    /* 清理旧数据 / Cleanup old data / Alte Daten bereinigen */
    if (ctx->rules != NULL || ctx->loaded_plugins != NULL || ctx->interface_states != NULL ||
        ctx->stored_type_name != NULL || ctx->plugin_dll_path != NULL) {
        cleanup_context();
    }
    
    /* 确保所有字段已初始化 / Ensure all fields are initialized / Sicherstellen, dass alle Felder initialisiert sind */
    ctx->stored_type = NXLD_PARAM_TYPE_UNKNOWN;
    
    /* 初始化哈希表结构 / Initialize hash table structure / Hash-Tabellen-Struktur initialisieren */
    ctx->rule_hash_table.buckets = NULL;
    ctx->rule_hash_table.bucket_count = 0;
    ctx->rule_hash_table.entry_count = 0;
    
    /* 初始化.nxpt哈希表结构 / Initialize .nxpt hash table structure / .nxpt-Hash-Tabellen-Struktur initialisieren */
    ctx->nxpt_hash_table.buckets = NULL;
    ctx->nxpt_hash_table.bucket_count = 0;
    ctx->nxpt_hash_table.entry_count = 0;
    
}

/**
 * @brief 字符串哈希函数（FNV-1a算法）/ String hash function (FNV-1a algorithm) / Zeichenfolgen-Hash-Funktion (FNV-1a-Algorithmus)
 */
static uint64_t hash_string(const char* str) {
    if (str == NULL) {
        return 0;
    }
    
    uint64_t hash = 14695981039346656037ULL; /* FNV偏移基数 / FNV offset basis / FNV-Offset-Basis */
    const char* p = str;
    
    while (*p != '\0') {
        hash ^= (uint64_t)(unsigned char)(*p);
        hash *= 1099511628211ULL; /* FNV质数 / FNV prime / FNV-Primzahl */
        p++;
    }
    
    return hash;
}

/**
 * @brief 计算规则哈希键 / Calculate rule hash key / Regel-Hash-Schlüssel berechnen
 */
static uint64_t calculate_rule_hash_key(const char* source_plugin, const char* source_interface, int source_param_index) {
    if (source_plugin == NULL && source_interface == NULL) {
        return 0;
    }
    
    char key_buffer[512];
    int written = snprintf(key_buffer, sizeof(key_buffer), "%s.%s.%d", 
                          source_plugin != NULL ? source_plugin : "",
                          source_interface != NULL ? source_interface : "",
                          source_param_index);
    if (written < 0 || (size_t)written >= sizeof(key_buffer)) {
        internal_log_write("WARNING", "calculate_rule_hash_key: key buffer overflow (plugin=%s, interface=%s, param=%d)", 
                          source_plugin != NULL ? source_plugin : "NULL",
                          source_interface != NULL ? source_interface : "NULL",
                          source_param_index);
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
        internal_log_write("ERROR", "expand_hash_table: hash_table is NULL");
        return -1;
    }
    
    size_t old_bucket_count = hash_table->bucket_count;
    rule_hash_node_t** old_buckets = hash_table->buckets;
    
    size_t new_bucket_count = old_bucket_count == 0 ? HASH_TABLE_INITIAL_SIZE : old_bucket_count * 2;
    
    /* 检查容量溢出 / Check capacity overflow / Kapazitätsüberlauf prüfen */
    if (new_bucket_count < old_bucket_count || new_bucket_count > SIZE_MAX / sizeof(rule_hash_node_t*)) {
        internal_log_write("ERROR", "expand_hash_table: bucket count overflow detected (old=%zu, new=%zu)", 
                          old_bucket_count, new_bucket_count);
        return -1;
    }
    
    rule_hash_node_t** new_buckets = (rule_hash_node_t**)calloc(new_bucket_count, sizeof(rule_hash_node_t*));
    if (new_buckets == NULL) {
        internal_log_write("ERROR", "expand_hash_table: failed to allocate memory for buckets (new_bucket_count=%zu)", new_bucket_count);
        return -1;
    }
    
    /* 重新哈希所有条目 / Rehash all entries / Alle Einträge neu hashieren */
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
        internal_log_write("ERROR", "insert_rule_into_hash_table: hash_table is NULL");
        return -1;
    }
    
    if (hash_key == 0) {
        internal_log_write("WARNING", "insert_rule_into_hash_table: hash_key is 0 for rule_index=%zu", rule_index);
        return -1;
    }
    
    /* 检查是否需要扩展 / Check if expansion is needed / Prüfen, ob Erweiterung erforderlich ist */
    if (hash_table->bucket_count == 0 || 
        (hash_table->entry_count > 0 && hash_table->bucket_count > 0 &&
         (double)hash_table->entry_count / hash_table->bucket_count > HASH_TABLE_LOAD_FACTOR)) {
        if (expand_hash_table(hash_table) != 0) {
            internal_log_write("ERROR", "insert_rule_into_hash_table: failed to expand hash table");
            return -1;
        }
    }
    
    if (hash_table->bucket_count == 0) {
        internal_log_write("ERROR", "insert_rule_into_hash_table: bucket_count is 0 after expansion");
        return -1;
    }
    
    size_t bucket = hash_key % hash_table->bucket_count;
    
    /* 创建新节点 / Create new node / Neuen Knoten erstellen */
    rule_hash_node_t* new_node = (rule_hash_node_t*)malloc(sizeof(rule_hash_node_t));
    if (new_node == NULL) {
        internal_log_write("ERROR", "insert_rule_into_hash_table: failed to allocate memory for hash node");
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
    if (ctx == NULL) {
        internal_log_write("ERROR", "build_rule_index: global context is NULL");
        return -1;
    }
    
    /* 释放旧哈希表 / Free old hash table / Alte Hash-Tabelle freigeben */
    free_hash_table(&ctx->rule_hash_table);
    
    if (ctx->rule_count == 0 || ctx->rules == NULL) {
        internal_log_write("INFO", "build_rule_index: no rules to index (rule_count=%zu)", ctx->rule_count);
        return 0;
    }
    
    /* 初始化哈希表 / Initialize hash table / Hash-Tabelle initialisieren */
    ctx->rule_hash_table.bucket_count = HASH_TABLE_INITIAL_SIZE;
    ctx->rule_hash_table.entry_count = 0;
    ctx->rule_hash_table.buckets = (rule_hash_node_t**)calloc(ctx->rule_hash_table.bucket_count, sizeof(rule_hash_node_t*));
    if (ctx->rule_hash_table.buckets == NULL) {
        internal_log_write("ERROR", "build_rule_index: failed to allocate memory for hash table buckets");
        return -1;
    }
    
    /* 构建索引项 / Build index entries / Indexeinträge erstellen */
    size_t indexed_count = 0;
    for (size_t i = 0; i < ctx->rule_count; i++) {
        pointer_transfer_rule_t* rule = &ctx->rules[i];
        if (!rule->enabled || rule->source_plugin == NULL || rule->source_interface == NULL) {
            continue;
        }
        
        uint64_t hash_key = calculate_rule_hash_key(rule->source_plugin, rule->source_interface, rule->source_param_index);
        if (hash_key != 0) {
            if (insert_rule_into_hash_table(&ctx->rule_hash_table, hash_key, i) != 0) {
                internal_log_write("ERROR", "build_rule_index: failed to insert rule %zu into hash table", i);
                free_hash_table(&ctx->rule_hash_table);
                return -1;
            }
            indexed_count++;
        } else {
            internal_log_write("WARNING", "build_rule_index: failed to calculate hash key for rule %zu (plugin=%s, interface=%s, param=%d)", 
                             i, 
                             rule->source_plugin != NULL ? rule->source_plugin : "NULL",
                             rule->source_interface != NULL ? rule->source_interface : "NULL",
                             rule->source_param_index);
        }
    }
    
    internal_log_write("INFO", "Built rule hash table with %zu entries in %zu buckets (indexed %zu/%zu rules)", 
                      ctx->rule_hash_table.entry_count, ctx->rule_hash_table.bucket_count, indexed_count, ctx->rule_count);
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
    if (ctx == NULL) {
        return 0;
    }
    
    if (ctx->rule_hash_table.buckets == NULL || ctx->rule_hash_table.entry_count == 0 || ctx->rule_hash_table.bucket_count == 0) {
        return 0;
    }
    
    if (ctx->rules == NULL || ctx->rule_count == 0) {
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
    size_t min_index = 0;
    size_t max_index = 0;
    int found = 0;
    
    while (node != NULL) {
        if (node->hash_key == hash_key) {
            /* 验证规则索引有效性 / Validate rule index validity / Regelindex-Gültigkeit prüfen */
            if (node->rule_index >= ctx->rule_count) {
                internal_log_write("WARNING", "find_rule_index_range: invalid rule_index %zu (rule_count=%zu)", 
                                 node->rule_index, ctx->rule_count);
                node = node->next;
                continue;
            }
            
            pointer_transfer_rule_t* rule = &ctx->rules[node->rule_index];
            if (rule->enabled && 
                rule->source_plugin != NULL && strcmp(rule->source_plugin, source_plugin) == 0 &&
                rule->source_interface != NULL && strcmp(rule->source_interface, source_interface) == 0 &&
                rule->source_param_index == source_param_index) {
                if (!found) {
                    /* 第一个匹配项，初始化最小索引和最大索引 / First match, initialize minimum index and maximum index / Erste Übereinstimmung, Minimalindex und Maximalindex initialisieren */
                    min_index = node->rule_index;
                    max_index = node->rule_index;
                    found = 1;
                } else {
                    if (node->rule_index < min_index) {
                        min_index = node->rule_index;
                    }
                    if (node->rule_index > max_index) {
                        max_index = node->rule_index;
                    }
                }
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
    if (ctx == NULL) {
        internal_log_write("ERROR", "build_rule_cache: global context is NULL");
        return -1;
    }
    
    /* 释放旧缓存 / Free old cache / Alten Cache freigeben */
    if (ctx->cached_rule_indices != NULL) {
        free(ctx->cached_rule_indices);
        ctx->cached_rule_indices = NULL;
        ctx->cached_rule_count = 0;
        ctx->cached_rule_capacity = 0;
    }
    
    if (ctx->rule_count == 0 || ctx->rules == NULL) {
        internal_log_write("INFO", "build_rule_cache: no rules to cache (rule_count=%zu)", ctx->rule_count);
        return 0;
    }
    
    /* 统计需要缓存的规则数量 / Count rules that require caching / Regeln zählen, die gecacht werden müssen */
    size_t cache_count = 0;
    for (size_t i = 0; i < ctx->rule_count; i++) {
        if (ctx->rules[i].cache_self && ctx->rules[i].enabled) {
            cache_count++;
        }
    }
    
    if (cache_count == 0) {
        internal_log_write("INFO", "build_rule_cache: no rules marked for caching");
        return 0;
    }
    
    /* 检查容量溢出 / Check capacity overflow / Kapazitätsüberlauf prüfen */
    if (cache_count > SIZE_MAX / sizeof(size_t)) {
        internal_log_write("ERROR", "build_rule_cache: cache_count overflow detected (cache_count=%zu)", cache_count);
        return -1;
    }
    
    /* 分配缓存数组 / Allocate cache array / Cache-Array zuweisen */
    ctx->cached_rule_indices = (size_t*)malloc(cache_count * sizeof(size_t));
    if (ctx->cached_rule_indices == NULL) {
        internal_log_write("ERROR", "build_rule_cache: failed to allocate memory for cache array (cache_count=%zu)", cache_count);
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
    if (ctx == NULL) {
        return 0;
    }
    return ctx->cached_rule_count;
}

/**
 * @brief 获取缓存的规则索引数组 / Get cached rule indices array / Gecachte Regelindex-Array abrufen
 * @return 缓存的规则索引数组指针 / Cached rule indices array pointer / Zeiger auf gecachte Regelindex-Array
 */
const size_t* get_cached_rule_indices(void) {
    pointer_transfer_context_t* ctx = get_global_context();
    if (ctx == NULL) {
        return NULL;
    }
    return ctx->cached_rule_indices;
}

/**
 * @brief 清理上下文 / Cleanup context / Kontext bereinigen
 */
void cleanup_context(void) {
    pointer_transfer_context_t* ctx = get_global_context();
    if (ctx == NULL) {
        return;
    }
    
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
                state->plugin_name = NULL;
            }
            if (state->interface_name != NULL) {
                free(state->interface_name);
                state->interface_name = NULL;
            }
            if (state->param_ready != NULL) {
                free(state->param_ready);
                state->param_ready = NULL;
            }
            if (state->param_values != NULL) {
                free(state->param_values);
                state->param_values = NULL;
            }
            if (state->param_types != NULL) {
                free(state->param_types);
                state->param_types = NULL;
            }
            if (state->param_sizes != NULL) {
                free(state->param_sizes);
                state->param_sizes = NULL;
            }
            if (state->param_int_values != NULL) {
                free(state->param_int_values);
                state->param_int_values = NULL;
            }
            if (state->param_float_values != NULL) {
                free(state->param_float_values);
                state->param_float_values = NULL;
            }
        }
        free(ctx->interface_states);
        ctx->interface_states = NULL;
        ctx->interface_state_count = 0;
        ctx->interface_state_capacity = 0;
    }
    if (ctx->loaded_plugins != NULL) {
        for (size_t i = 0; i < ctx->loaded_plugin_count; i++) {
            if (ctx->loaded_plugins[i].plugin_name != NULL) {
                free(ctx->loaded_plugins[i].plugin_name);
                ctx->loaded_plugins[i].plugin_name = NULL;
            }
            if (ctx->loaded_plugins[i].plugin_path != NULL) {
                free(ctx->loaded_plugins[i].plugin_path);
                ctx->loaded_plugins[i].plugin_path = NULL;
            }
            if (ctx->loaded_plugins[i].handle != NULL) {
                pt_platform_close_library(ctx->loaded_plugins[i].handle);
                ctx->loaded_plugins[i].handle = NULL;
            }
        }
        free(ctx->loaded_plugins);
        ctx->loaded_plugins = NULL;
        ctx->loaded_plugin_count = 0;
        ctx->loaded_plugin_capacity = 0;
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
                ctx->loaded_nxpt_files[i].plugin_name = NULL;
            }
            if (ctx->loaded_nxpt_files[i].nxpt_path != NULL) {
                free(ctx->loaded_nxpt_files[i].nxpt_path);
                ctx->loaded_nxpt_files[i].nxpt_path = NULL;
            }
        }
        free(ctx->loaded_nxpt_files);
        ctx->loaded_nxpt_files = NULL;
        ctx->loaded_nxpt_count = 0;
        ctx->loaded_nxpt_capacity = 0;
    }
    free_nxpt_hash_table_internal();
    
    int saved_disable_info_log = ctx->disable_info_log;
    int saved_enable_validation = ctx->enable_validation;
    
    /* 释放忽略插件列表 / Free ignored plugins list / Liste der ignorierten Plugins freigeben */
    if (ctx->ignore_plugins != NULL) {
        for (size_t i = 0; i < ctx->ignore_plugin_count; i++) {
            if (ctx->ignore_plugins[i] != NULL) {
                free(ctx->ignore_plugins[i]);
                ctx->ignore_plugins[i] = NULL;
            }
        }
        free(ctx->ignore_plugins);
        ctx->ignore_plugins = NULL;
        ctx->ignore_plugin_count = 0;
        ctx->ignore_plugin_capacity = 0;
    }
    
    /* 重置所有字段 / Reset all fields / Alle Felder zurücksetzen */
    memset(ctx, 0, sizeof(pointer_transfer_context_t));
    ctx->stored_type = NXLD_PARAM_TYPE_UNKNOWN;
    ctx->disable_info_log = saved_disable_info_log;  /* 恢复日志配置状态 / Restore log configuration state / Protokollkonfigurationsstatus wiederherstellen */
    ctx->enable_validation = saved_enable_validation;  /* 恢复验证配置状态 / Restore validation configuration state / Validierungskonfigurationsstatus wiederherstellen */
    
    internal_log_write("INFO", "Context cleaned up successfully");
}

/**
 * @brief 添加忽略插件路径 / Add ignored plugin path / Ignorierten Plugin-Pfad hinzufügen
 * @param plugin_path 插件路径（相对路径） / Plugin path (relative path) / Plugin-Pfad (relativer Pfad)
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
static int add_ignore_plugin_path(const char* plugin_path) {
    if (plugin_path == NULL) {
        return -1;
    }
    
    pointer_transfer_context_t* ctx = get_global_context();
    if (ctx == NULL) {
        return -1;
    }
    
    /* 检查是否已存在 / Check if already exists / Prüfen, ob bereits vorhanden */
    for (size_t i = 0; i < ctx->ignore_plugin_count; i++) {
        if (ctx->ignore_plugins[i] != NULL && strcmp(ctx->ignore_plugins[i], plugin_path) == 0) {
            /* 已存在，返回成功 / Already exists, return success / Bereits vorhanden, Erfolg zurückgeben */
            return 0;
        }
    }
    
    /* 检查是否需要扩容 / Check if capacity expansion is needed / Prüfen, ob Kapazitätserweiterung erforderlich ist */
    if (ctx->ignore_plugin_count >= ctx->ignore_plugin_capacity) {
        size_t new_capacity = ctx->ignore_plugin_capacity == 0 ? 8 : ctx->ignore_plugin_capacity * 2;
        
        if (new_capacity < ctx->ignore_plugin_capacity || new_capacity > SIZE_MAX / sizeof(char*)) {
            internal_log_write("ERROR", "add_ignore_plugin_path: capacity overflow detected (current=%zu, new=%zu)", 
                              ctx->ignore_plugin_capacity, new_capacity);
            return -1;
        }
        
        char** new_plugins = (char**)realloc(ctx->ignore_plugins, new_capacity * sizeof(char*));
        if (new_plugins == NULL) {
            internal_log_write("ERROR", "add_ignore_plugin_path: failed to allocate memory for ignore plugins array (new_capacity=%zu)", new_capacity);
            return -1;
        }
        
        memset(new_plugins + ctx->ignore_plugin_count, 0, (new_capacity - ctx->ignore_plugin_count) * sizeof(char*));
        ctx->ignore_plugins = new_plugins;
        ctx->ignore_plugin_capacity = new_capacity;
    }
    
    /* 分配并复制路径字符串 / Allocate and copy path string / Pfadzeichenfolge zuweisen und kopieren */
    char* path_copy = allocate_string(plugin_path);
    if (path_copy == NULL) {
        internal_log_write("ERROR", "add_ignore_plugin_path: failed to allocate memory for plugin path: %s", plugin_path);
        return -1;
    }
    
    ctx->ignore_plugins[ctx->ignore_plugin_count] = path_copy;
    ctx->ignore_plugin_count++;
    
    internal_log_write("INFO", "Added ignored plugin path: %s", plugin_path);
    return 0;
}

/**
 * @brief 检查插件路径是否在忽略列表中 / Check if plugin path is in ignore list / Prüfen, ob Plugin-Pfad in Ignorierliste ist
 * @param plugin_path 插件路径（可以是绝对路径或相对路径） / Plugin path (can be absolute or relative path) / Plugin-Pfad (kann absoluter oder relativer Pfad sein)
 * @return 在忽略列表中返回1，否则返回0 / Returns 1 if in ignore list, 0 otherwise / Gibt 1 zurück, wenn in Ignorierliste, sonst 0
 */
int is_plugin_ignored(const char* plugin_path) {
    if (plugin_path == NULL) {
        return 0;
    }
    
    pointer_transfer_context_t* ctx = get_global_context();
    if (ctx == NULL) {
        return 0;
    }
    
    if (ctx->ignore_plugins == NULL || ctx->ignore_plugin_count == 0) {
        return 0;
    }
    
    /* 将路径转换为相对路径格式（统一使用正斜杠） / Convert path to relative path format (use forward slash uniformly) / Pfad in relatives Pfadformat konvertieren (einheitlich Schrägstrich verwenden) */
    char normalized_path[1024];
#ifdef _WIN32
    strncpy_s(normalized_path, sizeof(normalized_path), plugin_path, _TRUNCATE);
#else
    strncpy(normalized_path, plugin_path, sizeof(normalized_path) - 1);
    normalized_path[sizeof(normalized_path) - 1] = '\0';
#endif
    
    /* 将反斜杠转换为正斜杠（Windows兼容） / Convert backslashes to forward slashes (Windows compatible) / Umgekehrte Schrägstriche in Schrägstriche konvertieren (Windows-kompatibel) */
    for (size_t i = 0; i < strlen(normalized_path); i++) {
        if (normalized_path[i] == '\\') {
            normalized_path[i] = '/';
        }
    }
    
    /* 提取相对路径部分（从 "plugins/" 开始） / Extract relative path part (starting from "plugins/") / Relativen Pfadteil extrahieren (beginnend mit "plugins/") */
    const char* plugins_pos = strstr(normalized_path, "plugins/");
    if (plugins_pos == NULL) {
        /* 未找到 "plugins/"，返回未匹配 / If "plugins/" not found, return no match / Wenn "plugins/" nicht gefunden, keine Übereinstimmung zurückgeben */
        return 0;
    }
    
    /* 提取自 "plugins/" 开始的相对路径 / Extract relative path starting from "plugins/" / Relativen Pfad ab "plugins/" extrahieren */
    const char* relative_path = plugins_pos;
    
    /* 执行相对路径的精确匹配 / Perform exact match of relative path / Exakte Übereinstimmung des relativen Pfads durchführen */
    for (size_t i = 0; i < ctx->ignore_plugin_count; i++) {
        if (ctx->ignore_plugins[i] != NULL) {
            if (strcmp(relative_path, ctx->ignore_plugins[i]) == 0) {
                return 1;
            }
        }
    }
    
    return 0;
}

/**
 * @brief 初始化默认忽略插件列表 / Initialize default ignored plugins list / Standard-Ignorierliste für Plugins initialisieren
 * @note 当前不默认忽略任何插件，所有忽略项需通过配置文件指定 / Currently does not ignore any plugins by default, all ignore items must be specified via config file / Ignoriert derzeit standardmäßig keine Plugins, alle Ignorier-Einträge müssen über Konfigurationsdatei angegeben werden
 */
void initialize_default_ignore_plugins(void) {
    /* 当前不默认忽略任何插件 / Currently does not ignore any plugins by default / Ignoriert derzeit standardmäßig keine Plugins */
    /* 所有忽略项需通过配置文件中的 IgnorePlugins 配置项指定 / All ignore items must be specified via IgnorePlugins config item / Alle Ignorier-Einträge müssen über IgnorePlugins-Konfigurationselement angegeben werden */
}

