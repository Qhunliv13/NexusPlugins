/**
 * @file pointer_transfer_interface.h
 * @brief 指针传递插件接口调用 / Pointer Transfer Plugin Interface Invocation / Zeigerübertragungs-Plugin-Schnittstellenaufruf
 */

#ifndef POINTER_TRANSFER_INTERFACE_H
#define POINTER_TRANSFER_INTERFACE_H

#include "pointer_transfer_types.h"
#include "nxld_plugin_interface.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* NXLD标准调用约定 / NXLD standard calling convention / NXLD-Standard-Aufrufkonvention */

/* call_function_generic已迁移至pointer_transfer_platform.c，替代函数为pt_platform_safe_call / call_function_generic migrated to pointer_transfer_platform.c, replaced by pt_platform_safe_call / call_function_generic nach pointer_transfer_platform.c migriert, ersetzt durch pt_platform_safe_call */

/**
 * @brief 查找目标接口状态（不创建）/ Find target interface state (without creating) / Ziel-Schnittstellenstatus suchen (ohne Erstellung)
 * @param plugin_name 插件名称 / Plugin name / Plugin-Name
 * @param interface_name 接口名称 / Interface name / Schnittstellenname
 * @return 成功返回接口状态指针，不存在返回NULL / Returns interface state pointer on success, NULL if not found / Gibt Schnittstellenstatus-Zeiger bei Erfolg zurück, NULL wenn nicht gefunden
 */
target_interface_state_t* find_interface_state(const char* plugin_name, const char* interface_name);

/**
 * @brief 查找或创建目标接口状态 / Find or create target interface state / Ziel-Schnittstellenstatus suchen oder erstellen
 * @param plugin_name 插件名称 / Plugin name / Plugin-Name
 * @param interface_name 接口名称 / Interface name / Schnittstellenname
 * @param handle 插件句柄 / Plugin handle / Plugin-Handle
 * @param func_ptr 函数指针 / Function pointer / Funktionszeiger
 * @return 成功返回接口状态指针，失败返回NULL / Returns interface state pointer on success, NULL on failure / Gibt Schnittstellenstatus-Zeiger bei Erfolg zurück, NULL bei Fehler
 */
target_interface_state_t* find_or_create_interface_state(const char* plugin_name, const char* interface_name, void* handle, void* func_ptr);

/**
 * @brief 调用目标插件接口 / Call target plugin interface / Ziel-Plugin-Schnittstelle aufrufen
 * @param rule 传递规则 / Transfer rule / Übertragungsregel
 * @param ptr 要传递的指针 / Pointer to transfer / Zu übertragender Zeiger
 * @return 成功返回0，失败返回非0 / Returns 0 on success, non-zero on failure / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
 */
int call_target_plugin_interface(const pointer_transfer_rule_t* rule, void* ptr);

#ifdef __cplusplus
}
#endif

#endif /* POINTER_TRANSFER_INTERFACE_H */

