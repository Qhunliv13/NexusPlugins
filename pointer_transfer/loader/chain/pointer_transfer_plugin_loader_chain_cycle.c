/**
 * @file loader/chain/pointer_transfer_plugin_loader_chain_cycle.c
 * @brief 链式加载循环检测实现 / Chain Loading Cycle Detection Implementation / Kettenlade-Zykluserkennung-Implementierung
 */

#include "pointer_transfer_plugin_loader.h"
#include "pointer_transfer_utils.h"
#include <string.h>

/**
 * @brief 检测循环依赖 / Detect circular dependency / Zirkuläre Abhängigkeit erkennen
 * @param plugin_name 插件名称 / Plugin name / Plugin-Name
 * @param loading_stack 加载栈数组 / Loading stack array / Ladestapel-Array
 * @param stack_size 加载栈大小 / Loading stack size / Ladestapel-Größe
 * @return 检测到循环返回1，否则返回0 / Returns 1 if cycle detected, 0 otherwise / Gibt 1 zurück, wenn Zyklus erkannt, sonst 0
 */
int detect_loading_cycle(const char* plugin_name, const char* loading_stack[], size_t stack_size) {
    if (plugin_name == NULL || loading_stack == NULL) {
        return 0;
    }
    
    for (size_t i = 0; i < stack_size; i++) {
        if (loading_stack[i] != NULL && strcmp(loading_stack[i], plugin_name) == 0) {
            internal_log_write("WARNING", "Circular dependency detected in plugin loading chain: ... -> %s -> ... -> %s", 
                         loading_stack[0] != NULL ? loading_stack[0] : "unknown", plugin_name);
            return 1;
        }
    }
    
    return 0;
}

