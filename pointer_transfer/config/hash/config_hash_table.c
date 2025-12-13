/**
 * @file config_hash_table.c
 * @brief 哈希表管理 / Hash Table Management / Hash-Tabellen-Verwaltung
 */

#include "config_hash_table.h"
#include "config_hash_calc.h"
#include "../common/config_errors.h"
#include "pointer_transfer_types.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/* 哈希表常量 / Hash table constants / Hash-Tabellen-Konstanten */
#define NXPT_HASH_TABLE_INITIAL_SIZE 8
#define NXPT_HASH_TABLE_LOAD_FACTOR 0.75

/**
 * @brief 确保哈希表有足够容量 / Ensure hash table has sufficient capacity / Sicherstellen, dass Hash-Tabelle ausreichend Kapazität hat
 */
int ensure_nxpt_hash_table_capacity(nxpt_hash_table_t* hash_table) {
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
int insert_into_nxpt_hash_table(nxpt_hash_table_t* hash_table, uint64_t hash_key, size_t array_index) {
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
void remove_from_nxpt_hash_table(nxpt_hash_table_t* hash_table, size_t array_index, const char* plugin_name) {
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

