/**
 * @file loader/load/pointer_transfer_plugin_loader_load_target.c
 * @brief 目标插件加载实现 / Target Plugin Loading Implementation / Ziel-Plugin-Lade-Implementierung
 */

#include "pointer_transfer_plugin_loader.h"
#include "pointer_transfer_context.h"
#include "pointer_transfer_platform.h"

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
    extern void* find_loaded_plugin(pointer_transfer_context_t* ctx, const char* plugin_name);
    void* handle = find_loaded_plugin(ctx, plugin_name);
    if (handle != NULL) {
        return handle;
    }
    
    handle = pt_platform_load_library(plugin_path);
    if (handle == NULL) {
        internal_log_write("WARNING", "Failed to load target plugin: %s from %s", plugin_name, plugin_path);
        return NULL;
    }
    
    extern int register_loaded_plugin(pointer_transfer_context_t* ctx, const char* plugin_name, const char* plugin_path, void* handle);
    if (register_loaded_plugin(ctx, plugin_name, plugin_path, handle) != 0) {
        pt_platform_close_library(handle);
        return NULL;
    }
    
    return handle;
}

