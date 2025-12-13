/**
 * @file pointer_transfer_rule_matcher.h
 * @brief 规则匹配和应用逻辑头文件 / Rule Matching and Application Logic Header / Regelabgleich und Anwendungslogik-Header
 */

#ifndef POINTER_TRANSFER_RULE_MATCHER_H
#define POINTER_TRANSFER_RULE_MATCHER_H

#include <stddef.h>

/**
 * @brief 应用匹配的规则（使用索引） / Apply matched rules (using index) / Passende Regeln anwenden (mit Index)
 */
size_t apply_matched_rules_indexed(const char* source_plugin_name, const char* source_interface_name, 
                                    int source_param_index, void* ptr, size_t start_index, size_t end_index,
                                    size_t* success_count);

/**
 * @brief 应用匹配的规则（线性查找） / Apply matched rules (linear search) / Passende Regeln anwenden (lineare Suche)
 */
size_t apply_matched_rules_linear(const char* source_plugin_name, const char* source_interface_name, 
                                   int source_param_index, void* ptr, size_t* success_count);

#endif /* POINTER_TRANSFER_RULE_MATCHER_H */

