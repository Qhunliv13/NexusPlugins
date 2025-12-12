/**
 * @file pointer_transfer_plugin_loader.c
 * @brief 指针传递插件加载器实现文件 / Pointer Transfer Plugin Loader Implementation File / Zeigerübertragungs-Plugin-Lader-Implementierungsdatei
 */

#include "pointer_transfer_plugin_loader.h"
#include "pointer_transfer_context.h"
#include "pointer_transfer_config.h"
#include "pointer_transfer_utils.h"
#include "pointer_transfer_platform.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

/**
 * @brief 加载目标插件 / Load target plugin / Ziel-Plugin laden
 * @param plugin_name 插件名称 / Plugin name / Plugin-Name
 * @param plugin_path 插件路径 / Plugin path / Plugin-Pfad
 * @return 成功返回插件句柄，失败返回NULL / Returns plugin handle on success, NULL on failure / Gibt Plugin-Handle bei Erfolg zurück, NULL bei Fehler
 */
void* load_target_plugin(const char* plugin_name, const char* plugin_path) {
    if (plugin_name == NULL || plugin_path == NULL) {
        return NULL;
    }
    
    pointer_transfer_context_t* ctx = get_global_context();
    if (ctx->loaded_plugins != NULL) {
        for (size_t i = 0; i < ctx->loaded_plugin_count; i++) {
            if (ctx->loaded_plugins[i].plugin_name != NULL && 
                strcmp(ctx->loaded_plugins[i].plugin_name, plugin_name) == 0) {
                return ctx->loaded_plugins[i].handle;
            }
        }
    }
    
    void* handle = pt_platform_load_library(plugin_path);
    
    if (handle == NULL) {
        internal_log_write("WARNING", "Failed to load target plugin: %s from %s", plugin_name, plugin_path);
        return NULL;
    }
    
    if (ctx->loaded_plugin_count >= ctx->loaded_plugin_capacity) {
        if (expand_loaded_plugins_capacity() != 0) {
            internal_log_write("ERROR", "Failed to expand loaded plugins capacity");
            pt_platform_close_library(handle);
            return NULL;
        }
    }
    
    loaded_plugin_info_t* plugin_info = &ctx->loaded_plugins[ctx->loaded_plugin_count];
    plugin_info->handle = handle;
    plugin_info->plugin_name = allocate_string(plugin_name);
    plugin_info->plugin_path = allocate_string(plugin_path);
    
    if (plugin_info->plugin_name == NULL || plugin_info->plugin_path == NULL) {
        if (plugin_info->plugin_name != NULL) free(plugin_info->plugin_name);
        if (plugin_info->plugin_path != NULL) free(plugin_info->plugin_path);
        internal_log_write("ERROR", "Failed to allocate memory for plugin info");
        pt_platform_close_library(handle);
        return NULL;
    }
    
    ctx->loaded_plugin_count++;
    internal_log_write("INFO", "Loaded target plugin: %s", plugin_name);
    
    return handle;
}

/**
 * @brief 链式加载插件.nxpt文件（内部函数） / Chain load plugin .nxpt file (internal function) / Plugin-.nxpt-Datei kettenweise laden (interne Funktion)
 * @param plugin_name 插件名称 / Plugin name / Plugin-Name
 * @param plugin_path 插件路径 / Plugin path / Plugin-Pfad
 * @param loading_stack 加载栈数组 / Loading stack array / Ladestapel-Array
 * @param stack_size 加载栈大小 / Loading stack size / Ladestapel-Größe
 * @return 成功返回0，失败返回非0 / Returns 0 on success, non-zero on failure / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
 */
