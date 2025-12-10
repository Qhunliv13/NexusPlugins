/**
 * @file pointer_transfer_utils.c
 * @brief 指针传递插件工具函数实现 / Pointer Transfer Plugin Utility Functions Implementation / Zeigerübertragungs-Plugin-Hilfsfunktionen-Implementierung
 */

#include "pointer_transfer_utils.h"
#include "pointer_transfer_types.h"
#include "pointer_transfer_context.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <limits.h>
#include <float.h>
#include <ctype.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

/**
 * @brief 内部日志输出函数 / Internal log output function / Interne Protokollausgabefunktion
 * @param level 日志级别字符串 / Log level string / Protokollierungsebenen-Zeichenfolge
 * @param format 格式化字符串 / Format string / Formatzeichenfolge
 * @param ... 可变参数 / Variable arguments / Variable Argumente
 */
void internal_log_write(const char* level, const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    time_t now;
    time(&now);
    struct tm* timeinfo = localtime(&now);
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", timeinfo);
    
    fprintf(stderr, "[%s] [%s] [PointerTransferPlugin] ", time_str, level);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    fflush(stderr);
    
    va_end(args);
}

/**
 * @brief 动态分配字符串 / Dynamically allocate string / Zeichenfolge dynamisch zuweisen
 */
char* allocate_string(const char* str) {
    if (str == NULL) {
        return NULL;
    }
    size_t len = strlen(str);
    char* result = (char*)malloc(len + 1);
    if (result != NULL) {
        memcpy(result, str, len);
        result[len] = '\0';
    }
    return result;
}

/**
 * @brief 去除字符串前后空白字符 / Trim whitespace from string / Leerzeichen von Zeichenfolge entfernen
 */
char* trim_string(char* str) {
    if (str == NULL) {
        return NULL;
    }
    
    while (*str == ' ' || *str == '\t') {
        str++;
    }
    
    size_t len = strlen(str);
    while (len > 0 && (str[len - 1] == ' ' || str[len - 1] == '\t' || str[len - 1] == '\r' || str[len - 1] == '\n')) {
        str[len - 1] = '\0';
        len--;
    }
    
    return str;
}

/**
 * @brief 解析键值对 / Parse key-value pair / Schlüssel-Wert-Paar parsen
 */
int parse_key_value_simple(const char* line, char* key, size_t key_size, char* value, size_t value_size) {
    if (line == NULL || key == NULL || value == NULL) {
        return 0;
    }
    
    const char* eq_pos = strchr(line, '=');
    if (eq_pos == NULL) {
        return 0;
    }
    
    size_t key_len = eq_pos - line;
    if (key_len >= key_size) {
        key_len = key_size - 1;
    }
    memcpy(key, line, key_len);
    key[key_len] = '\0';
    
    const char* value_start = eq_pos + 1;
    size_t value_len = strlen(value_start);
    if (value_len >= value_size) {
        value_len = value_size - 1;
    }
    memcpy(value, value_start, value_len);
    value[value_len] = '\0';
    
    return 1;
}

/**
 * @brief 获取参数类型名称字符串 / Get parameter type name string / Parametertypnamen-Zeichenfolge abrufen
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

/**
 * @brief 获取当前DLL路径 / Get current DLL path / Aktuellen DLL-Pfad abrufen
 */
int get_current_dll_path(char* path_buffer, size_t buffer_size) {
    if (path_buffer == NULL || buffer_size == 0) {
        return -1;
    }
    
#ifdef _WIN32
    HMODULE hModule = NULL;
    if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                           (LPCSTR)&get_current_dll_path, &hModule) == 0) {
        hModule = GetModuleHandleA(NULL);
        if (hModule == NULL) {
            return -1;
        }
    }
    
    DWORD result = GetModuleFileNameA(hModule, path_buffer, (DWORD)buffer_size);
    if (result == 0 || result >= buffer_size) {
        return -1;
    }
    return 0;
