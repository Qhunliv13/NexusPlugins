/**
 * @file config_rules_scanner.h
 * @brief 规则配置扫描器接口 / Rules Configuration Scanner Interface / Regeln-Konfigurationsscanner-Schnittstelle
 */

#ifndef CONFIG_RULES_SCANNER_H
#define CONFIG_RULES_SCANNER_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 扫描配置文件查找设置 / Scan config file for settings / Konfigurationsdatei nach Einstellungen scannen
 * @param fp 文件指针 / File pointer / Dateizeiger
 */
void scan_config_for_settings(FILE* fp);

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_RULES_SCANNER_H */