static int chain_load_plugin_nxpt_internal(const char* plugin_name, const char* plugin_path, const char* loading_stack[], size_t stack_size) {
    if (plugin_name == NULL || plugin_path == NULL) {
        return -1;
    }
    
    if (is_nxpt_loaded(plugin_name)) {
        return 0;
    }
    
    /* 检测循环依赖 / Detect circular dependency / Zirkuläre Abhängigkeit erkennen */
    for (size_t i = 0; i < stack_size; i++) {
        if (loading_stack[i] != NULL && strcmp(loading_stack[i], plugin_name) == 0) {
            internal_log_write("WARNING", "Circular dependency detected in plugin loading chain: ... -> %s -> ... -> %s", 
                         loading_stack[0] != NULL ? loading_stack[0] : "unknown", plugin_name);
            break;
        }
    }
    
    char nxpt_path[4096];
    if (build_nxpt_path(plugin_path, nxpt_path, sizeof(nxpt_path)) != 0) {
        internal_log_write("WARNING", "Failed to build .nxpt path for plugin %s", plugin_name);
        return -1;
    }
    
    internal_log_write("INFO", "Chain loading .nxpt file for plugin %s: %s", plugin_name, nxpt_path);
    
    pointer_transfer_context_t* ctx = get_global_context();
    size_t rule_count_before = ctx->rule_count;
    
    if (load_transfer_rules(nxpt_path) == 0) {
        mark_nxpt_loaded(plugin_name, nxpt_path);
        
        /* 构建加载栈 / Build loading stack / Ladestapel erstellen */
        const char* new_loading_stack[32];
        size_t new_stack_size = 0;
        if (stack_size < sizeof(new_loading_stack) / sizeof(new_loading_stack[0])) {
            for (size_t i = 0; i < stack_size; i++) {
                new_loading_stack[i] = loading_stack[i];
            }
            new_stack_size = stack_size;
        } else {
            /* 加载栈容量超出限制 / Loading stack capacity exceeded / Ladestapel-Kapazität überschritten */
            size_t start_idx = stack_size - (sizeof(new_loading_stack) / sizeof(new_loading_stack[0]) - 1);
            for (size_t i = start_idx; i < stack_size; i++) {
                new_loading_stack[i - start_idx] = loading_stack[i];
            }
            new_stack_size = stack_size - start_idx;
        }
        
        if (new_stack_size < sizeof(new_loading_stack) / sizeof(new_loading_stack[0])) {
            static char plugin_names[32][256];
            static size_t plugin_name_index = 0;
            size_t current_index = plugin_name_index % (sizeof(plugin_names) / sizeof(plugin_names[0]));
            snprintf(plugin_names[current_index], sizeof(plugin_names[0]), "%s", plugin_name);
            new_loading_stack[new_stack_size] = plugin_names[current_index];
            new_stack_size++;
            plugin_name_index++;
        }
        
        for (size_t i = rule_count_before; i < ctx->rule_count; i++) {
            pointer_transfer_rule_t* rule = &ctx->rules[i];
            if (rule->enabled && rule->target_plugin != NULL && rule->target_plugin_path != NULL) {
                if (!is_nxpt_loaded(rule->target_plugin)) {
                    chain_load_plugin_nxpt_internal(rule->target_plugin, rule->target_plugin_path, new_loading_stack, new_stack_size);
                }
            }
        }
        
        return 0;
    } else {
        internal_log_write("WARNING", "Failed to load .nxpt file for plugin %s: %s", plugin_name, nxpt_path);
        return -1;
    }
}

/**
 * @brief 链式加载插件的.nxpt文件 / Chain load plugin .nxpt file / Plugin-.nxpt-Datei kettenweise laden
 * @param plugin_name 插件名称 / Plugin name / Plugin-Name
 * @param plugin_path 插件路径 / Plugin path / Plugin-Pfad
 * @return 成功返回0，失败返回非0 / Returns 0 on success, non-zero on failure / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
 */
int chain_load_plugin_nxpt(const char* plugin_name, const char* plugin_path) {
    const char* initial_stack[1] = {NULL};
    size_t initial_stack_size = 0;
    
    if (plugin_name != NULL) {
        static char initial_plugin_name[256];
        snprintf(initial_plugin_name, sizeof(initial_plugin_name), "%s", plugin_name);
        initial_stack[0] = initial_plugin_name;
        initial_stack_size = 1;
    }
    
    return chain_load_plugin_nxpt_internal(plugin_name, plugin_path, initial_stack, initial_stack_size);
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
    
    if (ctx->loaded_plugins != NULL) {
        for (size_t i = 0; i < ctx->loaded_plugin_count; i++) {
            if (ctx->loaded_plugins[i].plugin_name != NULL &&
                strcmp(ctx->loaded_plugins[i].plugin_name, plugin_name) == 0 &&
                ctx->loaded_plugins[i].plugin_path != NULL) {
                size_t plugin_path_len = strlen(ctx->loaded_plugins[i].plugin_path);
                if (plugin_path_len < path_size) {
                    memcpy(plugin_path, ctx->loaded_plugins[i].plugin_path, plugin_path_len + 1);
                    
                    plugin_path_cache_entry_t* cache_entry = NULL;
                    if (ctx->path_cache == NULL || ctx->path_cache_count >= ctx->path_cache_capacity) {
                        /* 初始容量：8，增长因子：2 / Initial capacity: 8, growth factor: 2 / Anfangskapazität: 8, Wachstumsfaktor: 2 */
                        size_t new_capacity = ctx->path_cache_capacity == 0 ? 8 : ctx->path_cache_capacity * 2;
                        plugin_path_cache_entry_t* new_cache = (plugin_path_cache_entry_t*)realloc(ctx->path_cache, new_capacity * sizeof(plugin_path_cache_entry_t));
                        if (new_cache == NULL) {
                            return 0;
                        }
                        ctx->path_cache = new_cache;
                        memset(&ctx->path_cache[ctx->path_cache_count], 0, (new_capacity - ctx->path_cache_count) * sizeof(plugin_path_cache_entry_t));
                        ctx->path_cache_capacity = new_capacity;
                    }
                    
                    cache_entry = &ctx->path_cache[ctx->path_cache_count];
                    cache_entry->plugin_name = allocate_string(plugin_name);
                    cache_entry->plugin_path = allocate_string(plugin_path);
                    
                    if (cache_entry->plugin_name != NULL && cache_entry->plugin_path != NULL) {
                        ctx->path_cache_count++;
                    } else {
                        if (cache_entry->plugin_name != NULL) free(cache_entry->plugin_name);
                        if (cache_entry->plugin_path != NULL) free(cache_entry->plugin_path);
                    }
                    
                    return 0;
                }
            }
        }
    }
    
    return -1;
}

