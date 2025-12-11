/**
 * @file pointer_transfer_plugin.h
 * @brief NXLD指针传递插件接口 / NXLD Pointer Transfer Plugin Interface / NXLD-Zeigerübertragungs-Plugin-Schnittstelle
 */

#ifndef POINTER_TRANSFER_PLUGIN_H
#define POINTER_TRANSFER_PLUGIN_H

#include "nxld_plugin_interface.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#define POINTER_TRANSFER_PLUGIN_EXPORT __declspec(dllexport)
#define POINTER_TRANSFER_PLUGIN_CALL __cdecl
#else
#define POINTER_TRANSFER_PLUGIN_EXPORT __attribute__((visibility("default")))
#define POINTER_TRANSFER_PLUGIN_CALL
#endif

/**
 * @brief 传递指针 / Transfer pointer / Zeiger übertragen
 * @param source_plugin_name 源插件名称 / Source plugin name / Quell-Plugin-Name
 * @param source_interface_name 源接口名称 / Source interface name / Quell-Schnittstellenname
 * @param source_param_index 源参数索引 / Source parameter index / Quell-Parameterindex
 * @param ptr 指针 / Pointer / Zeiger
 * @param expected_type 数据类型 / Data type / Datentyp
 * @param type_name 类型名称 / Type name / Typname
 * @param data_size 数据大小 / Data size / Datengröße
 * @return 成功返回0，类型不匹配返回1，其他错误返回-1 / Returns 0 on success, 1 on type mismatch, -1 on other errors / Gibt 0 bei Erfolg zurück, 1 bei Typfehlanpassung, -1 bei anderen Fehlern
 */
POINTER_TRANSFER_PLUGIN_EXPORT int POINTER_TRANSFER_PLUGIN_CALL TransferPointer(const char* source_plugin_name, const char* source_interface_name, int source_param_index, void* ptr, nxld_param_type_t expected_type, const char* type_name, size_t data_size);

/**
 * @brief 调用目标插件接口 / Call target plugin interface / Ziel-Plugin-Schnittstelle aufrufen
 * @param source_plugin_name 源插件名称 / Source plugin name / Quell-Plugin-Name
 * @param source_interface_name 源接口名称 / Source interface name / Quell-Schnittstellenname
 * @param param_index 参数索引 / Parameter index / Parameterindex
 * @param param_value 参数值 / Parameter value / Parameterwert
 * @return 成功返回0，失败返回非0 / Returns 0 on success, non-zero on failure / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
 */
POINTER_TRANSFER_PLUGIN_EXPORT int POINTER_TRANSFER_PLUGIN_CALL CallPlugin(const char* source_plugin_name, const char* source_interface_name, int param_index, void* param_value);

#ifdef __cplusplus
}
#endif

#endif /* POINTER_TRANSFER_PLUGIN_H */

