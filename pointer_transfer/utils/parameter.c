/**
 * @file parameter.c
 * @brief 参数值设置函数 / Parameter value setting functions / Parameterwert-Setzfunktionen
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
 * @brief 从常量字符串设置参数值 / Set parameter value from constant string / Parameterwert aus Konstantenzeichenfolge setzen
 * @param state 接口状态结构体指针 / Interface state structure pointer / Schnittstellenstatus-Struktur-Zeiger
 * @param param_index 参数索引 / Parameter index / Parameterindex
 * @param const_value 常量值字符串 / Constant value string / Konstantenwert-Zeichenfolge
 * @param plugin_name 插件名称 / Plugin name / Plugin-Name
 * @param interface_name 接口名称 / Interface name / Schnittstellenname
 * @return 成功返回1，失败返回0 / Returns 1 on success, 0 on failure / Gibt 1 bei Erfolg zurück, 0 bei Fehler
 */
int set_parameter_value_from_const_string(struct target_interface_state_s* state, int param_index, const char* const_value, const char* plugin_name, const char* interface_name) {
    target_interface_state_t* typed_state = (target_interface_state_t*)state;
    if (typed_state == NULL || const_value == NULL || strlen(const_value) == 0 || 
        param_index < 0 || param_index >= typed_state->param_count ||
        typed_state->param_types == NULL || typed_state->param_values == NULL ||
        typed_state->param_int_values == NULL || typed_state->param_float_values == NULL) {
        return 0;
    }
    
    nxld_param_type_t param_type = typed_state->param_types[param_index];
    char* endptr = NULL;
    
    switch (param_type) {
        case NXLD_PARAM_TYPE_INT32: {
            long parsed_val = strtol(const_value, &endptr, 10);
            if (endptr != NULL && *endptr == '\0' && parsed_val >= INT32_MIN && parsed_val <= INT32_MAX) {
                typed_state->param_int_values[param_index] = (int64_t)parsed_val;
                typed_state->param_values[param_index] = &typed_state->param_int_values[param_index];
                typed_state->param_ready[param_index] = 1;
                typed_state->param_sizes[param_index] = sizeof(int32_t);
                internal_log_write("INFO", "Using constant value for parameter %d of %s.%s: %d", 
                    param_index, plugin_name != NULL ? plugin_name : "unknown", 
                    interface_name != NULL ? interface_name : "unknown", (int)typed_state->param_int_values[param_index]);
                return 1;
            }
            break;
        }
        case NXLD_PARAM_TYPE_INT64: {
            long long parsed_val = strtoll(const_value, &endptr, 10);
            if (endptr != NULL && *endptr == '\0') {
                typed_state->param_int_values[param_index] = (int64_t)parsed_val;
                typed_state->param_values[param_index] = &typed_state->param_int_values[param_index];
                typed_state->param_ready[param_index] = 1;
                typed_state->param_sizes[param_index] = sizeof(int64_t);
                internal_log_write("INFO", "Using constant value for parameter %d of %s.%s: %lld", 
                    param_index, plugin_name != NULL ? plugin_name : "unknown", 
                    interface_name != NULL ? interface_name : "unknown", (long long)typed_state->param_int_values[param_index]);
                return 1;
            }
            break;
        }
        case NXLD_PARAM_TYPE_FLOAT: {
            double parsed_val = strtod(const_value, &endptr);
            if (endptr != NULL && *endptr == '\0' && parsed_val >= -FLT_MAX && parsed_val <= FLT_MAX) {
                typed_state->param_float_values[param_index] = (double)parsed_val;
                typed_state->param_values[param_index] = &typed_state->param_float_values[param_index];
                typed_state->param_ready[param_index] = 1;
                typed_state->param_sizes[param_index] = sizeof(float);
                internal_log_write("INFO", "Using constant value for parameter %d of %s.%s: %f", 
                    param_index, plugin_name != NULL ? plugin_name : "unknown", 
                    interface_name != NULL ? interface_name : "unknown", (float)typed_state->param_float_values[param_index]);
                return 1;
            }
            break;
        }
        case NXLD_PARAM_TYPE_DOUBLE: {
            double parsed_val = strtod(const_value, &endptr);
            if (endptr != NULL && *endptr == '\0') {
                typed_state->param_float_values[param_index] = parsed_val;
                typed_state->param_values[param_index] = &typed_state->param_float_values[param_index];
                typed_state->param_ready[param_index] = 1;
                typed_state->param_sizes[param_index] = sizeof(double);
                internal_log_write("INFO", "Using constant value for parameter %d of %s.%s: %lf", 
                    param_index, plugin_name != NULL ? plugin_name : "unknown", 
                    interface_name != NULL ? interface_name : "unknown", typed_state->param_float_values[param_index]);
                return 1;
            }
            break;
        }
        case NXLD_PARAM_TYPE_CHAR: {
            typed_state->param_int_values[param_index] = (int64_t)(const_value[0]);
            typed_state->param_values[param_index] = &typed_state->param_int_values[param_index];
            typed_state->param_ready[param_index] = 1;
            typed_state->param_sizes[param_index] = sizeof(char);
            internal_log_write("INFO", "Using constant value for parameter %d of %s.%s: '%c'", 
                param_index, plugin_name != NULL ? plugin_name : "unknown", 
                interface_name != NULL ? interface_name : "unknown", (char)typed_state->param_int_values[param_index]);
            return 1;
        }
        case NXLD_PARAM_TYPE_STRING: {
            typed_state->param_values[param_index] = (void*)const_value;
            typed_state->param_ready[param_index] = 1;
            typed_state->param_sizes[param_index] = strlen(const_value) + 1;
            internal_log_write("INFO", "Using constant value for parameter %d of %s.%s: %s", 
                param_index, plugin_name != NULL ? plugin_name : "unknown", 
                interface_name != NULL ? interface_name : "unknown", const_value);
            return 1;
        }
        default:
            break;
    }
    
    return 0;
}

