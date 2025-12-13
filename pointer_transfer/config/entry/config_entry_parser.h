/**
 * @file config_entry_parser.h
 * @brief 入口配置解析器接口 / Entry Configuration Parser Interface / Einstiegs-Konfigurationsparser-Schnittstelle
 */

#ifndef CONFIG_ENTRY_PARSER_H
#define CONFIG_ENTRY_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 解析入口插件配置 / Parse entry plugin configuration / Einstiegs-Plugin-Konfiguration parsen
 * @param config_path 配置文件路径 / Configuration file path / Konfigurationsdateipfad
 * @return 成功返回0，失败返回非0 / Returns 0 on success, non-zero on failure / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
 */
int parse_entry_plugin_config(const char* config_path);

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_ENTRY_PARSER_H */

