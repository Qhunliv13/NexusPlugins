/**
 * @file config_parser_common.h
 * @brief 配置解析公共函数接口 / Configuration Parser Common Functions Interface / Konfigurationsparser-Gemeinsame Funktionen-Schnittstelle
 */

#ifndef CONFIG_PARSER_COMMON_H
#define CONFIG_PARSER_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 解析布尔值配置 / Parse boolean configuration / Boolesche Konfiguration parsen
 * @param value 字符串值 / String value / Zeichenfolgenwert
 * @return 1表示true, 0表示false / Returns 1 for true, 0 for false / Gibt 1 für true zurück, 0 für false
 */
int parse_boolean_value(const char* value);

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_PARSER_COMMON_H */

