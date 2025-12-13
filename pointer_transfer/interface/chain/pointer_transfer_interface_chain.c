/**
 * @file pointer_transfer_interface_chain.c
 * @brief 调用链管理 / Call Chain Management / Aufrufkette-Verwaltung
 */

#include "pointer_transfer_interface.h"
#include <string.h>
#include <stdio.h>

/**
 * @brief 构建新的调用链 / Build new call chain / Neue Aufrufkette erstellen
 * @param call_chain 原始调用链 / Original call chain / Ursprüngliche Aufrufkette
 * @param call_chain_size 原始调用链大小 / Original call chain size / Ursprüngliche Aufrufketten-Größe
 * @param plugin_name 插件名称 / Plugin name / Plugin-Name
 * @param interface_name 接口名称 / Interface name / Schnittstellenname
 * @param new_call_chain_out 输出新调用链 / Output new call chain / Ausgabe-Neue Aufrufkette
 * @param new_call_chain_size_out 输出新调用链大小 / Output new call chain size / Ausgabe-Neue Aufrufketten-Größe
 */
void build_new_call_chain(const char* call_chain[], size_t call_chain_size,
                          const char* plugin_name, const char* interface_name,
                          const char* new_call_chain_out[64], size_t* new_call_chain_size_out) {
    if (new_call_chain_out == NULL || new_call_chain_size_out == NULL) {
        return;
    }
    
    size_t new_call_chain_size = 0;
    
    if (call_chain_size < 64) {
        for (size_t chain_idx = 0; chain_idx < call_chain_size; chain_idx++) {
            new_call_chain_out[chain_idx] = call_chain[chain_idx];
        }
        new_call_chain_size = call_chain_size;
    } else {
        size_t start_idx = call_chain_size - 63;
        for (size_t chain_idx = start_idx; chain_idx < call_chain_size; chain_idx++) {
            new_call_chain_out[chain_idx - start_idx] = call_chain[chain_idx];
        }
        new_call_chain_size = call_chain_size - start_idx;
    }
    
    if (plugin_name != NULL && interface_name != NULL && new_call_chain_size < 64) {
        static char current_call_identifiers[64][512];
        static size_t current_call_id_index = 0;
        size_t current_id_index = current_call_id_index % 64;
        snprintf(current_call_identifiers[current_id_index], sizeof(current_call_identifiers[0]), "%s.%s", plugin_name, interface_name);
        new_call_chain_out[new_call_chain_size] = current_call_identifiers[current_id_index];
        new_call_chain_size++;
        current_call_id_index++;
    }
    
    *new_call_chain_size_out = new_call_chain_size;
}

