/**
 * @file type_compatibility.c
 * @brief 检查类型兼容性函数 / Check type compatibility function / Typkompatibilität prüfen Funktion
 */

#include "pointer_transfer_utils.h"
#include "pointer_transfer_types.h"
#include <string.h>

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

