/**
 * @file parameter_const.c
 * @brief 从常量字符串设置参数值函数 / Parameter value setting function from constant string / Parameterwert-Setzfunktion aus Konstantenzeichenfolge
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

