/**
 * @file broadcast_multicast_matcher.h
 * @brief BROADCAST和MULTICAST规则匹配器 / BROADCAST and MULTICAST Rule Matcher / BROADCAST- und MULTICAST-Regelabgleich
 */

#ifndef BROADCAST_MULTICAST_MATCHER_H
#define BROADCAST_MULTICAST_MATCHER_H

#include <stddef.h>

/**
 * @brief 应用BROADCAST和MULTICAST规则（使用索引） / Apply BROADCAST and MULTICAST rules (using index) / BROADCAST- und MULTICAST-Regeln anwenden (mit Index)
 * @param source_plugin_name 源插件名称 / Source plugin name / Quell-Plugin-Name
 * @param source_interface_name 源接口名称 / Source interface name / Quell-Schnittstellenname
 * @param source_param_index 源参数索引 / Source parameter index / Quell-Parameterindex
 * @param ptr 指针 / Pointer / Zeiger
 * @param start_index 起始索引 / Start index / Startindex
 * @param end_index 结束索引 / End index / Endindex
 * @param success_count 成功计数指针 / Success count pointer / Erfolgszähler-Zeiger
 * @return 匹配的规则数量 / Number of matched rules / Anzahl der abgeglichenen Regeln
 */
size_t apply_broadcast_multicast_rules_indexed(const char* source_plugin_name, const char* source_interface_name, 
                                                int source_param_index, void* ptr, size_t start_index, size_t end_index,
                                                size_t* success_count);

/**
 * @brief 应用BROADCAST和MULTICAST规则（线性查找） / Apply BROADCAST and MULTICAST rules (linear search) / BROADCAST- und MULTICAST-Regeln anwenden (lineare Suche)
 * @param source_plugin_name 源插件名称 / Source plugin name / Quell-Plugin-Name
 * @param source_interface_name 源接口名称 / Source interface name / Quell-Schnittstellenname
 * @param source_param_index 源参数索引 / Source parameter index / Quell-Parameterindex
 * @param ptr 指针 / Pointer / Zeiger
 * @param success_count 成功计数指针 / Success count pointer / Erfolgszähler-Zeiger
 * @return 匹配的规则数量 / Number of matched rules / Anzahl der abgeglichenen Regeln
 */
size_t apply_broadcast_multicast_rules_linear(const char* source_plugin_name, const char* source_interface_name, 
                                               int source_param_index, void* ptr, size_t* success_count);

#endif /* BROADCAST_MULTICAST_MATCHER_H */

