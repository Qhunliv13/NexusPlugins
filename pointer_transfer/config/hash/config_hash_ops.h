/**
 * @file config_hash_ops.h
 * @brief 哈希表操作函数接口 / Hash Table Operations Interface / Hash-Tabellen-Operationen-Schnittstelle
 */

#ifndef CONFIG_HASH_OPS_H
#define CONFIG_HASH_OPS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 清理已加载.nxpt文件哈希表 / Cleanup loaded .nxpt files hash table / Geladene .nxpt-Dateien-Hash-Tabelle bereinigen
 */
void free_nxpt_hash_table_internal(void);

/**
 * @brief 查询.nxpt文件加载状态（使用哈希表优化）/ Query .nxpt file load status (optimized with hash table) / .nxpt-Datei-Ladestatus abfragen (mit Hash-Tabelle optimiert)
 * @param plugin_name 插件名称 / Plugin name / Plugin-Name
 * @return 已加载返回1，未加载返回0 / Returns 1 if loaded, 0 if not loaded / Gibt 1 zurück, wenn geladen, 0 wenn nicht geladen
 */
int is_nxpt_loaded(const char* plugin_name);

/**
 * @brief 标记.nxpt文件为已加载 / Mark .nxpt file as loaded / .nxpt-Datei als geladen markieren
 * @param plugin_name 插件名称 / Plugin name / Plugin-Name
 * @param nxpt_path .nxpt文件路径 / .nxpt file path / .nxpt-Dateipfad
 * @return 成功返回0，失败返回非0 / Returns 0 on success, non-zero on failure / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
 */
int mark_nxpt_loaded(const char* plugin_name, const char* nxpt_path);

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_HASH_OPS_H */

