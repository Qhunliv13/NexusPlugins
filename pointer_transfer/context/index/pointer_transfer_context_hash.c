/**
 * @file pointer_transfer_context_hash.c
 * @brief 规则哈希表操作 / Rule Hash Table Operations / Regel-Hash-Tabellen-Operationen
 */

#include "pointer_transfer_context.h"
#include "pointer_transfer_utils.h"
#include "pointer_transfer_types.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <stdio.h>

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
uint64_t calculate_rule_hash_key(const char* source_plugin, const char* source_interface, int source_param_index) {
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

/* 导出给 index.c 使用的函数 / Functions exported for use by index.c / Für index.c exportierte Funktionen */
uint64_t calculate_rule_hash_key_for_index(const char* source_plugin, const char* source_interface, int source_param_index) {
    return calculate_rule_hash_key(source_plugin, source_interface, source_param_index);
}

void free_hash_table_for_index(rule_hash_table_t* hash_table) {
    free_hash_table(hash_table);
}

int insert_rule_into_hash_table_for_index(rule_hash_table_t* hash_table, uint64_t hash_key, size_t rule_index) {
    return insert_rule_into_hash_table(hash_table, hash_key, rule_index);
}

int expand_hash_table_for_index(rule_hash_table_t* hash_table) {
    return expand_hash_table(hash_table);
}

size_t get_hash_table_initial_size(void) {
    return HASH_TABLE_INITIAL_SIZE;
}

