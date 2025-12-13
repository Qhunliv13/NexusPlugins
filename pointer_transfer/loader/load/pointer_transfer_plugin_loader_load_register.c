/**
 * @file loader/load/pointer_transfer_plugin_loader_load_register.c
 * @brief 插件注册实现 / Plugin Register Implementation / Plugin-Registrierung-Implementierung
 */

#include "pointer_transfer_plugin_loader.h"
#include "pointer_transfer_context.h"
#include "pointer_transfer_utils.h"
#include <stdlib.h>
#include <string.h>

/**
 * @brief 注册已加载的插件 / Register loaded plugin / Geladenes Plugin registrieren
 * @param ctx 上下文 / Context / Kontext
 * @param plugin_name 插件名称 / Plugin name / Plugin-Name
 * @param plugin_path 插件路径 / Plugin path / Plugin-Pfad
 * @param handle 插件句柄 / Plugin handle / Plugin-Handle
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int register_loaded_plugin(pointer_transfer_context_t* ctx, const char* plugin_name, const char* plugin_path, void* handle) {
    if (ctx == NULL || plugin_name == NULL || plugin_path == NULL || handle == NULL) {
        return -1;
    }
    
    if (ctx->loaded_plugin_count >= ctx->loaded_plugin_capacity) {
        if (expand_loaded_plugins_capacity() != 0) {
            internal_log_write("ERROR", "Failed to expand loaded plugins capacity");
            return -1;
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
        return -1;
    }
    
    ctx->loaded_plugin_count++;
    internal_log_write("INFO", "Loaded target plugin: %s", plugin_name);
    
    return 0;
}

