/**
 * @file pointer_transfer_plugin_loader.h
 * @brief 指针传递插件加载器 / Pointer Transfer Plugin Loader / Zeigerübertragungs-Plugin-Lader
 */

#ifndef POINTER_TRANSFER_PLUGIN_LOADER_H
#define POINTER_TRANSFER_PLUGIN_LOADER_H

#include "pointer_transfer_types.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 加载目标插件 / Load target plugin / Ziel-Plugin laden
 * @param plugin_name 插件名称 / Plugin name / Plugin-Name
 * @param plugin_path 插件路径 / Plugin path / Plugin-Pfad
 * @return 成功返回插件句柄，失败返回NULL / Returns plugin handle on success, NULL on failure / Gibt Plugin-Handle bei Erfolg zurück, NULL bei Fehler
 */
void* load_target_plugin(const char* plugin_name, const char* plugin_path);

/**
 * @brief 链式加载插件的.nxpt文件 / Chain load plugin .nxpt file / Plugin-.nxpt-Datei kettenweise laden
 * @param plugin_name 插件名称 / Plugin name / Plugin-Name
 * @param plugin_path 插件路径 / Plugin path / Plugin-Pfad
 * @return 成功返回0，失败返回非0 / Returns 0 on success, non-zero on failure / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
 */
int chain_load_plugin_nxpt(const char* plugin_name, const char* plugin_path);

/**
 * @brief 获取插件路径（缓存） / Get plugin path (cached) / Plugin-Pfad abrufen (gecacht)
 * @param plugin_name 插件名称 / Plugin name / Plugin-Name
 * @param plugin_path 输出路径缓冲区 / Output path buffer / Ausgabe-Pfad-Puffer
 * @param path_size 路径缓冲区大小 / Path buffer size / Pfad-Puffergröße
 * @return 成功返回0，失败返回非0 / Returns 0 on success, non-zero on failure / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
 */
int get_plugin_path_cached(const char* plugin_name, char* plugin_path, size_t path_size);

#ifdef __cplusplus
}
#endif

#endif /* POINTER_TRANSFER_PLUGIN_LOADER_H */

