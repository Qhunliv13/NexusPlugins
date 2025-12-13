/**
 * @file loader/cache/pointer_transfer_plugin_loader_cache_add.c
 * @brief 插件路径缓存添加实现 / Plugin Path Cache Add Implementation / Plugin-Pfad-Cache-Hinzufügen-Implementierung
 */

#include "pointer_transfer_plugin_loader.h"
#include "pointer_transfer_context.h"
#include "pointer_transfer_utils.h"
#include <stdlib.h>
#include <string.h>

/**
 * @brief 扩展路径缓存容量 / Expand path cache capacity / Pfad-Cache-Kapazität erweitern
 * @param ctx 上下文 / Context / Kontext
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int expand_path_cache_capacity(pointer_transfer_context_t* ctx) {
    if (ctx == NULL) {
        return -1;
    }
    
    size_t new_capacity = ctx->path_cache_capacity == 0 ? 8 : ctx->path_cache_capacity * 2;
    plugin_path_cache_entry_t* new_cache = (plugin_path_cache_entry_t*)realloc(ctx->path_cache, new_capacity * sizeof(plugin_path_cache_entry_t));
    if (new_cache == NULL) {
        return -1;
    }
    
    ctx->path_cache = new_cache;
    memset(&ctx->path_cache[ctx->path_cache_count], 0, (new_capacity - ctx->path_cache_count) * sizeof(plugin_path_cache_entry_t));
    ctx->path_cache_capacity = new_capacity;
    
    return 0;
}

/**
 * @brief 添加路径到缓存 / Add path to cache / Pfad zum Cache hinzufügen
 * @param ctx 上下文 / Context / Kontext
 * @param plugin_name 插件名称 / Plugin name / Plugin-Name
 * @param plugin_path 插件路径 / Plugin path / Plugin-Pfad
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int add_path_to_cache(pointer_transfer_context_t* ctx, const char* plugin_name, const char* plugin_path) {
    if (ctx == NULL || plugin_name == NULL || plugin_path == NULL) {
        return -1;
    }
    
    if (ctx->path_cache == NULL || ctx->path_cache_count >= ctx->path_cache_capacity) {
        if (expand_path_cache_capacity(ctx) != 0) {
            return -1;
        }
    }
    
    plugin_path_cache_entry_t* cache_entry = &ctx->path_cache[ctx->path_cache_count];
    cache_entry->plugin_name = allocate_string(plugin_name);
    cache_entry->plugin_path = allocate_string(plugin_path);
    
    if (cache_entry->plugin_name != NULL && cache_entry->plugin_path != NULL) {
        ctx->path_cache_count++;
        return 0;
    } else {
        if (cache_entry->plugin_name != NULL) free(cache_entry->plugin_name);
        if (cache_entry->plugin_path != NULL) free(cache_entry->plugin_path);
        return -1;
    }
}

