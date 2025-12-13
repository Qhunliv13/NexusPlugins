/**
 * @file parameter_return.c
 * @brief 从接口描述推断返回值类型函数 / Return type inference function from interface description / Rückgabetyp-Ableitungsfunktion aus Schnittstellenbeschreibung
 */

#include "pointer_transfer_utils.h"
#include "pointer_transfer_types.h"
#include "pointer_transfer_context.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <ctype.h>

/**
 * @brief 从接口描述字符串推断返回值类型 / Infer return type from interface description string / Rückgabetyp aus Schnittstellenbeschreibungs-Zeichenfolge ableiten
 * @param description 接口描述字符串 / Interface description string / Schnittstellenbeschreibungs-Zeichenfolge
 * @return 推断的返回值类型枚举值 / Inferred return type enumeration value / Abgeleiteter Rückgabetyp-Aufzählungswert
 */
pt_return_type_t infer_return_type_from_description(const char* description) {
    if (description == NULL || strlen(description) == 0) {
        return PT_RETURN_TYPE_INTEGER;
    }
    
    char* desc_lower = (char*)malloc(strlen(description) + 1);
    if (desc_lower == NULL) {
        return PT_RETURN_TYPE_INTEGER;
    }
    
    size_t desc_len = strlen(description);
    if (desc_len > 0) {
        memcpy(desc_lower, description, desc_len);
        desc_lower[desc_len] = '\0';
    } else {
        desc_lower[0] = '\0';
    }
    for (size_t j = 0; j < strlen(desc_lower); j++) {
        desc_lower[j] = (char)tolower((unsigned char)desc_lower[j]);
    }
    
    pt_return_type_t return_type = PT_RETURN_TYPE_INTEGER;
    if (strstr(desc_lower, "return float") != NULL || strstr(desc_lower, "returns float") != NULL ||
        strstr(desc_lower, "返回float") != NULL || strstr(desc_lower, "返回浮点") != NULL) {
        return_type = PT_RETURN_TYPE_FLOAT;
    } else if (strstr(desc_lower, "return double") != NULL || strstr(desc_lower, "returns double") != NULL ||
               strstr(desc_lower, "返回double") != NULL || strstr(desc_lower, "返回双精度") != NULL) {
        return_type = PT_RETURN_TYPE_DOUBLE;
    } else if (strstr(desc_lower, "return string pointer") != NULL || strstr(desc_lower, "returns string pointer") != NULL ||
               strstr(desc_lower, "return string") != NULL || strstr(desc_lower, "returns string") != NULL ||
               strstr(desc_lower, "返回字符串指针") != NULL || strstr(desc_lower, "返回字符串") != NULL ||
               strstr(desc_lower, "返回string") != NULL) {
        return_type = PT_RETURN_TYPE_STRUCT_PTR;
    } else if (strstr(desc_lower, "return struct") != NULL || strstr(desc_lower, "returns struct") != NULL ||
               strstr(desc_lower, "返回结构") != NULL) {
        return_type = PT_RETURN_TYPE_STRUCT_PTR;
    }
    
    free(desc_lower);
    return return_type;
}

