/**
 * @file parameter_pointer.c
 * @brief 从指针设置参数值函数 / Parameter value setting function from pointer / Parameterwert-Setzfunktion aus Zeiger
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
            int64_t value = 0;
            if (ptr != NULL) {
                if (stored_size == sizeof(int64_t)) {
                    value = *(int64_t*)ptr;
                } else if (stored_size == sizeof(int32_t)) {
                    value = (int64_t)*(int32_t*)ptr;
                } else {
                    value = (int64_t)*(int32_t*)ptr;
                }
            }
            typed_state->param_int_values[param_index] = value;
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

