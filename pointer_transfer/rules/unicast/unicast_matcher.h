/**
 * @file unicast_matcher.h
 * @brief UNICAST规则匹配器 / UNICAST Rule Matcher / UNICAST-Regelabgleich
 */

#ifndef UNICAST_MATCHER_H
#define UNICAST_MATCHER_H

#include <stddef.h>

/**
 * @brief 应用UNICAST规则（使用索引） / Apply UNICAST rules (using index) / UNICAST-Regeln anwenden (mit Index)
 * @param source_plugin_name 源插件名称 / Source plugin name / Quell-Plugin-Name
 * @param source_interface_name 源接口名称 / Source interface name / Quell-Schnittstellenname
 * @param source_param_index 源参数索引 / Source parameter index / Quell-Parameterindex
 * @param ptr 指针 / Pointer / Zeiger
 * @param start_index 起始索引 / Start index / Startindex
 * @param end_index 结束索引 / End index / Endindex
 * @param success_count 成功计数指针 / Success count pointer / Erfolgszähler-Zeiger
 * @return 匹配的规则数量 / Number of matched rules / Anzahl der abgeglichenen Regeln
 */
size_t apply_unicast_rules_indexed(const char* source_plugin_name, const char* source_interface_name, 
                                    int source_param_index, void* ptr, size_t start_index, size_t end_index,
                                    size_t* success_count);

/**
 * @brief 应用UNICAST规则（线性查找） / Apply UNICAST rules (linear search) / UNICAST-Regeln anwenden (lineare Suche)
 * @param source_plugin_name 源插件名称 / Source plugin name / Quell-Plugin-Name
 * @param source_interface_name 源接口名称 / Source interface name / Quell-Schnittstellenname
 * @param source_param_index 源参数索引 / Source parameter index / Quell-Parameterindex
 * @param ptr 指针 / Pointer / Zeiger
 * @param success_count 成功计数指针 / Success count pointer / Erfolgszähler-Zeiger
 * @return 匹配的规则数量 / Number of matched rules / Anzahl der abgeglichenen Regeln
 */
size_t apply_unicast_rules_linear(const char* source_plugin_name, const char* source_interface_name, 
                                   int source_param_index, void* ptr, size_t* success_count);

#endif /* UNICAST_MATCHER_H */

