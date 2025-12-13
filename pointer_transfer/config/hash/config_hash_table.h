/**
 * @file config_hash_table.h
 * @brief 哈希表管理接口 / Hash Table Management Interface / Hash-Tabellen-Verwaltung-Schnittstelle
 */

#ifndef CONFIG_HASH_TABLE_H
#define CONFIG_HASH_TABLE_H

#include "pointer_transfer_types.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 确保哈希表有足够容量 / Ensure hash table has sufficient capacity / Sicherstellen, dass Hash-Tabelle ausreichend Kapazität hat
 * @param hash_table 哈希表指针 / Hash table pointer / Hash-Tabellen-Zeiger
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int ensure_nxpt_hash_table_capacity(nxpt_hash_table_t* hash_table);

/**
 * @brief 插入条目到哈希表 / Insert entry into hash table / Eintrag in Hash-Tabelle einfügen
 * @param hash_table 哈希表指针 / Hash table pointer / Hash-Tabellen-Zeiger
 * @param hash_key 哈希键 / Hash key / Hash-Schlüssel
 * @param array_index 数组索引 / Array index / Array-Index
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int insert_into_nxpt_hash_table(nxpt_hash_table_t* hash_table, uint64_t hash_key, size_t array_index);

/**
 * @brief 从哈希表中移除条目 / Remove entry from hash table / Eintrag aus Hash-Tabelle entfernen
 * @param hash_table 哈希表指针 / Hash table pointer / Hash-Tabellen-Zeiger
 * @param array_index 数组索引 / Array index / Array-Index
 * @param plugin_name 插件名称 / Plugin name / Plugin-Name
 */
void remove_from_nxpt_hash_table(nxpt_hash_table_t* hash_table, size_t array_index, const char* plugin_name);

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_HASH_TABLE_H */

