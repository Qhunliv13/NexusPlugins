/**
 * @file config_hash_calc.c
 * @brief 哈希计算函数 / Hash Calculation Functions / Hash-Berechnungsfunktionen
 */

#include "config_hash_calc.h"
#include <stdint.h>

/**
 * @brief 计算插件名称的哈希值（FNV-1a算法）/ Calculate hash value of plugin name (FNV-1a algorithm) / Hash-Wert des Plugin-Namens berechnen (FNV-1a-Algorithmus)
 */
uint64_t hash_plugin_name(const char* plugin_name) {
    if (plugin_name == NULL) {
        return 0;
    }
    
    uint64_t hash = 14695981039346656037ULL; /* FNV偏移基数 / FNV offset basis / FNV-Offset-Basis */
    const char* p = plugin_name;
    
    while (*p != '\0') {
        hash ^= (uint64_t)(unsigned char)(*p);
        hash *= 1099511628211ULL; /* FNV质数 / FNV prime / FNV-Primzahl */
        p++;
    }
    
    return hash;
}

