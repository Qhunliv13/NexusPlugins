/**
 * @file pointer_transfer_interface_cycle.c
 * @brief 接口调用循环检测 / Interface Call Cycle Detection / Schnittstellenaufruf-Zykluserkennung
 */

#include "pointer_transfer_interface.h"
#include <string.h>
#include <stdio.h>
#include "pointer_transfer_utils.h"

/**
 * @brief 检测调用循环 / Detect call cycle / Aufrufzyklus erkennen
 * @param rule 传递规则 / Transfer rule / Übertragungsregel
 * @param call_chain 调用链 / Call chain / Aufrufkette
 * @param call_chain_size 调用链大小 / Call chain size / Aufrufketten-Größe
 * @return 检测到循环返回-1，否则返回0 / Returns -1 if cycle detected, 0 otherwise / Gibt -1 zurück, wenn Zyklus erkannt, sonst 0
 */
int detect_call_cycle(const pointer_transfer_rule_t* rule, const char* call_chain[], size_t call_chain_size) {
    if (rule == NULL || rule->target_plugin == NULL || rule->target_interface == NULL) {
        return -1;
    }
    
    char current_call[512];
    snprintf(current_call, sizeof(current_call), "%s.%s", rule->target_plugin, rule->target_interface);
    
    if (call_chain_size > 0) {
        for (size_t i = 0; i < call_chain_size; i++) {
            if (call_chain[i] != NULL && strcmp(call_chain[i], current_call) == 0) {
                internal_log_write("WARNING", "Call cycle detected: %s -> ... -> %s", call_chain[0] != NULL ? call_chain[0] : "unknown", current_call);
                return -1;
            }
        }
    }
    
    return 0;
}

/**
 * @brief 检查递归深度 / Check recursion depth / Rekursionstiefe prüfen
 * @param recursion_depth 递归深度 / Recursion depth / Rekursionstiefe
 */
void check_recursion_depth(int recursion_depth) {
    if (recursion_depth > 32) {
        internal_log_write("WARNING", "High recursion depth detected in call_target_plugin_interface (depth=%d)", recursion_depth);
    }
}

