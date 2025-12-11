/**
 * @file format_plugin.c
 * @brief 格式化插件实现 / Format Plugin Implementation / Format-Plugin-Implementierung
 * @details 将数字格式化为字符串 / Format numbers as strings / Zahlen als Zeichenfolgen formatieren
 */

#include "nxld_plugin_interface.h"
#include "pointer_transfer_plugin_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef _WIN32
#define NXLD_PLUGIN_EXPORT __declspec(dllexport)
#define NXLD_PLUGIN_CALL __cdecl
#else
#define NXLD_PLUGIN_EXPORT __attribute__((visibility("default")))
#define NXLD_PLUGIN_CALL
#endif

#define PLUGIN_NAME "FormatPlugin"
#define PLUGIN_VERSION "1.0.0"
#define INTERFACE_COUNT 2

static const char* interface_names[] = {
    "FormatInt32",
    "FormatDouble"
};

static const char* interface_descriptions[] = {
    "Format int32 as string (value: int32), returns string pointer",
    "Format double as string (value: double), returns string pointer"
};

static const char* interface_version = "1.0.0";

/* 静态缓冲区用于存储格式化的字符串 / Static buffer for storing formatted strings / Statischer Puffer zum Speichern formatierter Zeichenfolgen */
static char format_buffer[256];

/**
 * @brief 获取插件名称 / Get plugin name / Plugin-Namen abrufen
 */
NXLD_PLUGIN_EXPORT int32_t NXLD_PLUGIN_CALL nxld_plugin_get_name(char* name, size_t name_size) {
    if (name == NULL || name_size == 0) {
        return -1;
    }
    size_t len = strlen(PLUGIN_NAME);
    if (len >= name_size) {
        len = name_size - 1;
    }
    memcpy(name, PLUGIN_NAME, len);
    name[len] = '\0';
    return 0;
}

/**
 * @brief 获取插件版本 / Get plugin version / Plugin-Version abrufen
 */
NXLD_PLUGIN_EXPORT int32_t NXLD_PLUGIN_CALL nxld_plugin_get_version(char* version, size_t version_size) {
    if (version == NULL || version_size == 0) {
        return -1;
    }
    size_t len = strlen(PLUGIN_VERSION);
    if (len >= version_size) {
        len = version_size - 1;
    }
    memcpy(version, PLUGIN_VERSION, len);
    version[len] = '\0';
    return 0;
}

/**
 * @brief 获取接口数量 / Get interface count / Schnittstellenanzahl abrufen
 */
NXLD_PLUGIN_EXPORT int32_t NXLD_PLUGIN_CALL nxld_plugin_get_interface_count(size_t* count) {
    if (count == NULL) {
        return -1;
    }
    *count = INTERFACE_COUNT;
    return 0;
}

/**
 * @brief 获取接口信息 / Get interface information / Schnittstelleninformationen abrufen
 */
NXLD_PLUGIN_EXPORT int32_t NXLD_PLUGIN_CALL nxld_plugin_get_interface_info(size_t index, 
                                                        char* name, size_t name_size,
                                                        char* description, size_t desc_size,
                                                        char* version, size_t version_size) {
    if (index >= INTERFACE_COUNT) {
        return -1;
    }
    
    if (name != NULL && name_size > 0) {
        size_t len = strlen(interface_names[index]);
        if (len >= name_size) {
            len = name_size - 1;
        }
        memcpy(name, interface_names[index], len);
        name[len] = '\0';
    }
    
    if (description != NULL && desc_size > 0) {
        size_t len = strlen(interface_descriptions[index]);
        if (len >= desc_size) {
            len = desc_size - 1;
        }
        memcpy(description, interface_descriptions[index], len);
        description[len] = '\0';
    }
    
    if (version != NULL && version_size > 0) {
        size_t len = strlen(interface_version);
        if (len >= version_size) {
            len = version_size - 1;
        }
        memcpy(version, interface_version, len);
        version[len] = '\0';
    }
    
    return 0;
}

/**
 * @brief 获取接口参数数量 / Get interface parameter count / Schnittstellenparameteranzahl abrufen
 */
