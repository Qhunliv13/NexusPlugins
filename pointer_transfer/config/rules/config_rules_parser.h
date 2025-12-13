/**
 * @file config_rules_parser.h
 * @brief 规则键值解析器接口 / Rule Key-Value Parser Interface / Regel-Schlüssel-Wert-Parser-Schnittstelle
 */

#ifndef CONFIG_RULES_PARSER_H
#define CONFIG_RULES_PARSER_H

#include "pointer_transfer_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 解析规则键值对 / Parse rule key-value pair / Regel-Schlüssel-Wert-Paar parsen
 * @param rule 规则结构体指针 / Rule structure pointer / Regelstruktur-Zeiger
 * @param key 键名 / Key name / Schlüsselname
 * @param value 值 / Value / Wert
 */
void parse_rule_key_value(pointer_transfer_rule_t* rule, const char* key, const char* value);

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_RULES_PARSER_H */

