/**
 * @file loader/load/pointer_transfer_plugin_loader_load_find.c
 * @brief 插件查找实现 / Plugin Find Implementation / Plugin-Suche-Implementierung
 */

#include "pointer_transfer_plugin_loader.h"
#include "pointer_transfer_context.h"
#include <string.h>

/**
 * @brief 查找已加载的插件 / Find loaded plugin / Geladenes Plugin suchen
 * @param ctx 上下文 / Context / Kontext
 * @param plugin_name 插件名称 / Plugin name / Plugin-Name
 * @return 找到返回插件句柄，否则返回NULL / Returns plugin handle if found, NULL otherwise / Gibt Plugin-Handle zurück, wenn gefunden, sonst NULL
 */
void* find_loaded_plugin(pointer_transfer_context_t* ctx, const char* plugin_name) {
    if (ctx == NULL || ctx->loaded_plugins == NULL || plugin_name == NULL) {
        return NULL;
    }
    
    for (size_t i = 0; i < ctx->loaded_plugin_count; i++) {
        if (ctx->loaded_plugins[i].plugin_name != NULL && 
            strcmp(ctx->loaded_plugins[i].plugin_name, plugin_name) == 0) {
            return ctx->loaded_plugins[i].handle;
        }
    }
    
    return NULL;
}

