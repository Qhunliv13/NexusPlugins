/**
 * @file type_name.c
 * @brief 获取参数类型名称字符串函数 / Get parameter type name string function / Parametertypnamen-Zeichenfolge abrufen Funktion
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

