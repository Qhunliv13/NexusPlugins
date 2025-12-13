/**
 * @file config_rules_parser.c
 * @brief 规则键值解析器 / Rule Key-Value Parser / Regel-Schlüssel-Wert-Parser
 */

#include "config_rules_parser.h"
#include "pointer_transfer_types.h"
#include "pointer_transfer_utils.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/**
 * @brief 解析规则键值对 / Parse rule key-value pair / Regel-Schlüssel-Wert-Paar parsen
 */
void parse_rule_key_value(pointer_transfer_rule_t* rule, const char* key, const char* value) {
    if (rule == NULL || key == NULL || value == NULL) {
        return;
    }
    
    if (strcmp(key, "SourcePlugin") == 0) {
        rule->source_plugin = allocate_string(value);
    } else if (strcmp(key, "SourceInterface") == 0) {
        rule->source_interface = allocate_string(value);
    } else if (strcmp(key, "SourceParamIndex") == 0) {
        char* endptr = NULL;
        long parsed_val = strtol(value, &endptr, 10);
        if (endptr != NULL && *endptr == '\0' && parsed_val >= INT_MIN && parsed_val <= INT_MAX) {
            rule->source_param_index = (int)parsed_val;
        }
    } else if (strcmp(key, "TargetPlugin") == 0) {
        rule->target_plugin = allocate_string(value);
    } else if (strcmp(key, "TargetPluginPath") == 0) {
        rule->target_plugin_path = allocate_string(value);
    } else if (strcmp(key, "TargetInterface") == 0) {
        rule->target_interface = allocate_string(value);
    } else if (strcmp(key, "TargetParamIndex") == 0) {
        char* endptr = NULL;
        long parsed_val = strtol(value, &endptr, 10);
        if (endptr != NULL && *endptr == '\0' && parsed_val >= INT_MIN && parsed_val <= INT_MAX) {
            rule->target_param_index = (int)parsed_val;
        }
    } else if (strcmp(key, "TargetParamValue") == 0) {
        rule->target_param_value = allocate_string(value);
    } else if (strcmp(key, "Description") == 0) {
        rule->description = allocate_string(value);
    } else if (strcmp(key, "MulticastGroup") == 0) {
        rule->multicast_group = allocate_string(value);
    } else if (strcmp(key, "TransferMode") == 0) {
        if (strcmp(value, "broadcast") == 0 || strcmp(value, "Broadcast") == 0) {
            rule->transfer_mode = TRANSFER_MODE_BROADCAST;
        } else if (strcmp(value, "multicast") == 0 || strcmp(value, "Multicast") == 0) {
            rule->transfer_mode = TRANSFER_MODE_MULTICAST;
        } else {
            rule->transfer_mode = TRANSFER_MODE_UNICAST;
        }
    } else if (strcmp(key, "Enabled") == 0) {
        rule->enabled = (strcmp(value, "true") == 0 || strcmp(value, "1") == 0) ? 1 : 0;
    } else if (strcmp(key, "Condition") == 0) {
        rule->condition = allocate_string(value);
    } else if (strcmp(key, "CacheSelf") == 0) {
        rule->cache_self = (strcmp(value, "true") == 0 || strcmp(value, "1") == 0) ? 1 : 0;
    } else if (strcmp(key, "SetGroup") == 0) {
        rule->set_group = allocate_string(value);
    }
}

