/**
 * @file pointer_transfer_context_index.c
 * @brief 规则索引构建和查找 / Rule Index Building and Searching / Regelindex-Erstellung und -Suche
 */

#include "pointer_transfer_context.h"
#include "pointer_transfer_utils.h"
#include "pointer_transfer_types.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

/* 从 hash.c 导入函数 / Import functions from hash.c / Funktionen aus hash.c importieren */
extern uint64_t calculate_rule_hash_key_for_index(const char* source_plugin, const char* source_interface, int source_param_index);
extern void free_hash_table_for_index(rule_hash_table_t* hash_table);
extern int insert_rule_into_hash_table_for_index(rule_hash_table_t* hash_table, uint64_t hash_key, size_t rule_index);
extern int expand_hash_table_for_index(rule_hash_table_t* hash_table);
extern size_t get_hash_table_initial_size(void);

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
    free_hash_table_for_index(&ctx->rule_hash_table);
    
    if (ctx->rule_count == 0 || ctx->rules == NULL) {
        internal_log_write("INFO", "build_rule_index: no rules to index (rule_count=%zu)", ctx->rule_count);
        return 0;
    }
    
    /* 初始化哈希表 / Initialize hash table / Hash-Tabelle initialisieren */
    ctx->rule_hash_table.bucket_count = get_hash_table_initial_size();
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
        
        uint64_t hash_key = calculate_rule_hash_key_for_index(rule->source_plugin, rule->source_interface, rule->source_param_index);
        if (hash_key != 0) {
            if (insert_rule_into_hash_table_for_index(&ctx->rule_hash_table, hash_key, i) != 0) {
                internal_log_write("ERROR", "build_rule_index: failed to insert rule %zu into hash table", i);
                free_hash_table_for_index(&ctx->rule_hash_table);
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
    uint64_t hash_key = calculate_rule_hash_key_for_index(source_plugin, source_interface, source_param_index);
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
