/**
 * @file pointer_transfer_context_cache_get.c
 * @brief 规则缓存获取操作 / Rule Cache Get Operations / Regel-Cache-Abrufoperationen
 */

#include "pointer_transfer_context.h"
#include <stddef.h>

/**
 * @brief 获取缓存的规则数量 / Get cached rule count / Anzahl gecachter Regeln abrufen
 * @return 缓存的规则数量 / Cached rule count / Anzahl gecachter Regeln
 */
size_t get_cached_rule_count(void) {
    pointer_transfer_context_t* ctx = get_global_context();
    if (ctx == NULL) {
        return 0;
    }
    return ctx->cached_rule_count;
}

/**
 * @brief 获取缓存的规则索引数组 / Get cached rule indices array / Gecachte Regelindex-Array abrufen
 * @return 缓存的规则索引数组指针 / Cached rule indices array pointer / Zeiger auf gecachte Regelindex-Array
 */
const size_t* get_cached_rule_indices(void) {
    pointer_transfer_context_t* ctx = get_global_context();
    if (ctx == NULL) {
        return NULL;
    }
    return ctx->cached_rule_indices;
}

