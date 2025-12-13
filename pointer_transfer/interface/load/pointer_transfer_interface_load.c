/**
 * @file pointer_transfer_interface_load.c
 * @brief 插件加载和函数获取 / Plugin Loading and Function Retrieval / Plugin-Laden und Funktionsabruf
 */

#include "pointer_transfer_interface.h"
#include "pointer_transfer_context.h"
#include "pointer_transfer_config.h"
#include "pointer_transfer_plugin_loader.h"
#include "pointer_transfer_platform.h"
#include "pointer_transfer_utils.h"
#include <string.h>

/**
 * @brief 加载目标插件并获取函数指针 / Load target plugin and get function pointer / Ziel-Plugin laden und Funktionszeiger abrufen
 * @param rule 传递规则 / Transfer rule / Übertragungsregel
 * @param handle_out 输出插件句柄 / Output plugin handle / Ausgabe-Plugin-Handle
 * @param func_ptr_out 输出函数指针 / Output function pointer / Ausgabe-Funktionszeiger
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int load_plugin_and_get_function(const pointer_transfer_rule_t* rule, void** handle_out, void** func_ptr_out) {
    if (rule == NULL || rule->target_plugin == NULL || rule->target_interface == NULL || 
        handle_out == NULL || func_ptr_out == NULL) {
        return -1;
    }
    
    if (rule->target_plugin_path == NULL || strlen(rule->target_plugin_path) == 0) {
        internal_log_write("WARNING", "No plugin path configured for %s", rule->target_plugin);
        return -1;
    }
    
    pointer_transfer_context_t* ctx = get_global_context();
    if (!is_nxpt_loaded(rule->target_plugin)) {
        chain_load_plugin_nxpt(rule->target_plugin, rule->target_plugin_path);
    }
    
    const char* plugin_path = rule->target_plugin_path;
    internal_log_write("INFO", "Using configured plugin path for %s: %s", rule->target_plugin, plugin_path);
    
    void* handle = load_target_plugin(rule->target_plugin, plugin_path);
    if (handle == NULL) {
        internal_log_write("ERROR", "Failed to load target plugin: %s from %s", rule->target_plugin, plugin_path);
        return -1;
    }
    
    void* func_ptr = pt_platform_get_symbol(handle, rule->target_interface);
    if (func_ptr == NULL) {
        internal_log_write("ERROR", "Function %s not found in plugin %s", rule->target_interface, rule->target_plugin);
        internal_log_write("ERROR", "Plugin %s does not export required function %s", 
                          rule->target_plugin, rule->target_interface);
        return -1;
    }
    
    *handle_out = handle;
    *func_ptr_out = func_ptr;
    return 0;
}

