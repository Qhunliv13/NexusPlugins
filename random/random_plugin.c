/**
 * @file random_plugin.c
 * @brief 随机数生成插件实现 / Random Number Generation Plugin Implementation / Zufallszahlengenerierungs-Plugin-Implementierung
 * @details 能够生成任意类型的随机数（int32/int64/float/double），支持指定范围 / Can generate random numbers of any type (int32/int64/float/double), supports range specification / Kann Zufallszahlen beliebigen Typs (int32/int64/float/double) generieren, unterstützt Bereichsangabe
 */

#include "nxld_plugin_interface.h"
#include "pointer_transfer_plugin_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <math.h>

#ifdef _WIN32
#define NXLD_PLUGIN_EXPORT __declspec(dllexport)
#define NXLD_PLUGIN_CALL __cdecl
#include <windows.h>
#include <wincrypt.h>
#else
#define NXLD_PLUGIN_EXPORT __attribute__((visibility("default")))
#define NXLD_PLUGIN_CALL
#endif

/* 插件名称 / Plugin name / Plugin-Name */
#define PLUGIN_NAME "RandomPlugin"
#define PLUGIN_VERSION "1.0.0"
#define INTERFACE_COUNT 4

/* 接口名称 / Interface names / Schnittstellennamen */
static const char* interface_names[] = {
    "RandomInt32",
    "RandomInt64",
    "RandomFloat",
    "RandomDouble"
};

/* 接口描述 / Interface descriptions / Schnittstellenbeschreibungen */
static const char* interface_descriptions[] = {
    "Generate random int32 (optional: min, max), returns int32",
    "Generate random int64 (optional: min, max), returns int64",
    "Generate random float (optional: min, max), returns float",
    "Generate random double (optional: min, max), returns double"
};

/* 接口版本 / Interface versions / Schnittstellenversionen */
static const char* interface_version = "1.0.0";

/* 随机数生成器初始化标志 / Random number generator initialization flag / Zufallszahlengenerator-Initialisierungsflag */
static int random_initialized = 0;

/**
 * @brief 初始化随机数生成器 / Initialize random number generator / Zufallszahlengenerator initialisieren
 */
static void init_random(void) {
    if (!random_initialized) {
#ifdef _WIN32
        /* Windows: 使用 CryptGenRandom 或 time+rand / Windows: Use CryptGenRandom or time+rand / Windows: Verwenden Sie CryptGenRandom oder time+rand */
        srand((unsigned int)time(NULL));
#else
        /* Linux/Unix: 使用 time+rand / Linux/Unix: Use time+rand / Linux/Unix: Verwenden Sie time+rand */
        srand((unsigned int)time(NULL));
#endif
        random_initialized = 1;
    }
}

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
    
    /* 所有接口都支持 0-2 个参数（无参数、min、min+max）/ All interfaces support 0-2 parameters (no params, min, min+max) / Alle Schnittstellen unterstützen 0-2 Parameter (keine Parameter, min, min+max) */
    *count_type = NXLD_PARAM_COUNT_VARIABLE;
    *min_count = 0;
    *max_count = 2;
    
    return 0;
}

/**
 * @brief 获取接口参数信息 / Get interface parameter information / Schnittstellenparameterinformationen abrufen
 */
