/**
 * @file pointer_transfer_context_cache_build.c
 * @brief 规则缓存构建操作 / Rule Cache Building Operations / Regel-Cache-Erstellungsoperationen
 */

#include "pointer_transfer_context.h"
#include "pointer_transfer_utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

/**
 * @brief 构建规则缓存 / Build rule cache / Regel-Cache erstellen
 * @return 成功返回0，失败返回非0 / Returns 0 on success, non-zero on failure / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
 */
int build_rule_cache(void) {
    pointer_transfer_context_t* ctx = get_global_context();
    if (ctx == NULL) {
        internal_log_write("ERROR", "build_rule_cache: global context is NULL");
        return -1;
    }
    
    /* 释放旧缓存 / Free old cache / Alten Cache freigeben */
    if (ctx->cached_rule_indices != NULL) {
        free(ctx->cached_rule_indices);
        ctx->cached_rule_indices = NULL;
        ctx->cached_rule_count = 0;
        ctx->cached_rule_capacity = 0;
    }
    
    if (ctx->rule_count == 0 || ctx->rules == NULL) {
        internal_log_write("INFO", "build_rule_cache: no rules to cache (rule_count=%zu)", ctx->rule_count);
        return 0;
    }
    
    /* 统计需要缓存的规则数量 / Count rules that require caching / Regeln zählen, die gecacht werden müssen */
    size_t cache_count = 0;
    for (size_t i = 0; i < ctx->rule_count; i++) {
        if (ctx->rules[i].cache_self && ctx->rules[i].enabled) {
            cache_count++;
        }
    }
    
    if (cache_count == 0) {
        internal_log_write("INFO", "build_rule_cache: no rules marked for caching");
        return 0;
    }
    
    /* 检查容量溢出 / Check capacity overflow / Kapazitätsüberlauf prüfen */
    if (cache_count > SIZE_MAX / sizeof(size_t)) {
        internal_log_write("ERROR", "build_rule_cache: cache_count overflow detected (cache_count=%zu)", cache_count);
        return -1;
    }
    
    /* 分配缓存数组 / Allocate cache array / Cache-Array zuweisen */
    ctx->cached_rule_indices = (size_t*)malloc(cache_count * sizeof(size_t));
    if (ctx->cached_rule_indices == NULL) {
        internal_log_write("ERROR", "build_rule_cache: failed to allocate memory for cache array (cache_count=%zu)", cache_count);
        return -1;
    }
    
    /* 填充缓存数组 / Fill cache array / Cache-Array füllen */
    size_t cache_index = 0;
    for (size_t i = 0; i < ctx->rule_count; i++) {
        if (ctx->rules[i].cache_self && ctx->rules[i].enabled) {
            ctx->cached_rule_indices[cache_index++] = i;
        }
    }
    
    ctx->cached_rule_count = cache_count;
    ctx->cached_rule_capacity = cache_count;
    
    internal_log_write("INFO", "Built rule cache with %zu cached rules", cache_count);
    return 0;
}