#else
    Dl_info info;
    if (dladdr((void*)&get_current_dll_path, &info) == 0) {
        return -1;
    }
    
    size_t len = strlen(info.dli_fname);
    if (len >= buffer_size) {
        len = buffer_size - 1;
    }
    memcpy(path_buffer, info.dli_fname, len);
    path_buffer[len] = '\0';
    return 0;
#endif
}

/**
 * @brief 构建.nxpt文件路径 / Build .nxpt file path / .nxpt-Dateipfad erstellen
 * @param file_path 源文件路径 / Source file path / Quelldateipfad
 * @param nxpt_path 输出.nxpt路径缓冲区 / Output .nxpt path buffer / Ausgabe-.nxpt-Pfad-Puffer
 * @param buffer_size 缓冲区大小 / Buffer size / Puffergröße
 * @return 成功返回0，失败返回非0 / Returns 0 on success, non-zero on failure / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
 */
int build_nxpt_path(const char* file_path, char* nxpt_path, size_t buffer_size) {
    if (file_path == NULL || nxpt_path == NULL || buffer_size == 0) {
        return -1;
    }
    
    const char* ext_pos = strrchr(file_path, '.');
    
    size_t base_len;
    if (ext_pos != NULL) {
        base_len = ext_pos - file_path;
    } else {
        base_len = strlen(file_path);
    }
    
    if (base_len + 5 >= buffer_size) {
        return -1;
    }
    
    memcpy(nxpt_path, file_path, base_len);
    memcpy(nxpt_path + base_len, ".nxpt", 6);
    
    return 0;
}

/**
 * @brief 从常量字符串设置参数值 / Set parameter value from constant string / Parameterwert aus Konstantenzeichenfolge setzen
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
        case NXLD_PARAM_TYPE_VARIADIC:
        case NXLD_PARAM_TYPE_ANY:
        case NXLD_PARAM_TYPE_UNKNOWN: {
            typed_state->param_values[param_index] = ptr;
            typed_state->param_ready[param_index] = 1;
            typed_state->param_sizes[param_index] = stored_size > 0 ? stored_size : sizeof(void*);
            internal_log_write("INFO", "Stored parameter %d for %s.%s (type=%d)", 
                param_index, plugin_name != NULL ? plugin_name : "unknown", 
                interface_name != NULL ? interface_name : "unknown", param_type);
            return 1;
        }
        default: {
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
 * @brief 从接口描述推断返回值类型 / Infer return type from interface description / Rückgabetyp aus Schnittstellenbeschreibung ableiten
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
    } else if (strstr(desc_lower, "return struct") != NULL || strstr(desc_lower, "returns struct") != NULL ||
               strstr(desc_lower, "返回结构") != NULL) {
        return_type = PT_RETURN_TYPE_STRUCT_PTR;
    }
    
    free(desc_lower);
    return return_type;
}

/**
 * @brief 释放单个传递规则的内存 / Free memory of single transfer rule / Speicher einer einzelnen Übertragungsregel freigeben
 */
void free_single_rule(pointer_transfer_rule_t* rule) {
    if (rule == NULL) {
        return;
    }
    
    if (rule->source_plugin != NULL) {
        free(rule->source_plugin);
        rule->source_plugin = NULL;
    }
    if (rule->source_interface != NULL) {
        free(rule->source_interface);
        rule->source_interface = NULL;
    }
    if (rule->target_plugin != NULL) {
        free(rule->target_plugin);
        rule->target_plugin = NULL;
    }
    if (rule->target_plugin_path != NULL) {
        free(rule->target_plugin_path);
        rule->target_plugin_path = NULL;
    }
    if (rule->target_interface != NULL) {
        free(rule->target_interface);
        rule->target_interface = NULL;
    }
    if (rule->target_param_value != NULL) {
        free(rule->target_param_value);
        rule->target_param_value = NULL;
    }
    if (rule->description != NULL) {
        free(rule->description);
        rule->description = NULL;
    }
    if (rule->multicast_group != NULL) {
        free(rule->multicast_group);
        rule->multicast_group = NULL;
    }
    if (rule->condition != NULL) {
        free(rule->condition);
        rule->condition = NULL;
    }
}