NXLD_PLUGIN_EXPORT int32_t NXLD_PLUGIN_CALL nxld_plugin_get_interface_param_info(size_t index, int32_t param_index,
                                                              char* param_name, size_t name_size,
                                                              nxld_param_type_t* param_type,
                                                              char* type_name, size_t type_name_size) {
    if (index >= INTERFACE_COUNT || param_index < 0 || param_index >= 2 ||
        param_name == NULL || name_size == 0 || param_type == NULL) {
        return -1;
    }
    
    static const char* param_names[][2] = {
        { "min", "max" },  /* RandomInt32 */
        { "min", "max" },  /* RandomInt64 */
        { "min", "max" },  /* RandomFloat */
        { "min", "max" }   /* RandomDouble */
    };
    
    /* 参数类型根据接口类型确定 / Parameter type determined by interface type / Parametertyp durch Schnittstellentyp bestimmt */
    static const nxld_param_type_t param_types[][2] = {
        { NXLD_PARAM_TYPE_INT32, NXLD_PARAM_TYPE_INT32 },  /* RandomInt32 */
        { NXLD_PARAM_TYPE_INT64, NXLD_PARAM_TYPE_INT64 },  /* RandomInt64 */
        { NXLD_PARAM_TYPE_FLOAT, NXLD_PARAM_TYPE_FLOAT }, /* RandomFloat */
        { NXLD_PARAM_TYPE_DOUBLE, NXLD_PARAM_TYPE_DOUBLE }  /* RandomDouble */
    };
    
    size_t len = strlen(param_names[index][param_index]);
    if (len >= name_size) {
        len = name_size - 1;
    }
    memcpy(param_name, param_names[index][param_index], len);
    param_name[len] = '\0';
    
    *param_type = param_types[index][param_index];
    
    if (type_name != NULL && type_name_size > 0) {
        const char* type_name_str = NULL;
        if (index == 0) {
            type_name_str = "int32_t";
        } else if (index == 1) {
            type_name_str = "int64_t";
        } else if (index == 2) {
            type_name_str = "float";
        } else if (index == 3) {
            type_name_str = "double";
        }
        
        if (type_name_str != NULL) {
            len = strlen(type_name_str);
            if (len >= type_name_size) {
                len = type_name_size - 1;
            }
            memcpy(type_name, type_name_str, len);
            type_name[len] = '\0';
        }
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
 * @brief 生成随机 int32 / Generate random int32 / Zufälliges int32 generieren
 * @param pack_ptr 参数包指针 / Parameter pack pointer / Parameterpaket-Zeiger
 * @return 随机 int32 值 / Random int32 value / Zufälliger int32-Wert
 * 
 * @details
 * 参数：
 * - 无参数：返回 0 到 RAND_MAX 之间的随机数
 * - 1个参数 (min)：返回 min 到 RAND_MAX 之间的随机数
 * - 2个参数 (min, max)：返回 min 到 max 之间的随机数
 * 
 * Parameters:
 * - No params: Returns random number between 0 and RAND_MAX
 * - 1 param (min): Returns random number between min and RAND_MAX
 * - 2 params (min, max): Returns random number between min and max
 */
NXLD_PLUGIN_EXPORT int32_t NXLD_PLUGIN_CALL RandomInt32(void* pack_ptr) {
    pt_param_pack_t* pack = (pt_param_pack_t*)pack_ptr;
    
    init_random();
    
    if (pack == NULL || pack->param_count == 0) {
        /* 无参数：返回 0 到 RAND_MAX / No params: return 0 to RAND_MAX / Keine Parameter: 0 bis RAND_MAX zurückgeben */
        return (int32_t)(rand() % ((int32_t)RAND_MAX + 1));
    }
    
    if (pack->param_count == 1) {
        /* 1个参数：min 到 RAND_MAX / 1 param: min to RAND_MAX / 1 Parameter: min bis RAND_MAX */
        int32_t min = (int32_t)convert_to_double(&pack->params[0]);
        int32_t range = ((int32_t)RAND_MAX + 1) - min;
        if (range <= 0) {
            return min;
        }
        return min + (int32_t)(rand() % range);
    }
    
    /* 2个参数：min 到 max / 2 params: min to max / 2 Parameter: min bis max */
    int32_t min = (int32_t)convert_to_double(&pack->params[0]);
    int32_t max = (int32_t)convert_to_double(&pack->params[1]);
    
    if (min > max) {
        /* 交换 min 和 max / Swap min and max / min und max vertauschen */
        int32_t temp = min;
        min = max;
        max = temp;
    }
    
    int32_t range = max - min + 1;
    if (range <= 0) {
        return min;
    }
    
    return min + (int32_t)(rand() % range);
}

/**
 * @brief 生成随机 int64 / Generate random int64 / Zufälliges int64 generieren
 * @param pack_ptr 参数包指针 / Parameter pack pointer / Parameterpaket-Zeiger
 * @return 随机 int64 值 / Random int64 value / Zufälliger int64-Wert
 */
NXLD_PLUGIN_EXPORT int64_t NXLD_PLUGIN_CALL RandomInt64(void* pack_ptr) {
    pt_param_pack_t* pack = (pt_param_pack_t*)pack_ptr;
    
    init_random();
    
    if (pack == NULL || pack->param_count == 0) {
        /* 无参数：返回 0 到 RAND_MAX / No params: return 0 to RAND_MAX / Keine Parameter: 0 bis RAND_MAX zurückgeben */
        return (int64_t)rand();
    }
    
    if (pack->param_count == 1) {
        /* 1个参数：min 到 RAND_MAX / 1 param: min to RAND_MAX / 1 Parameter: min bis RAND_MAX */
        int64_t min = (int64_t)convert_to_double(&pack->params[0]);
        int64_t range = ((int64_t)RAND_MAX + 1) - min;
        if (range <= 0) {
            return min;
        }
        /* 使用多个 rand() 调用生成更大的范围 / Use multiple rand() calls for larger range / Mehrere rand()-Aufrufe für größeren Bereich */
        int64_t result = min + ((int64_t)rand() % range);
        return result;
    }
    
    /* 2个参数：min 到 max / 2 params: min to max / 2 Parameter: min bis max */
    int64_t min = (int64_t)convert_to_double(&pack->params[0]);
    int64_t max = (int64_t)convert_to_double(&pack->params[1]);
    
    if (min > max) {
        int64_t temp = min;
        min = max;
        max = temp;
    }
    
    int64_t range = max - min + 1;
    if (range <= 0) {
        return min;
    }
    
    /* 对于大范围，使用多个 rand() 调用 / For large ranges, use multiple rand() calls / Für große Bereiche mehrere rand()-Aufrufe verwenden */
    if (range > (int64_t)RAND_MAX) {
        int64_t result = min;
        int64_t remaining = range;
        while (remaining > (int64_t)RAND_MAX) {
            result += (int64_t)(rand() % ((int64_t)RAND_MAX + 1));
            remaining -= (int64_t)RAND_MAX;
        }
        result += (int64_t)(rand() % remaining);
        return result;
    }
    
    return min + (int64_t)(rand() % range);
}

/**
 * @brief 生成随机 float / Generate random float / Zufälliges float generieren
 * @param pack_ptr 参数包指针 / Parameter pack pointer / Parameterpaket-Zeiger
 * @return 随机 float 值 / Random float value / Zufälliger float-Wert
 * 
 * @details
 * 参数：
 * - 无参数：返回 0.0 到 1.0 之间的随机数
 * - 1个参数 (min)：返回 min 到 1.0 之间的随机数
 * - 2个参数 (min, max)：返回 min 到 max 之间的随机数
 */
NXLD_PLUGIN_EXPORT float NXLD_PLUGIN_CALL RandomFloat(void* pack_ptr) {
    pt_param_pack_t* pack = (pt_param_pack_t*)pack_ptr;
    
    init_random();
    
    if (pack == NULL || pack->param_count == 0) {
        /* 无参数：返回 0.0 到 1.0 / No params: return 0.0 to 1.0 / Keine Parameter: 0.0 bis 1.0 zurückgeben */
        return (float)((double)rand() / (double)RAND_MAX);
    }
    
    if (pack->param_count == 1) {
        /* 1个参数：min 到 1.0 / 1 param: min to 1.0 / 1 Parameter: min bis 1.0 */
        float min = (float)convert_to_double(&pack->params[0]);
        float range = 1.0f - min;
        return min + (float)((double)rand() / (double)RAND_MAX) * range;
    }
    
    /* 2个参数：min 到 max / 2 params: min to max / 2 Parameter: min bis max */
    float min = (float)convert_to_double(&pack->params[0]);
    float max = (float)convert_to_double(&pack->params[1]);
    
    if (min > max) {
        float temp = min;
        min = max;
        max = temp;
    }
    
    float range = max - min;
    return min + (float)((double)rand() / (double)RAND_MAX) * range;
}

/**
 * @brief 生成随机 double / Generate random double / Zufälliges double generieren
 * @param pack_ptr 参数包指针 / Parameter pack pointer / Parameterpaket-Zeiger
 * @return 随机 double 值 / Random double value / Zufälliger double-Wert
 * 
 * @details
 * 参数：
 * - 无参数：返回 0.0 到 1.0 之间的随机数
 * - 1个参数 (min)：返回 min 到 1.0 之间的随机数
 * - 2个参数 (min, max)：返回 min 到 max 之间的随机数
 */
NXLD_PLUGIN_EXPORT double NXLD_PLUGIN_CALL RandomDouble(void* pack_ptr) {
    pt_param_pack_t* pack = (pt_param_pack_t*)pack_ptr;
    
    init_random();
    
    if (pack == NULL || pack->param_count == 0) {
        /* 无参数：返回 0.0 到 1.0 / No params: return 0.0 to 1.0 / Keine Parameter: 0.0 bis 1.0 zurückgeben */
        return (double)rand() / (double)RAND_MAX;
    }
    
    if (pack->param_count == 1) {
        /* 1个参数：min 到 1.0 / 1 param: min to 1.0 / 1 Parameter: min bis 1.0 */
        double min = convert_to_double(&pack->params[0]);
        double range = 1.0 - min;
        return min + ((double)rand() / (double)RAND_MAX) * range;
    }
    
    /* 2个参数：min 到 max / 2 params: min to max / 2 Parameter: min bis max */
    double min = convert_to_double(&pack->params[0]);
    double max = convert_to_double(&pack->params[1]);
    
    if (min > max) {
        double temp = min;
        min = max;
        max = temp;
    }
    
    double range = max - min;
    return min + ((double)rand() / (double)RAND_MAX) * range;
}

