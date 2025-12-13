/**
 * @file config_rules_loader.h
 * @brief 规则配置加载器接口 / Rules Configuration Loader Interface / Regeln-Konfigurationslader-Schnittstelle
 */

#ifndef CONFIG_RULES_LOADER_H
#define CONFIG_RULES_LOADER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 加载传递规则配置文件 / Load transfer rules configuration file / Übertragungsregel-Konfigurationsdatei laden
 * @param config_path 配置文件路径 / Configuration file path / Konfigurationsdateipfad
 * @return 成功返回0，失败返回非0 / Returns 0 on success, non-zero on failure / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
 */
int load_transfer_rules(const char* config_path);

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_RULES_LOADER_H */

