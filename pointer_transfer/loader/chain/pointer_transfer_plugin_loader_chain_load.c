/**
 * @file loader/chain/pointer_transfer_plugin_loader_chain_load.c
 * @brief 链式加载实现 / Chain Loading Implementation / Kettenlade-Implementierung
 */

#include "pointer_transfer_plugin_loader.h"
#include "pointer_transfer_context.h"
#include "pointer_transfer_config.h"
#include "pointer_transfer_utils.h"
#include <string.h>
#include <stdio.h>

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
    
    extern int detect_loading_cycle(const char* plugin_name, const char* loading_stack[], size_t stack_size);
    if (detect_loading_cycle(plugin_name, loading_stack, stack_size)) {
        return -1;
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
        
        extern void build_new_loading_stack(const char* loading_stack[], size_t stack_size, const char* plugin_name,
                                          const char* new_loading_stack_out[32], size_t* new_stack_size_out);
        const char* new_loading_stack[32];
        size_t new_stack_size = 0;
        build_new_loading_stack(loading_stack, stack_size, plugin_name, new_loading_stack, &new_stack_size);
        
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

