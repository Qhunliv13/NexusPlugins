/**
 * @file config_rules_merger.h
 * @brief 规则合并器接口 / Rules Merger Interface / Regeln-Zusammenführer-Schnittstelle
 */

#ifndef CONFIG_RULES_MERGER_H
#define CONFIG_RULES_MERGER_H

#include "pointer_transfer_types.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 合并规则到上下文 / Merge rules to context / Regeln in Kontext zusammenführen
 * @param temp_rules 临时规则数组 / Temporary rules array / Temporäres Regeln-Array
 * @param temp_rules_count 临时规则数量 / Temporary rules count / Anzahl der temporären Regeln
 * @param max_seen_index 最大看到的索引 / Maximum seen index / Maximaler gesehener Index
 * @return 成功返回0，失败返回错误码 / Returns 0 on success, error code on failure / Gibt 0 bei Erfolg zurück, Fehlercode bei Fehler
 */
int merge_rules_to_context(pointer_transfer_rule_t* temp_rules, size_t temp_rules_count, 
                           int max_seen_index);

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_RULES_MERGER_H */

