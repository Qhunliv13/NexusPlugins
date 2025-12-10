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

#ifdef __cplusplus
}
#endif

#endif /* POINTER_TRANSFER_CONTEXT_H */

