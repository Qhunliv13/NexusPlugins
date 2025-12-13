/**
 * @file rule.c
 * @brief 规则内存管理函数 / Rule memory management functions / Regel-Speicherverwaltungsfunktionen
 */

#include "pointer_transfer_utils.h"
#include "pointer_transfer_types.h"
#include <stdlib.h>

/**
 * @brief 释放单个传递规则结构体的内存 / Free memory of single transfer rule structure / Speicher einer einzelnen Übertragungsregel-Struktur freigeben
 * @param rule 规则结构体指针 / Rule structure pointer / Regel-Struktur-Zeiger
 */
void free_single_rule(pointer_transfer_rule_t* rule) {
    if (rule == NULL) {
        return;
    }
    
    if (rule->source_plugin != NULL) {
        free(rule->source_plugin);
        rule->source_plugin = NULL;
    }
    if (rule->source_interface != NULL) {
        free(rule->source_interface);
        rule->source_interface = NULL;
    }
    if (rule->target_plugin != NULL) {
        free(rule->target_plugin);
        rule->target_plugin = NULL;
    }
    if (rule->target_plugin_path != NULL) {
        free(rule->target_plugin_path);
        rule->target_plugin_path = NULL;
    }
    if (rule->target_interface != NULL) {
        free(rule->target_interface);
        rule->target_interface = NULL;
    }
    if (rule->target_param_value != NULL) {
        free(rule->target_param_value);
        rule->target_param_value = NULL;
    }
    if (rule->description != NULL) {
        free(rule->description);
        rule->description = NULL;
    }
    if (rule->multicast_group != NULL) {
        free(rule->multicast_group);
        rule->multicast_group = NULL;
    }
    if (rule->condition != NULL) {
        free(rule->condition);
        rule->condition = NULL;
    }
    if (rule->set_group != NULL) {
        free(rule->set_group);
        rule->set_group = NULL;
    }
}

