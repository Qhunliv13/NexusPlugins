/**
 * @file pointer_transfer_config_hash.c
 * @brief 配置哈希表操作 / Configuration Hash Table Operations / Konfigurations-Hash-Tabellen-Operationen
 */

#include "pointer_transfer_config.h"
#include "pointer_transfer_context.h"
#include "pointer_transfer_utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

/* 哈希表常量 / Hash table constants / Hash-Tabellen-Konstanten */
#define NXPT_HASH_TABLE_INITIAL_SIZE 8
#define NXPT_HASH_TABLE_LOAD_FACTOR 0.75

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
 * @return 成功返回0，失败返回相应错误码 / Returns 0 on success, error code on failure / Gibt 0 bei Erfolg zurück, Fehlercode bei Fehler
 */
int mark_nxpt_loaded(const char* plugin_name, const char* nxpt_path) {
    if (plugin_name == NULL || nxpt_path == NULL) {
        internal_log_write("ERROR", "mark_nxpt_loaded: invalid parameters (plugin_name=%p, nxpt_path=%p)", 
            (void*)plugin_name, (void*)nxpt_path);
        return -1;
    }
    
    if (is_nxpt_loaded(plugin_name)) {
        return 0;
    }
    
    pointer_transfer_context_t* ctx = get_global_context();
    if (ctx == NULL) {
        internal_log_write("ERROR", "mark_nxpt_loaded: global context is NULL");
        return -1;
    }
    
    /* 检查是否需要扩容 / Check if capacity expansion is needed / Prüfen, ob Kapazitätserweiterung erforderlich ist */
    if (ctx->loaded_nxpt_count >= ctx->loaded_nxpt_capacity) {
        size_t new_capacity = ctx->loaded_nxpt_capacity == 0 ? 8 : ctx->loaded_nxpt_capacity * 2;
        
        /* 检查溢出 / Check overflow / Überlauf prüfen */
        if (new_capacity > SIZE_MAX / sizeof(loaded_nxpt_info_t)) {
            internal_log_write("ERROR", "mark_nxpt_loaded: capacity overflow detected (new_capacity=%zu)", new_capacity);
            return -5;
        }
        
        loaded_nxpt_info_t* new_files = (loaded_nxpt_info_t*)realloc(ctx->loaded_nxpt_files, new_capacity * sizeof(loaded_nxpt_info_t));
        if (new_files == NULL) {
            internal_log_write("ERROR", "mark_nxpt_loaded: failed to allocate memory for nxpt files array (new_capacity=%zu)", new_capacity);
            return -3;
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
        return -3;
    }
    
    info->nxpt_path = allocate_string(nxpt_path);
    if (info->nxpt_path == NULL) {
        internal_log_write("ERROR", "mark_nxpt_loaded: failed to allocate memory for nxpt_path");
        free(info->plugin_name);
        info->plugin_name = NULL;
        return -3;
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
    
    return 0;
}

