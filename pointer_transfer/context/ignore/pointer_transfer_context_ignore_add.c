/**
 * @file pointer_transfer_context_ignore_add.c
 * @brief 忽略插件列表添加操作 / Ignore Plugin List Add Operations / Ignorier-Plugin-Listen-Hinzufügungsoperationen
 */

#include "pointer_transfer_context.h"
#include "pointer_transfer_utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

/**
 * @brief 添加忽略插件路径 / Add ignored plugin path / Ignorierten Plugin-Pfad hinzufügen
 * @param plugin_path 插件路径（相对路径） / Plugin path (relative path) / Plugin-Pfad (relativer Pfad)
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int add_ignore_plugin_path(const char* plugin_path) {
    if (plugin_path == NULL) {
        return -1;
    }
    
    pointer_transfer_context_t* ctx = get_global_context();
    if (ctx == NULL) {
        return -1;
    }
    
    /* 检查是否已存在 / Check if already exists / Prüfen, ob bereits vorhanden */
    for (size_t i = 0; i < ctx->ignore_plugin_count; i++) {
        if (ctx->ignore_plugins[i] != NULL && strcmp(ctx->ignore_plugins[i], plugin_path) == 0) {
            /* 已存在，返回成功 / Already exists, return success / Bereits vorhanden, Erfolg zurückgeben */
            return 0;
        }
    }
    
    /* 检查是否需要扩容 / Check if capacity expansion is needed / Prüfen, ob Kapazitätserweiterung erforderlich ist */
    if (ctx->ignore_plugin_count >= ctx->ignore_plugin_capacity) {
        size_t new_capacity = ctx->ignore_plugin_capacity == 0 ? 8 : ctx->ignore_plugin_capacity * 2;
        
        if (new_capacity < ctx->ignore_plugin_capacity || new_capacity > SIZE_MAX / sizeof(char*)) {
            internal_log_write("ERROR", "add_ignore_plugin_path: capacity overflow detected (current=%zu, new=%zu)", 
                              ctx->ignore_plugin_capacity, new_capacity);
            return -1;
        }
        
        char** new_plugins = (char**)realloc(ctx->ignore_plugins, new_capacity * sizeof(char*));
        if (new_plugins == NULL) {
            internal_log_write("ERROR", "add_ignore_plugin_path: failed to allocate memory for ignore plugins array (new_capacity=%zu)", new_capacity);
            return -1;
        }
        
        memset(new_plugins + ctx->ignore_plugin_count, 0, (new_capacity - ctx->ignore_plugin_count) * sizeof(char*));
        ctx->ignore_plugins = new_plugins;
        ctx->ignore_plugin_capacity = new_capacity;
    }
    
    /* 分配并复制路径字符串 / Allocate and copy path string / Pfadzeichenfolge zuweisen und kopieren */
    char* path_copy = allocate_string(plugin_path);
    if (path_copy == NULL) {
        internal_log_write("ERROR", "add_ignore_plugin_path: failed to allocate memory for plugin path: %s", plugin_path);
        return -1;
    }
    
    ctx->ignore_plugins[ctx->ignore_plugin_count] = path_copy;
    ctx->ignore_plugin_count++;
    
    internal_log_write("INFO", "Added ignored plugin path: %s", plugin_path);
    return 0;
}

/**
 * @brief 初始化默认忽略插件列表 / Initialize default ignored plugins list / Standard-Ignorierliste für Plugins initialisieren
 * @note 当前不默认忽略任何插件，所有忽略项需通过配置文件指定 / Currently does not ignore any plugins by default, all ignore items must be specified via config file / Ignoriert derzeit standardmäßig keine Plugins, alle Ignorier-Einträge müssen über Konfigurationsdatei angegeben werden
 */
void initialize_default_ignore_plugins(void) {
    /* 当前不默认忽略任何插件 / Currently does not ignore any plugins by default / Ignoriert derzeit standardmäßig keine Plugins */
    /* 所有忽略项需通过配置文件中的 IgnorePlugins 配置项指定 / All ignore items must be specified via IgnorePlugins config item / Alle Ignorier-Einträge müssen über IgnorePlugins-Konfigurationselement angegeben werden */
}

