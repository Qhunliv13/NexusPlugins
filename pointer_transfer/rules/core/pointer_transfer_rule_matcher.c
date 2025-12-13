/**
 * @file pointer_transfer_rule_matcher.c
 * @brief 规则匹配和应用逻辑 / Rule Matching and Application Logic / Regelabgleich und Anwendungslogik
 */

#include "pointer_transfer_rule_matcher.h"
#include "../broadcast_multicast/broadcast_multicast_matcher.h"
#include "../unicast/unicast_matcher.h"

/**
 * @brief 应用匹配的规则（使用索引） / Apply matched rules (using index) / Passende Regeln anwenden (mit Index)
 */
size_t apply_matched_rules_indexed(const char* source_plugin_name, const char* source_interface_name, 
                                    int source_param_index, void* ptr, size_t start_index, size_t end_index,
                                    size_t* success_count) {
    size_t matched_count = 0;
    
    /* BROADCAST和MULTICAST规则 / BROADCAST and MULTICAST rules / BROADCAST- und MULTICAST-Regeln */
    matched_count += apply_broadcast_multicast_rules_indexed(source_plugin_name, source_interface_name, 
                                                              source_param_index, ptr, start_index, end_index, success_count);
    
    /* UNICAST规则 / UNICAST rules / UNICAST-Regeln */
    matched_count += apply_unicast_rules_indexed(source_plugin_name, source_interface_name, 
                                                  source_param_index, ptr, start_index, end_index, success_count);
    
    return matched_count;
}

/**
 * @brief 应用匹配的规则（线性查找） / Apply matched rules (linear search) / Passende Regeln anwenden (lineare Suche)
 */
size_t apply_matched_rules_linear(const char* source_plugin_name, const char* source_interface_name, 
                                   int source_param_index, void* ptr, size_t* success_count) {
    size_t matched_count = 0;
    
    /* BROADCAST和MULTICAST规则 / BROADCAST and MULTICAST rules / BROADCAST- und MULTICAST-Regeln */
    matched_count += apply_broadcast_multicast_rules_linear(source_plugin_name, source_interface_name, 
                                                              source_param_index, ptr, success_count);
    
    /* UNICAST规则 / UNICAST rules / UNICAST-Regeln */
    matched_count += apply_unicast_rules_linear(source_plugin_name, source_interface_name, 
                                                 source_param_index, ptr, success_count);
    
    return matched_count;
}

