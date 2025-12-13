/**
 * @file pointer_transfer_context_index.c
 * @brief 规则索引和哈希表操作 / Rule Index and Hash Table Operations / Regelindex und Hash-Tabellen-Operationen
 */

#include "pointer_transfer_context.h"
#include "pointer_transfer_utils.h"
#include "pointer_transfer_types.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

/* 哈希表常量 / Hash table constants / Hash-Tabellen-Konstanten */
#define HASH_TABLE_INITIAL_SIZE 16
#define HASH_TABLE_LOAD_FACTOR 0.75

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

