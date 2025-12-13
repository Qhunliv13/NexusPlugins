/**
 * @file config_hash_calc.h
 * @brief 哈希计算函数接口 / Hash Calculation Functions Interface / Hash-Berechnungsfunktionen-Schnittstelle
 */

#ifndef CONFIG_HASH_CALC_H
#define CONFIG_HASH_CALC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 计算插件名称的哈希值（FNV-1a算法）/ Calculate hash value of plugin name (FNV-1a algorithm) / Hash-Wert des Plugin-Namens berechnen (FNV-1a-Algorithmus)
 * @param plugin_name 插件名称 / Plugin name / Plugin-Name
 * @return 哈希值 / Hash value / Hash-Wert
 */
uint64_t hash_plugin_name(const char* plugin_name);

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_HASH_CALC_H */

