/**
 * @file add_plugin.c
 * @brief 加法插件 / Addition Plugin / Additions-Plugin
 */

#include "nxld_plugin_interface.h"
#include "pointer_transfer_plugin_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#ifdef _WIN32
#define NXLD_PLUGIN_EXPORT __declspec(dllexport)
#define NXLD_PLUGIN_CALL __cdecl
#else
#define NXLD_PLUGIN_EXPORT __attribute__((visibility("default")))
#define NXLD_PLUGIN_CALL
#endif

#define PLUGIN_NAME "AddPlugin"
#define PLUGIN_VERSION "1.0.0"
#define INTERFACE_COUNT 1

static const char* interface_name = "Add";
static const char* interface_description = "Add two numbers of any type (int32/int64/float/double, positive/negative), returns double";
static const char* interface_version = "1.0.0";

/**
 * @brief 获取插件名称 / Get plugin name / Plugin-Namen abrufen
 * @param name 名称缓冲区 / Name buffer / Namenspuffer
 * @param name_size 缓冲区大小 / Buffer size / Puffergröße
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
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
 * @param version 版本缓冲区 / Version buffer / Versionspuffer
 * @param version_size 缓冲区大小 / Buffer size / Puffergröße
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
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
 * @param count 接口数量输出指针 / Interface count output pointer / Schnittstellenanzahl-Ausgabezeiger
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
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
 * @param index 接口索引 / Interface index / Schnittstellenindex
 * @param name 名称缓冲区 / Name buffer / Namenspuffer
 * @param name_size 名称缓冲区大小 / Name buffer size / Namenspuffergröße
 * @param description 描述缓冲区 / Description buffer / Beschreibungspuffer
 * @param desc_size 描述缓冲区大小 / Description buffer size / Beschreibungspuffergröße
 * @param version 版本缓冲区 / Version buffer / Versionspuffer
 * @param version_size 版本缓冲区大小 / Version buffer size / Versionspuffergröße
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
NXLD_PLUGIN_EXPORT int32_t NXLD_PLUGIN_CALL nxld_plugin_get_interface_info(size_t index, 
                                                        char* name, size_t name_size,
                                                        char* description, size_t desc_size,
                                                        char* version, size_t version_size) {
    if (index >= INTERFACE_COUNT) {
        return -1;
    }
    
    if (name != NULL && name_size > 0) {
        size_t len = strlen(interface_name);
        if (len >= name_size) {
            len = name_size - 1;
        }
        memcpy(name, interface_name, len);
        name[len] = '\0';
    }
    
    if (description != NULL && desc_size > 0) {
        size_t len = strlen(interface_description);
        if (len >= desc_size) {
            len = desc_size - 1;
        }
        memcpy(description, interface_description, len);
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
 * @param index 接口索引 / Interface index / Schnittstellenindex
 * @param count_type 参数数量类型输出指针 / Parameter count type output pointer / Parameteranzahl-Typ-Ausgabezeiger
 * @param min_count 最小参数数量输出指针 / Minimum parameter count output pointer / Mindestparameteranzahl-Ausgabezeiger
 * @param max_count 最大参数数量输出指针 / Maximum parameter count output pointer / Maximalparameteranzahl-Ausgabezeiger
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
NXLD_PLUGIN_EXPORT int32_t NXLD_PLUGIN_CALL nxld_plugin_get_interface_param_count(size_t index,
                                                               nxld_param_count_type_t* count_type,
                                                               int32_t* min_count, int32_t* max_count) {
    if (index >= INTERFACE_COUNT || count_type == NULL || min_count == NULL || max_count == NULL) {
        return -1;
    }
    
    *count_type = NXLD_PARAM_COUNT_FIXED;
    *min_count = 2;
    *max_count = 2;
    
    return 0;
}

/**
 * @brief 获取接口参数信息 / Get interface parameter information / Schnittstellenparameterinformationen abrufen
 * @param index 接口索引 / Interface index / Schnittstellenindex
 * @param param_index 参数索引 / Parameter index / Parameterindex
 * @param param_name 参数名称缓冲区 / Parameter name buffer / Parameternamen-Puffer
 * @param name_size 参数名称缓冲区大小 / Parameter name buffer size / Parameternamen-Puffergröße
 * @param param_type 参数类型输出指针 / Parameter type output pointer / Parametertyp-Ausgabezeiger
 * @param type_name 类型名称缓冲区 / Type name buffer / Typnamen-Puffer
 * @param type_name_size 类型名称缓冲区大小 / Type name buffer size / Typnamen-Puffergröße
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
NXLD_PLUGIN_EXPORT int32_t NXLD_PLUGIN_CALL nxld_plugin_get_interface_param_info(size_t index, int32_t param_index,
                                                              char* param_name, size_t name_size,
                                                              nxld_param_type_t* param_type,
                                                              char* type_name, size_t type_name_size) {
    if (index >= INTERFACE_COUNT || param_index < 0 || param_index >= 2 ||
        param_name == NULL || name_size == 0 || param_type == NULL) {
        return -1;
    }
    
    static const char* param_names[] = { "a", "b" };
    static const nxld_param_type_t param_types[] = { 
        NXLD_PARAM_TYPE_ANY,
        NXLD_PARAM_TYPE_ANY
    };
    
    size_t len = strlen(param_names[param_index]);
    if (len >= name_size) {
        len = name_size - 1;
    }
    memcpy(param_name, param_names[param_index], len);
    param_name[len] = '\0';
    
    *param_type = param_types[param_index];
    
    if (type_name != NULL && type_name_size > 0) {
        const char* type_name_str = "int32_t|int64_t|float|double";
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
 * @param param 参数指针 / Parameter pointer / Parameterzeiger
 * @return double 值 / double value / double-Wert
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
        case NXLD_PARAM_TYPE_CHAR:
            return (double)param->value.char_val;
        case NXLD_PARAM_TYPE_ANY:
        case NXLD_PARAM_TYPE_POINTER:
        case NXLD_PARAM_TYPE_VARIADIC:
        case NXLD_PARAM_TYPE_UNKNOWN: {
            // 对于ANY/POINTER类型，尝试从ptr_val中读取值 / For ANY/POINTER types, try to read value from ptr_val / Für ANY/POINTER-Typen, versuchen, Wert aus ptr_val zu lesen
            if (param->value.ptr_val != NULL) {
                // 根据size判断类型 / Determine type by size / Typ nach Größe bestimmen
                if (param->size == sizeof(int32_t) || (param->size == 0 && sizeof(void*) == sizeof(int32_t))) {
                    int32_t* int32_ptr = (int32_t*)param->value.ptr_val;
                    return (double)*int32_ptr;
                } else if (param->size == sizeof(int64_t) || (param->size == sizeof(void*) && sizeof(void*) == sizeof(int64_t))) {
                    int64_t* int64_ptr = (int64_t*)param->value.ptr_val;
                    return (double)*int64_ptr;
                } else if (param->size == sizeof(double)) {
                    double* double_ptr = (double*)param->value.ptr_val;
                    return *double_ptr;
                } else if (param->size == sizeof(float)) {
                    float* float_ptr = (float*)param->value.ptr_val;
                    return (double)*float_ptr;
                } else {
                    // 默认尝试读取int64_t（因为result_int是int64_t类型）/ Default try to read int64_t (because result_int is int64_t) / Standardmäßig versuchen, int64_t zu lesen (weil result_int int64_t ist)
                    int64_t* int64_ptr = (int64_t*)param->value.ptr_val;
                    return (double)*int64_ptr;
                }
            }
            return 0.0;
        }
        default:
            return 0.0;
    }
}

/**
 * @brief 加法函数 / Addition function / Additionsfunktion
 * @param pack_ptr 参数包指针 / Parameter pack pointer / Parameterpaket-Zeiger
 * @return 两个参数的和 / Sum of two parameters / Summe zweier Parameter
 */
NXLD_PLUGIN_EXPORT double NXLD_PLUGIN_CALL Add(void* pack_ptr) {
    pt_param_pack_t* pack = (pt_param_pack_t*)pack_ptr;
    
    if (pack == NULL || pack->param_count < 2 || pack->params == NULL) {
        return 0.0;
    }
    
    double a = convert_to_double(&pack->params[0]);
    double b = convert_to_double(&pack->params[1]);
    
    return a + b;
}

