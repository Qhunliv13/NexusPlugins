/**
 * @file type.c
 * @brief 类型检查和转换函数 / Type checking and conversion functions / Typprüfungs- und Konvertierungsfunktionen
 */

#include "pointer_transfer_utils.h"
#include "pointer_transfer_types.h"
#include <string.h>

/**
 * @brief 获取参数类型名称字符串 / Get parameter type name string / Parametertypnamen-Zeichenfolge abrufen
 * @param type 参数类型枚举值 / Parameter type enumeration value / Parametertyp-Aufzählungswert
 * @return 类型名称字符串指针 / Type name string pointer / Typnamen-Zeichenfolgen-Zeiger
 */
const char* get_type_name_string(nxld_param_type_t type) {
    switch (type) {
        case NXLD_PARAM_TYPE_VOID: return "void";
        case NXLD_PARAM_TYPE_INT32: return "int";
        case NXLD_PARAM_TYPE_INT64: return "long";
        case NXLD_PARAM_TYPE_FLOAT: return "float";
        case NXLD_PARAM_TYPE_DOUBLE: return "double";
        case NXLD_PARAM_TYPE_CHAR: return "char";
        case NXLD_PARAM_TYPE_POINTER: return "pointer";
        case NXLD_PARAM_TYPE_STRING: return "string";
        case NXLD_PARAM_TYPE_VARIADIC: return "variadic";
        case NXLD_PARAM_TYPE_ANY: return "any";
        case NXLD_PARAM_TYPE_UNKNOWN: return "unknown";
        default: return "unknown";
    }
}

/**
 * @brief 检查类型兼容性 / Check type compatibility / Typkompatibilität prüfen
 * @param actual_type 实际类型枚举值 / Actual type enumeration value / Tatsächlicher Typ-Aufzählungswert
 * @param expected_type 期望类型枚举值 / Expected type enumeration value / Erwarteter Typ-Aufzählungswert
 * @return 兼容返回1，不兼容返回0 / Returns 1 if compatible, 0 if not / Gibt 1 zurück wenn kompatibel, 0 wenn nicht
 */
int check_type_compatibility(nxld_param_type_t actual_type, nxld_param_type_t expected_type) {
    if (actual_type == expected_type) {
        return 1;
    }
    
    if (expected_type == NXLD_PARAM_TYPE_ANY) {
        return 1;
    }
    
    if (expected_type == NXLD_PARAM_TYPE_POINTER && actual_type == NXLD_PARAM_TYPE_STRING) {
        return 1;
    }
    
    if (expected_type == NXLD_PARAM_TYPE_STRING && actual_type == NXLD_PARAM_TYPE_POINTER) {
        return 1;
    }
    
    return 0;
}

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

