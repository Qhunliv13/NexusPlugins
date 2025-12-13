/**
 * @file loader/cache/pointer_transfer_plugin_loader_cache_get.c
 * @brief 插件路径缓存获取实现 / Plugin Path Cache Get Implementation / Plugin-Pfad-Cache-Abruf-Implementierung
 */

#include "pointer_transfer_plugin_loader.h"
#include "pointer_transfer_context.h"
#include "pointer_transfer_utils.h"
#include <stdlib.h>
#include <string.h>

/**
 * @brief 从缓存中获取插件路径 / Get plugin path from cache / Plugin-Pfad aus Cache abrufen
 * @param ctx 上下文 / Context / Kontext
 * @param plugin_name 插件名称 / Plugin name / Plugin-Name
 * @param plugin_path 输出路径缓冲区 / Output path buffer / Ausgabe-Pfad-Puffer
 * @param path_size 路径缓冲区大小 / Path buffer size / Pfad-Puffergröße
 * @return 找到返回0，否则返回-1 / Returns 0 if found, -1 otherwise / Gibt 0 zurück, wenn gefunden, sonst -1
 */
int get_plugin_path_from_cache(pointer_transfer_context_t* ctx, const char* plugin_name, char* plugin_path, size_t path_size) {
    if (ctx == NULL || ctx->path_cache == NULL || plugin_name == NULL || plugin_path == NULL || path_size == 0) {
        return -1;
    }
    
    for (size_t i = 0; i < ctx->path_cache_count; i++) {
        if (ctx->path_cache[i].plugin_name != NULL &&
            strcmp(ctx->path_cache[i].plugin_name, plugin_name) == 0) {
            size_t cached_len = strlen(ctx->path_cache[i].plugin_path);
            if (cached_len < path_size) {
                memcpy(plugin_path, ctx->path_cache[i].plugin_path, cached_len + 1);
                return 0;
            }
            return -1;
        }
    }
    
    return -1;
}

/**
 * @brief 从已加载插件中获取路径 / Get path from loaded plugins / Pfad aus geladenen Plugins abrufen
 * @param ctx 上下文 / Context / Kontext
 * @param plugin_name 插件名称 / Plugin name / Plugin-Name
 * @param plugin_path 输出路径缓冲区 / Output path buffer / Ausgabe-Pfad-Puffer
 * @param path_size 路径缓冲区大小 / Path buffer size / Pfad-Puffergröße
 * @return 找到返回0，否则返回-1 / Returns 0 if found, -1 otherwise / Gibt 0 zurück, wenn gefunden, sonst -1
 */
int get_plugin_path_from_loaded(pointer_transfer_context_t* ctx, const char* plugin_name, char* plugin_path, size_t path_size) {
    if (ctx == NULL || ctx->loaded_plugins == NULL || plugin_name == NULL || plugin_path == NULL || path_size == 0) {
        return -1;
    }
    
    for (size_t i = 0; i < ctx->loaded_plugin_count; i++) {
        if (ctx->loaded_plugins[i].plugin_name != NULL &&
            strcmp(ctx->loaded_plugins[i].plugin_name, plugin_name) == 0 &&
            ctx->loaded_plugins[i].plugin_path != NULL) {
            size_t plugin_path_len = strlen(ctx->loaded_plugins[i].plugin_path);
            if (plugin_path_len < path_size) {
                memcpy(plugin_path, ctx->loaded_plugins[i].plugin_path, plugin_path_len + 1);
                return 0;
            }
            return -1;
        }
    }
    
    return -1;
}

/**
 * @brief 获取插件路径（缓存） / Get plugin path (cached) / Plugin-Pfad abrufen (gecacht)
 * @param plugin_name 插件名称 / Plugin name / Plugin-Name
 * @param plugin_path 输出路径缓冲区 / Output path buffer / Ausgabe-Pfad-Puffer
 * @param path_size 路径缓冲区大小 / Path buffer size / Pfad-Puffergröße
 * @return 成功返回0，失败返回非0 / Returns 0 on success, non-zero on failure / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
 */
int get_plugin_path_cached(const char* plugin_name, char* plugin_path, size_t path_size) {
    if (plugin_name == NULL || plugin_path == NULL || path_size == 0) {
        return -1;
    }
    
    pointer_transfer_context_t* ctx = get_global_context();
    
    if (get_plugin_path_from_cache(ctx, plugin_name, plugin_path, path_size) == 0) {
        return 0;
    }
    
    if (get_plugin_path_from_loaded(ctx, plugin_name, plugin_path, path_size) == 0) {
        extern int add_path_to_cache(pointer_transfer_context_t* ctx, const char* plugin_name, const char* plugin_path);
        add_path_to_cache(ctx, plugin_name, plugin_path);
        return 0;
    }
    
    return -1;
}

