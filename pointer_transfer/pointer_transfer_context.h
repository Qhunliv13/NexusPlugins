/**
 * @file pointer_transfer_context.h
 * @brief 指针传递插件上下文操作 / Pointer Transfer Plugin Context Operations / Zeigerübertragungs-Plugin-Kontextoperationen
 */

#ifndef POINTER_TRANSFER_CONTEXT_H
#define POINTER_TRANSFER_CONTEXT_H

#include "pointer_transfer_types.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 容量常量 / Capacity constants / Kapazitätskonstanten */
#define INITIAL_RULE_CAPACITY 16
#define INITIAL_PLUGIN_CAPACITY 8
#define INITIAL_INTERFACE_STATE_CAPACITY 8
#define CAPACITY_GROWTH_FACTOR 2

/**
 * @brief 获取全局上下文指针 / Get global context pointer / Globalen Kontextzeiger abrufen
 * @return 全局上下文指针 / Global context pointer / Globaler Kontextzeiger
 */
pointer_transfer_context_t* get_global_context(void);

/**
 * @brief 扩展规则数组容量 / Expand rules array capacity / Regel-Array-Kapazität erweitern
 * @return 成功返回0，失败返回非0 / Returns 0 on success, non-zero on failure / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
 */
int expand_rules_capacity(void);

/**
 * @brief 扩展已加载插件数组容量 / Expand loaded plugins array capacity / Geladenes Plugin-Array-Kapazität erweitern
 * @return 成功返回0，失败返回非0 / Returns 0 on success, non-zero on failure / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
 */
int expand_loaded_plugins_capacity(void);

/**
 * @brief 扩展接口状态数组容量 / Expand interface states array capacity / Schnittstellen-Status-Array-Kapazität erweitern
 * @return 成功返回0，失败返回非0 / Returns 0 on success, non-zero on failure / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
 */
int expand_interface_states_capacity(void);

/**
 * @brief 释放传递规则内存 / Free transfer rules memory / Übertragungsregel-Speicher freigeben
 */
void free_transfer_rules(void);

/**
 * @brief 初始化上下文 / Initialize context / Kontext initialisieren
 */
void init_context(void);

/**
 * @brief 清理上下文 / Cleanup context / Kontext bereinigen
 */
void cleanup_context(void);

/**
 * @brief 构建规则索引 / Build rule index / Regelindex erstellen
 * @return 成功返回0，失败返回非0 / Returns 0 on success, non-zero on failure / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
 */
int build_rule_index(void);

/**
 * @brief 查找规则索引范围 / Find rule index range / Regelindex-Bereich suchen
 * @param source_plugin 源插件名称 / Source plugin name / Quell-Plugin-Name
 * @param source_interface 源接口名称 / Source interface name / Quell-Schnittstellenname
 * @param source_param_index 源参数索引 / Source parameter index / Quell-Parameterindex
 * @param start_index 输出起始索引指针 / Output start index pointer / Ausgabe-Startindex-Zeiger
 * @param end_index 输出结束索引指针 / Output end index pointer / Ausgabe-Endindex-Zeiger
 * @return 找到返回1，未找到返回0 / Returns 1 if found, 0 if not found / Gibt 1 zurück wenn gefunden, 0 wenn nicht gefunden
 */
int find_rule_index_range(const char* source_plugin, const char* source_interface, int source_param_index, size_t* start_index, size_t* end_index);

/**
 * @brief 构建规则缓存 / Build rule cache / Regel-Cache erstellen
 * @return 成功返回0，失败返回非0 / Returns 0 on success, non-zero on failure / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
 */
int build_rule_cache(void);

/**
 * @brief 获取缓存的规则数量 / Get cached rule count / Anzahl gecachter Regeln abrufen
 * @return 缓存的规则数量 / Cached rule count / Anzahl gecachter Regeln
 */
size_t get_cached_rule_count(void);

/**
 * @brief 获取缓存的规则索引数组 / Get cached rule indices array / Gecachte Regelindex-Array abrufen
 * @return 缓存的规则索引数组指针 / Cached rule indices array pointer / Zeiger auf gecachte Regelindex-Array
 */
const size_t* get_cached_rule_indices(void);

/**
 * @brief 检查插件路径是否在忽略列表中 / Check if plugin path is in ignore list / Prüfen, ob Plugin-Pfad in Ignorierliste ist
 * @param plugin_path 插件路径（可以是绝对路径或相对路径） / Plugin path (can be absolute or relative path) / Plugin-Pfad (kann absoluter oder relativer Pfad sein)
 * @return 在忽略列表中返回1，否则返回0 / Returns 1 if in ignore list, 0 otherwise / Gibt 1 zurück, wenn in Ignorierliste, sonst 0
 */
int is_plugin_ignored(const char* plugin_path);

/**
 * @brief 初始化默认忽略插件列表 / Initialize default ignored plugins list / Standard-Ignorierliste für Plugins initialisieren
 */
void initialize_default_ignore_plugins(void);

#ifdef __cplusplus
}
#endif

#endif /* POINTER_TRANSFER_CONTEXT_H */

