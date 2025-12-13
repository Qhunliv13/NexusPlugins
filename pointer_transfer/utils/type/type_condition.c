/**
 * @file type_condition.c
 * @brief 检查传递条件函数 / Check transfer condition function / Übertragungsbedingung prüfen Funktion
 */

#include "pointer_transfer_utils.h"
#include "pointer_transfer_types.h"
#include <string.h>

/**
 * @brief 检查传递条件 / Check transfer condition / Übertragungsbedingung prüfen
 * @param condition 条件字符串 / Condition string / Bedingungszeichenfolge
 * @param param_value 参数值指针 / Parameter value pointer / Parameterwert-Zeiger
 * @return 满足条件返回1，否则返回0 / Returns 1 if condition is met, 0 otherwise / Gibt 1 zurück, wenn Bedingung erfüllt ist, sonst 0
 */
int check_condition(const char* condition, void* param_value) {
    if (condition == NULL || strlen(condition) == 0) {
        return 1;
    }
    
    if (strcmp(condition, "not_null") == 0) {
        return (param_value != NULL) ? 1 : 0;
    }
    
    if (strcmp(condition, "null") == 0) {
        return (param_value == NULL) ? 1 : 0;
    }
    
    if (strcmp(condition, ">0") == 0) {
        if (param_value == NULL) return 0;
        int* int_val = (int*)param_value;
        return (*int_val > 0) ? 1 : 0;
    }
    
    if (strcmp(condition, "<0") == 0) {
        if (param_value == NULL) return 0;
        int* int_val = (int*)param_value;
        return (*int_val < 0) ? 1 : 0;
    }
    
    if (strcmp(condition, "==0") == 0 || strcmp(condition, "=0") == 0) {
        if (param_value == NULL) return 1;
        int* int_val = (int*)param_value;
        return (*int_val == 0) ? 1 : 0;
    }
    
    if (strcmp(condition, "!=0") == 0) {
        if (param_value == NULL) return 0;
        int* int_val = (int*)param_value;
        return (*int_val != 0) ? 1 : 0;
    }
    
    return 1;
}