/**
 * @brief 从指针设置参数值 / Set parameter value from pointer / Parameterwert aus Zeiger setzen
 * @param state 接口状态结构体指针 / Interface state structure pointer / Schnittstellenstatus-Struktur-Zeiger
 * @param param_index 参数索引 / Parameter index / Parameterindex
 * @param ptr 参数值指针 / Parameter value pointer / Parameterwert-Zeiger
 * @param stored_size 存储的数据大小 / Stored data size / Gespeicherte Datengröße
 * @param plugin_name 插件名称 / Plugin name / Plugin-Name
 * @param interface_name 接口名称 / Interface name / Schnittstellenname
 * @return 成功返回1，失败返回0 / Returns 1 on success, 0 on failure / Gibt 1 bei Erfolg zurück, 0 bei Fehler
 */
int set_parameter_value_from_pointer(struct target_interface_state_s* state, int param_index, void* ptr, size_t stored_size, const char* plugin_name, const char* interface_name) {
    target_interface_state_t* typed_state = (target_interface_state_t*)state;
    if (typed_state == NULL || param_index < 0 || param_index >= typed_state->param_count ||
        typed_state->param_types == NULL || typed_state->param_values == NULL ||
        typed_state->param_int_values == NULL || typed_state->param_float_values == NULL) {
        return 0;
    }
    
    nxld_param_type_t param_type = typed_state->param_types[param_index];
    
    switch (param_type) {
        case NXLD_PARAM_TYPE_INT32: {
            typed_state->param_int_values[param_index] = ptr != NULL ? (int64_t)*(int32_t*)ptr : 0;
            typed_state->param_values[param_index] = &typed_state->param_int_values[param_index];
            typed_state->param_ready[param_index] = 1;
            typed_state->param_sizes[param_index] = sizeof(int32_t);
            internal_log_write("INFO", "Stored parameter %d for %s.%s (INT32 value: %d)", 
                param_index, plugin_name != NULL ? plugin_name : "unknown", 
                interface_name != NULL ? interface_name : "unknown", (int)typed_state->param_int_values[param_index]);
            return 1;
        }
        case NXLD_PARAM_TYPE_INT64: {
            typed_state->param_int_values[param_index] = ptr != NULL ? *(int64_t*)ptr : 0;
            typed_state->param_values[param_index] = &typed_state->param_int_values[param_index];
            typed_state->param_ready[param_index] = 1;
            typed_state->param_sizes[param_index] = sizeof(int64_t);
            internal_log_write("INFO", "Stored parameter %d for %s.%s (INT64 value: %lld)", 
                param_index, plugin_name != NULL ? plugin_name : "unknown", 
                interface_name != NULL ? interface_name : "unknown", (long long)typed_state->param_int_values[param_index]);
            return 1;
        }
        case NXLD_PARAM_TYPE_FLOAT: {
            typed_state->param_float_values[param_index] = ptr != NULL ? (double)*(float*)ptr : 0.0;
            typed_state->param_values[param_index] = &typed_state->param_float_values[param_index];
            typed_state->param_ready[param_index] = 1;
            typed_state->param_sizes[param_index] = sizeof(float);
            internal_log_write("INFO", "Stored parameter %d for %s.%s (FLOAT value: %f)", 
                param_index, plugin_name != NULL ? plugin_name : "unknown", 
                interface_name != NULL ? interface_name : "unknown", (float)typed_state->param_float_values[param_index]);
            return 1;
        }
        case NXLD_PARAM_TYPE_DOUBLE: {
            typed_state->param_float_values[param_index] = ptr != NULL ? *(double*)ptr : 0.0;
            typed_state->param_values[param_index] = &typed_state->param_float_values[param_index];
            typed_state->param_ready[param_index] = 1;
            typed_state->param_sizes[param_index] = sizeof(double);
            internal_log_write("INFO", "Stored parameter %d for %s.%s (DOUBLE value: %lf)", 
                param_index, plugin_name != NULL ? plugin_name : "unknown", 
                interface_name != NULL ? interface_name : "unknown", typed_state->param_float_values[param_index]);
            return 1;
        }
        case NXLD_PARAM_TYPE_CHAR: {
            typed_state->param_int_values[param_index] = ptr != NULL ? (int64_t)*(char*)ptr : 0;
            typed_state->param_values[param_index] = &typed_state->param_int_values[param_index];
            typed_state->param_ready[param_index] = 1;
            typed_state->param_sizes[param_index] = sizeof(char);
            internal_log_write("INFO", "Stored parameter %d for %s.%s (CHAR value: '%c')", 
                param_index, plugin_name != NULL ? plugin_name : "unknown", 
                interface_name != NULL ? interface_name : "unknown", (char)typed_state->param_int_values[param_index]);
            return 1;
        }
        case NXLD_PARAM_TYPE_POINTER:
        case NXLD_PARAM_TYPE_STRING: {
            typed_state->param_values[param_index] = ptr;
            typed_state->param_ready[param_index] = 1;
            typed_state->param_sizes[param_index] = stored_size > 0 ? stored_size : sizeof(void*);
            internal_log_write("INFO", "Stored parameter %d for %s.%s (type=%d, size=%zu)", 
                param_index, plugin_name != NULL ? plugin_name : "unknown", 
                interface_name != NULL ? interface_name : "unknown", param_type, typed_state->param_sizes[param_index]);
            return 1;
        }
        case NXLD_PARAM_TYPE_VARIADIC:
        case NXLD_PARAM_TYPE_ANY:
        case NXLD_PARAM_TYPE_UNKNOWN: {
            if (stored_size > 0 && stored_size <= sizeof(int64_t) && ptr != NULL) {
                typed_state->param_int_values[param_index] = *(int64_t*)ptr;
                typed_state->param_values[param_index] = &typed_state->param_int_values[param_index];
                typed_state->param_ready[param_index] = 1;
                typed_state->param_sizes[param_index] = stored_size;
                internal_log_write("INFO", "Stored parameter %d for %s.%s (type=%d, INT64 value: %lld)", 
                    param_index, plugin_name != NULL ? plugin_name : "unknown", 
                    interface_name != NULL ? interface_name : "unknown", param_type, (long long)typed_state->param_int_values[param_index]);
                return 1;
            }
            typed_state->param_values[param_index] = ptr;
            typed_state->param_ready[param_index] = 1;
            typed_state->param_sizes[param_index] = stored_size > 0 ? stored_size : sizeof(void*);
            internal_log_write("INFO", "Stored parameter %d for %s.%s (type=%d)", 
                param_index, plugin_name != NULL ? plugin_name : "unknown", 
                interface_name != NULL ? interface_name : "unknown", param_type);
            return 1;
        }
        default: {
            if (stored_size > 0 && stored_size <= sizeof(int64_t) && ptr != NULL) {
                typed_state->param_int_values[param_index] = *(int64_t*)ptr;
                typed_state->param_values[param_index] = &typed_state->param_int_values[param_index];
                typed_state->param_ready[param_index] = 1;
                typed_state->param_sizes[param_index] = stored_size;
                internal_log_write("INFO", "Stored parameter %d for %s.%s (type=%d, INT64 value: %lld)", 
                    param_index, plugin_name != NULL ? plugin_name : "unknown", 
                    interface_name != NULL ? interface_name : "unknown", param_type, (long long)typed_state->param_int_values[param_index]);
                return 1;
            }
            typed_state->param_values[param_index] = ptr;
            typed_state->param_ready[param_index] = 1;
            typed_state->param_sizes[param_index] = stored_size > 0 ? stored_size : sizeof(void*);
            internal_log_write("INFO", "Stored parameter %d for %s.%s (type=%d, size=%zu)", 
                param_index, plugin_name != NULL ? plugin_name : "unknown", 
                interface_name != NULL ? interface_name : "unknown", param_type, typed_state->param_sizes[param_index]);
            return 1;
        }
    }
}

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