NXLD_PLUGIN_EXPORT int32_t NXLD_PLUGIN_CALL nxld_plugin_get_interface_param_count(size_t index,
                                                               nxld_param_count_type_t* count_type,
                                                               int32_t* min_count, int32_t* max_count) {
    if (index >= INTERFACE_COUNT || count_type == NULL || min_count == NULL || max_count == NULL) {
        return -1;
    }
    
    *count_type = NXLD_PARAM_COUNT_FIXED;
    *min_count = 1;
    *max_count = 1;
    
    return 0;
}

/**
 * @brief 获取接口参数信息 / Get interface parameter information / Schnittstellenparameterinformationen abrufen
 */
NXLD_PLUGIN_EXPORT int32_t NXLD_PLUGIN_CALL nxld_plugin_get_interface_param_info(size_t index, int32_t param_index,
                                                              char* param_name, size_t name_size,
                                                              nxld_param_type_t* param_type,
                                                              char* type_name, size_t type_name_size) {
    if (index >= INTERFACE_COUNT || param_index < 0 || param_index >= 1 ||
        param_name == NULL || name_size == 0 || param_type == NULL) {
        return -1;
    }
    
    static const char* param_names[] = { "value" };
    static const nxld_param_type_t param_types[] = { 
        NXLD_PARAM_TYPE_INT32,  /* FormatInt32 */
        NXLD_PARAM_TYPE_DOUBLE   /* FormatDouble */
    };
    
    size_t len = strlen(param_names[param_index]);
    if (len >= name_size) {
        len = name_size - 1;
    }
    memcpy(param_name, param_names[param_index], len);
    param_name[len] = '\0';
    
    *param_type = param_types[index];
    
    if (type_name != NULL && type_name_size > 0) {
        const char* type_name_str = (index == 0) ? "int32_t" : "double";
        len = strlen(type_name_str);
        if (len >= type_name_size) {
            len = type_name_size - 1;
        }
        memcpy(type_name, type_name_str, len);
        type_name[len] = '\0';
    }
    
    return 0;
}

/**
 * @brief 将参数值转换为 double / Convert parameter value to double / Parameterwert in double umwandeln
 */
static double convert_to_double(const pt_curried_param_t* param) {
    if (param == NULL) {
        return 0.0;
    }
    
    switch (param->type) {
        case NXLD_PARAM_TYPE_INT32:
            return (double)param->value.int32_val;
        case NXLD_PARAM_TYPE_INT64:
            return (double)param->value.int64_val;
        case NXLD_PARAM_TYPE_FLOAT:
            return (double)param->value.float_val;
        case NXLD_PARAM_TYPE_DOUBLE:
            return param->value.double_val;
        default:
            return 0.0;
    }
}

/**
 * @brief 格式化 int32 为字符串 / Format int32 as string / int32 als Zeichenfolge formatieren
 * @param pack_ptr 参数包指针 / Parameter pack pointer / Parameterpaket-Zeiger
 * @return 格式化后的字符串指针 / Formatted string pointer / Formatierter Zeichenfolgen-Zeiger
 */
NXLD_PLUGIN_EXPORT const char* NXLD_PLUGIN_CALL FormatInt32(void* pack_ptr) {
    pt_param_pack_t* pack = (pt_param_pack_t*)pack_ptr;
    
    if (pack == NULL || pack->param_count < 1 || pack->params == NULL) {
        snprintf(format_buffer, sizeof(format_buffer), "0");
        return format_buffer;
    }
    
    int32_t value = (int32_t)convert_to_double(&pack->params[0]);
    snprintf(format_buffer, sizeof(format_buffer), "%d", value);
    
    return format_buffer;
}

/**
 * @brief 格式化 double 为字符串 / Format double as string / double als Zeichenfolge formatieren
 * @param pack_ptr 参数包指针 / Parameter pack pointer / Parameterpaket-Zeiger
 * @return 格式化后的字符串指针 / Formatted string pointer / Formatierter Zeichenfolgen-Zeiger
 */
NXLD_PLUGIN_EXPORT const char* NXLD_PLUGIN_CALL FormatDouble(void* pack_ptr) {
    pt_param_pack_t* pack = (pt_param_pack_t*)pack_ptr;
    
    if (pack == NULL || pack->param_count < 1 || pack->params == NULL) {
        snprintf(format_buffer, sizeof(format_buffer), "0.0");
        return format_buffer;
    }
    
    double value = convert_to_double(&pack->params[0]);
    snprintf(format_buffer, sizeof(format_buffer), "%.6f", value);
    
    return format_buffer;
}

