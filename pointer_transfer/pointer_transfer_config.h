/**
 * @file pointer_transfer_config.h
 * @brief 指针传递插件配置接口 / Pointer Transfer Plugin Configuration Interface / Zeigerübertragungs-Plugin-Konfigurationsschnittstelle
 */

#ifndef POINTER_TRANSFER_CONFIG_H
#define POINTER_TRANSFER_CONFIG_H

#include "pointer_transfer_types.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 解析入口插件配置 / Parse entry plugin configuration / Einstiegs-Plugin-Konfiguration analysieren
 * @param config_path 配置文件路径 / Configuration file path / Konfigurationsdateipfad
 * @return 成功返回0，失败返回非0 / Returns 0 on success, non-zero on failure / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
 */
int parse_entry_plugin_config(const char* config_path);

/**
 * @brief 加载传递规则配置文件 / Load transfer rules configuration file / Übertragungsregel-Konfigurationsdatei laden
 * @param config_path 配置文件路径 / Configuration file path / Konfigurationsdateipfad
 * @return 成功返回0，失败返回非0 / Returns 0 on success, non-zero on failure / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
 */
int load_transfer_rules(const char* config_path);

/**
 * @brief 检查.nxpt文件是否已加载 / Check if .nxpt file is loaded / Prüfen, ob .nxpt-Datei geladen ist
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

#endif /* POINTER_TRANSFER_CONFIG_H */

