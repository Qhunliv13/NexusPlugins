/**
 * @file config_parser_common.c
 * @brief 配置解析公共函数 / Configuration Parser Common Functions / Konfigurationsparser-Gemeinsame Funktionen
 */

#include "config_parser_common.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/**
 * @brief 解析布尔值配置 / Parse boolean configuration / Boolesche Konfiguration parsen
 */
int parse_boolean_value(const char* value) {
    if (value == NULL) {
        return 0;
    }
    
    if (strcmp(value, "1") == 0 || strcmp(value, "true") == 0 || 
        strcmp(value, "True") == 0 || strcmp(value, "TRUE") == 0 ||
        strcmp(value, "yes") == 0 || strcmp(value, "Yes") == 0 || 
        strcmp(value, "YES") == 0 ||
        strcmp(value, "on") == 0 || strcmp(value, "On") == 0 || 
        strcmp(value, "ON") == 0) {
        return 1;
    } else if (strcmp(value, "0") == 0 || strcmp(value, "false") == 0 || 
               strcmp(value, "False") == 0 || strcmp(value, "FALSE") == 0 ||
               strcmp(value, "no") == 0 || strcmp(value, "No") == 0 || 
               strcmp(value, "NO") == 0 ||
               strcmp(value, "off") == 0 || strcmp(value, "Off") == 0 || 
               strcmp(value, "OFF") == 0) {
        return 0;
    } else {
        /* 尝试解析为整数 / Try to parse as integer / Als Ganzzahl parsen versuchen */
        char* endptr = NULL;
        long int_val = strtol(value, &endptr, 10);
        if (endptr != NULL && *endptr == '\0') {
            return (int_val != 0) ? 1 : 0;
        }
        return 0;
    }
}

