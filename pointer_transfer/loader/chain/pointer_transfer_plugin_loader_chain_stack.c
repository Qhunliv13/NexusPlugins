/**
 * @file loader/chain/pointer_transfer_plugin_loader_chain_stack.c
 * @brief 链式加载栈操作实现 / Chain Loading Stack Operations Implementation / Kettenlade-Stapeloperationen-Implementierung
 */

#include "pointer_transfer_plugin_loader.h"
#include <string.h>
#include <stdio.h>

/**
 * @brief 构建新的加载栈 / Build new loading stack / Neuen Ladestapel erstellen
 * @param loading_stack 原始加载栈 / Original loading stack / Ursprünglicher Ladestapel
 * @param stack_size 原始加载栈大小 / Original stack size / Ursprüngliche Ladestapel-Größe
 * @param plugin_name 插件名称 / Plugin name / Plugin-Name
 * @param new_loading_stack_out 输出新加载栈 / Output new loading stack / Ausgabe-Neuer Ladestapel
 * @param new_stack_size_out 输出新加载栈大小 / Output new stack size / Ausgabe-Neue Ladestapel-Größe
 */
void build_new_loading_stack(const char* loading_stack[], size_t stack_size, const char* plugin_name,
                              const char* new_loading_stack_out[32], size_t* new_stack_size_out) {
    if (new_loading_stack_out == NULL || new_stack_size_out == NULL) {
        return;
    }
    
    size_t new_stack_size = 0;
    
    if (stack_size < 32) {
        for (size_t i = 0; i < stack_size; i++) {
            new_loading_stack_out[i] = loading_stack[i];
        }
        new_stack_size = stack_size;
    } else {
        size_t start_idx = stack_size - 31;
        for (size_t i = start_idx; i < stack_size; i++) {
            new_loading_stack_out[i - start_idx] = loading_stack[i];
        }
        new_stack_size = stack_size - start_idx;
    }
    
    if (plugin_name != NULL && new_stack_size < 32) {
        static char plugin_names[32][256];
        static size_t plugin_name_index = 0;
        size_t current_index = plugin_name_index % 32;
        snprintf(plugin_names[current_index], sizeof(plugin_names[0]), "%s", plugin_name);
        new_loading_stack_out[new_stack_size] = plugin_names[current_index];
        new_stack_size++;
        plugin_name_index++;
    }
    
    *new_stack_size_out = new_stack_size;
}

