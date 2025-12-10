/**
 * @file test_plugin.c
 * @brief 测试插件实现 / Test Plugin Implementation / Test-Plugin-Implementierung
 * @details 用于测试指针传递插件的示例插件 / Example plugin for testing pointer transfer plugin / Beispiel-Plugin zum Testen des Zeigerübertragungs-Plugins
 */

#include "nxld_plugin_interface.h"
/* 
 * 插件可选择包含类型定义头文件以实现类型安全，但不强制要求
 * 插件函数签名采用void*，参数包结构作为标准ABI约定
 * Plugins may optionally include type definition header for type safety, but it's not required
 * Plugin function signatures use void*, param pack structure as standard ABI convention
 * Plugins können optional Typdefinitions-Header für Typsicherheit einbinden, aber es ist nicht erforderlich
 * Plugin-Funktionssignaturen verwenden void*, Parameterpaket-Struktur als Standard-ABI-Konvention
 */
#include "pointer_transfer_plugin_types.h"  /* 可选，用于类型安全 / Optional, for type safety */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>

#ifdef _WIN32
#define NXLD_PLUGIN_EXPORT __declspec(dllexport)
#define NXLD_PLUGIN_CALL __cdecl
#else
#define NXLD_PLUGIN_EXPORT __attribute__((visibility("default")))
#define NXLD_PLUGIN_CALL
#endif

/* 插件名称 / Plugin name / Plugin-Name */
NXLD_PLUGIN_EXPORT int32_t NXLD_PLUGIN_CALL nxld_plugin_get_name(char* name, size_t name_size) {
    if (name == NULL || name_size == 0) {
        return -1;
    }
    const char* plugin_name = "TestPlugin";
    size_t len = strlen(plugin_name);
    if (len >= name_size) {
        len = name_size - 1;
    }
    memcpy(name, plugin_name, len);
    name[len] = '\0';
    return 0;
}

/* 插件版本 / Plugin version / Plugin-Version */
NXLD_PLUGIN_EXPORT int32_t NXLD_PLUGIN_CALL nxld_plugin_get_version(char* version, size_t version_size) {
    if (version == NULL || version_size == 0) {
        return -1;
    }
    const char* plugin_version = "1.0.0";
    size_t len = strlen(plugin_version);
    if (len >= version_size) {
        len = version_size - 1;
    }
    memcpy(version, plugin_version, len);
    version[len] = '\0';
    return 0;
}

/* 接口数量 / Interface count / Schnittstellenanzahl */
#define INTERFACE_COUNT 13

NXLD_PLUGIN_EXPORT int32_t NXLD_PLUGIN_CALL nxld_plugin_get_interface_count(size_t* count) {
    if (count == NULL) {
        return -1;
    }
    *count = INTERFACE_COUNT;
    return 0;
}

/* 接口名称 / Interface names / Schnittstellennamen */
static const char* interface_names[] = {
    "AddInt",
    "AddDouble",
    "AddFloat",
    "StrLen",
    "MallocWrapper",
    "CreatePoint2D",
    "AddInt64",
    "GetChar",
    "CreatePoint3D",
    "MixedParams",
    "ManyParams",
    "StructParam",
    "VariadicSum"
};

/* 接口描述 / Interface descriptions / Schnittstellenbeschreibungen */
static const char* interface_descriptions[] = {
    "Add two integers, returns integer",
    "Add two doubles, returns double",
    "Add two floats, returns float",
    "Get string length, returns integer",
    "Wrapper for malloc, returns pointer",
    "Create 2D point, returns struct",
    "Add two int64, returns int64",
    "Get character value, returns char",
    "Create 3D point, returns struct",
    "Mixed parameter types, returns integer",
    "Many parameters (>4 for Windows), returns integer",
    "Struct parameter (value), returns integer",
    "Variadic sum function, returns integer"
};

NXLD_PLUGIN_EXPORT int32_t NXLD_PLUGIN_CALL nxld_plugin_get_interface_info(size_t index, 
                                                        char* name, size_t name_size,
                                                        char* description, size_t desc_size,
                                                        char* version, size_t version_size) {
    if (index >= INTERFACE_COUNT || name == NULL || name_size == 0) {
        return -1;
    }
    
    size_t len = strlen(interface_names[index]);
    if (len >= name_size) {
        len = name_size - 1;
    }
    memcpy(name, interface_names[index], len);
    name[len] = '\0';
    
    if (description != NULL && desc_size > 0 && index < sizeof(interface_descriptions)/sizeof(interface_descriptions[0])) {
        len = strlen(interface_descriptions[index]);
        if (len >= desc_size) {
            len = desc_size - 1;
        }
        memcpy(description, interface_descriptions[index], len);
        description[len] = '\0';
    }
    
    if (version != NULL && version_size > 0) {
        const char* iface_version = "1.0.0";
        len = strlen(iface_version);
        if (len >= version_size) {
            len = version_size - 1;
        }
        memcpy(version, iface_version, len);
        version[len] = '\0';
    }
    
    return 0;
}

/* 参数数量信息 / Parameter count information / Parameteranzahl-Informationen */
static const nxld_param_count_type_t param_count_types[] = {
    NXLD_PARAM_COUNT_FIXED,
    NXLD_PARAM_COUNT_FIXED,
    NXLD_PARAM_COUNT_FIXED,
    NXLD_PARAM_COUNT_FIXED,
    NXLD_PARAM_COUNT_FIXED,
    NXLD_PARAM_COUNT_FIXED,
    NXLD_PARAM_COUNT_FIXED,
    NXLD_PARAM_COUNT_FIXED,
    NXLD_PARAM_COUNT_FIXED,
    NXLD_PARAM_COUNT_FIXED,
    NXLD_PARAM_COUNT_FIXED,
    NXLD_PARAM_COUNT_FIXED,
    NXLD_PARAM_COUNT_VARIABLE
};

static const int32_t param_min_counts[] = { 2, 2, 2, 1, 1, 2, 2, 1, 3, 5, 6, 1, 1 };
static const int32_t param_max_counts[] = { 2, 2, 2, 1, 1, 2, 2, 1, 3, 5, 6, 1, -1 };

NXLD_PLUGIN_EXPORT int32_t NXLD_PLUGIN_CALL nxld_plugin_get_interface_param_count(size_t index,
                                                               nxld_param_count_type_t* count_type,
                                                               int32_t* min_count, int32_t* max_count) {
    if (index >= INTERFACE_COUNT || count_type == NULL || min_count == NULL || max_count == NULL) {
        return -1;
    }
    
    *count_type = param_count_types[index];
    *min_count = param_min_counts[index];
    *max_count = param_max_counts[index];
    
    return 0;
}

/* 参数信息 / Parameter information / Parameter-Informationen */
static const char* param_names[][6] = {
    { "a", "b", NULL, NULL, NULL, NULL },
    { "a", "b", NULL, NULL, NULL, NULL },
    { "a", "b", NULL, NULL, NULL, NULL },
    { "str", NULL, NULL, NULL, NULL, NULL },
    { "size", NULL, NULL, NULL, NULL, NULL },
    { "x", "y", NULL, NULL, NULL, NULL },
    { "a", "b", NULL, NULL, NULL, NULL },
    { "c", NULL, NULL, NULL, NULL, NULL },
    { "x", "y", "z", NULL, NULL, NULL },
    { "i", "f", "d", "p", "s", NULL },
    { "a", "b", "c", "d", "e", "f" },
    { "p", NULL, NULL, NULL, NULL, NULL },
    { "count", NULL, NULL, NULL, NULL, NULL }
};

static const nxld_param_type_t param_types[][6] = {
    { NXLD_PARAM_TYPE_INT32, NXLD_PARAM_TYPE_INT32, NXLD_PARAM_TYPE_VOID, NXLD_PARAM_TYPE_VOID, NXLD_PARAM_TYPE_VOID, NXLD_PARAM_TYPE_VOID },
    { NXLD_PARAM_TYPE_DOUBLE, NXLD_PARAM_TYPE_DOUBLE, NXLD_PARAM_TYPE_VOID, NXLD_PARAM_TYPE_VOID, NXLD_PARAM_TYPE_VOID, NXLD_PARAM_TYPE_VOID },
    { NXLD_PARAM_TYPE_FLOAT, NXLD_PARAM_TYPE_FLOAT, NXLD_PARAM_TYPE_VOID, NXLD_PARAM_TYPE_VOID, NXLD_PARAM_TYPE_VOID, NXLD_PARAM_TYPE_VOID },
    { NXLD_PARAM_TYPE_STRING, NXLD_PARAM_TYPE_VOID, NXLD_PARAM_TYPE_VOID, NXLD_PARAM_TYPE_VOID, NXLD_PARAM_TYPE_VOID, NXLD_PARAM_TYPE_VOID },
    { NXLD_PARAM_TYPE_INT64, NXLD_PARAM_TYPE_VOID, NXLD_PARAM_TYPE_VOID, NXLD_PARAM_TYPE_VOID, NXLD_PARAM_TYPE_VOID, NXLD_PARAM_TYPE_VOID },
    { NXLD_PARAM_TYPE_INT32, NXLD_PARAM_TYPE_INT32, NXLD_PARAM_TYPE_VOID, NXLD_PARAM_TYPE_VOID, NXLD_PARAM_TYPE_VOID, NXLD_PARAM_TYPE_VOID },
    { NXLD_PARAM_TYPE_INT64, NXLD_PARAM_TYPE_INT64, NXLD_PARAM_TYPE_VOID, NXLD_PARAM_TYPE_VOID, NXLD_PARAM_TYPE_VOID, NXLD_PARAM_TYPE_VOID },
    { NXLD_PARAM_TYPE_CHAR, NXLD_PARAM_TYPE_VOID, NXLD_PARAM_TYPE_VOID, NXLD_PARAM_TYPE_VOID, NXLD_PARAM_TYPE_VOID, NXLD_PARAM_TYPE_VOID },
    { NXLD_PARAM_TYPE_INT32, NXLD_PARAM_TYPE_INT32, NXLD_PARAM_TYPE_INT32, NXLD_PARAM_TYPE_VOID, NXLD_PARAM_TYPE_VOID, NXLD_PARAM_TYPE_VOID },
    { NXLD_PARAM_TYPE_INT32, NXLD_PARAM_TYPE_FLOAT, NXLD_PARAM_TYPE_DOUBLE, NXLD_PARAM_TYPE_POINTER, NXLD_PARAM_TYPE_STRING, NXLD_PARAM_TYPE_VOID },
    { NXLD_PARAM_TYPE_INT32, NXLD_PARAM_TYPE_INT32, NXLD_PARAM_TYPE_INT32, NXLD_PARAM_TYPE_INT32, NXLD_PARAM_TYPE_INT32, NXLD_PARAM_TYPE_INT32 },
    { NXLD_PARAM_TYPE_POINTER, NXLD_PARAM_TYPE_VOID, NXLD_PARAM_TYPE_VOID, NXLD_PARAM_TYPE_VOID, NXLD_PARAM_TYPE_VOID, NXLD_PARAM_TYPE_VOID },
    { NXLD_PARAM_TYPE_INT32, NXLD_PARAM_TYPE_VARIADIC, NXLD_PARAM_TYPE_VOID, NXLD_PARAM_TYPE_VOID, NXLD_PARAM_TYPE_VOID, NXLD_PARAM_TYPE_VOID }
};

NXLD_PLUGIN_EXPORT int32_t NXLD_PLUGIN_CALL nxld_plugin_get_interface_param_info(size_t index, int32_t param_index,
                                                              char* param_name, size_t name_size,
                                                              nxld_param_type_t* param_type,
                                                              char* type_name, size_t type_name_size) {
    if (index >= INTERFACE_COUNT || param_index < 0 || param_index >= param_max_counts[index] ||
        param_name == NULL || name_size == 0 || param_type == NULL) {
        return -1;
    }
    
    if (param_index < 5 && param_names[index][param_index] != NULL) {
        size_t len = strlen(param_names[index][param_index]);
        if (len >= name_size) {
            len = name_size - 1;
        }
        memcpy(param_name, param_names[index][param_index], len);
        param_name[len] = '\0';
    } else {
        param_name[0] = '\0';
    }
    
    if (param_index < 5) {
        *param_type = param_types[index][param_index];
    } else {
        *param_type = NXLD_PARAM_TYPE_VOID;
    }
    
    if (type_name != NULL && type_name_size > 0) {
        type_name[0] = '\0';
    }
    
    return 0;
}

/* 测试函数实现 / Test function implementations / Testfunktions-Implementierungen */
typedef int32_t (*AddIntFunc)(int32_t, int32_t);
typedef double (*AddDoubleFunc)(double, double);
typedef float (*AddFloatFunc)(float, float);
typedef int32_t (*StrLenFunc)(const char*);
typedef void* (*MallocWrapperFunc)(size_t);

typedef struct {
    int32_t x;
    int32_t y;
} Point2D;

typedef Point2D (*CreatePoint2DFunc)(int32_t, int32_t);

/* 导出函数 / Exported functions / Exportierte Funktionen */
/* 所有函数现在接受void*参数包指针 / All functions now accept void* param pack pointer */
/* 函数签名采用void*以避免头文件依赖 / Function signatures use void* to avoid header dependencies / Funktionssignaturen verwenden void*, um Header-Abhängigkeiten zu vermeiden */
NXLD_PLUGIN_EXPORT int32_t NXLD_PLUGIN_CALL AddInt(void* pack_ptr) {
    pt_param_pack_t* pack = (pt_param_pack_t*)pack_ptr;  /* 类型转换（可选头文件提供类型） / Type cast (optional header provides type) */
    if (pack == NULL || pack->param_count < 2) {
        return 0;
    }
    int32_t a = pack->params[0].value.int32_val;
    int32_t b = pack->params[1].value.int32_val;
    return a + b;
}

NXLD_PLUGIN_EXPORT double NXLD_PLUGIN_CALL AddDouble(void* pack_ptr) {
    pt_param_pack_t* pack = (pt_param_pack_t*)pack_ptr;
    if (pack == NULL || pack->param_count < 2) {
        return 0.0;
    }
    double a = pack->params[0].value.double_val;
    double b = pack->params[1].value.double_val;
    return a + b;
}

NXLD_PLUGIN_EXPORT float NXLD_PLUGIN_CALL AddFloat(void* pack_ptr) {
    pt_param_pack_t* pack = (pt_param_pack_t*)pack_ptr;
    if (pack == NULL || pack->param_count < 2) {
        return 0.0f;
    }
    float a = pack->params[0].value.float_val;
    float b = pack->params[1].value.float_val;
    return a + b;
}

NXLD_PLUGIN_EXPORT int32_t NXLD_PLUGIN_CALL StrLen(void* pack_ptr) {
    pt_param_pack_t* pack = (pt_param_pack_t*)pack_ptr;
    if (pack == NULL || pack->param_count < 1) {
        return 0;
    }
    const char* str = (const char*)pack->params[0].value.ptr_val;
    if (str == NULL) {
        return 0;
    }
    return (int32_t)strlen(str);
}

NXLD_PLUGIN_EXPORT void* NXLD_PLUGIN_CALL MallocWrapper(void* pack_ptr) {
    pt_param_pack_t* pack = (pt_param_pack_t*)pack_ptr;
    if (pack == NULL || pack->param_count < 1) {
        return NULL;
    }
    size_t size = (size_t)pack->params[0].value.int64_val;
    return malloc(size);
}

NXLD_PLUGIN_EXPORT Point2D NXLD_PLUGIN_CALL CreatePoint2D(void* pack_ptr) {
    pt_param_pack_t* pack = (pt_param_pack_t*)pack_ptr;
    Point2D p = {0, 0};
    if (pack == NULL || pack->param_count < 2) {
        return p;
    }
    p.x = pack->params[0].value.int32_val;
    p.y = pack->params[1].value.int32_val;
    return p;
}

/* 新增测试函数 / New test functions / Neue Testfunktionen */
NXLD_PLUGIN_EXPORT int64_t NXLD_PLUGIN_CALL AddInt64(void* pack_ptr) {
    pt_param_pack_t* pack = (pt_param_pack_t*)pack_ptr;
    if (pack == NULL || pack->param_count < 2) {
        return 0;
    }
    int64_t a = pack->params[0].value.int64_val;
    int64_t b = pack->params[1].value.int64_val;
    return a + b;
}

NXLD_PLUGIN_EXPORT char NXLD_PLUGIN_CALL GetChar(void* pack_ptr) {
    pt_param_pack_t* pack = (pt_param_pack_t*)pack_ptr;
    if (pack == NULL || pack->param_count < 1) {
        return 0;
    }
    return pack->params[0].value.char_val;
}

/* Point3D结构体定义 / Point3D structure definition / Point3D-Strukturdefinition */
typedef struct {
    int32_t x;
    int32_t y;
    int32_t z;
} Point3D;

NXLD_PLUGIN_EXPORT Point3D NXLD_PLUGIN_CALL CreatePoint3D(void* pack_ptr) {
    pt_param_pack_t* pack = (pt_param_pack_t*)pack_ptr;
    Point3D p = {0, 0, 0};
    if (pack == NULL || pack->param_count < 3) {
        return p;
    }
    p.x = pack->params[0].value.int32_val;
    p.y = pack->params[1].value.int32_val;
    p.z = pack->params[2].value.int32_val;
    return p;
}

NXLD_PLUGIN_EXPORT int32_t NXLD_PLUGIN_CALL MixedParams(void* pack_ptr) {
    pt_param_pack_t* pack = (pt_param_pack_t*)pack_ptr;
    if (pack == NULL || pack->param_count < 5) {
        return 0;
    }
    int32_t i = pack->params[0].value.int32_val;
    float f = pack->params[1].value.float_val;
    double d = pack->params[2].value.double_val;
    void* p = pack->params[3].value.ptr_val;
    const char* s = (const char*)pack->params[4].value.ptr_val;
    /* 避免未使用变量警告 / Avoid unused variable warning / Nicht verwendete Variablen-Warnung vermeiden */
    (void)p;
    if (s != NULL && strlen(s) > 0) {
        return (int32_t)(i + (int32_t)f + (int32_t)d + (int32_t)strlen(s));
    }
    return (int32_t)(i + (int32_t)f + (int32_t)d);
}

/* 超过寄存器数量的参数测试（Windows：大于4个参数）/ Test for parameters exceeding register count (Windows: >4 params) / Test für Parameter über Registeranzahl (Windows: >4 Parameter) */
NXLD_PLUGIN_EXPORT int32_t NXLD_PLUGIN_CALL ManyParams(void* pack_ptr) {
    pt_param_pack_t* pack = (pt_param_pack_t*)pack_ptr;
    if (pack == NULL || pack->param_count < 6) {
        return 0;
    }
    int32_t a = pack->params[0].value.int32_val;
    int32_t b = pack->params[1].value.int32_val;
    int32_t c = pack->params[2].value.int32_val;
    int32_t d = pack->params[3].value.int32_val;
    int32_t e = pack->params[4].value.int32_val;
    int32_t f = pack->params[5].value.int32_val;
    return a + b + c + d + e + f;
}

/* 结构体参数（值传递）测试 / Test struct parameter (value passing) / Strukturparameter-Test (Wertübergabe) */
NXLD_PLUGIN_EXPORT int32_t NXLD_PLUGIN_CALL StructParam(void* pack_ptr) {
    pt_param_pack_t* pack = (pt_param_pack_t*)pack_ptr;
    if (pack == NULL || pack->param_count < 1) {
        return 0;
    }
    /* 结构体参数通过指针传递 / Struct parameter passed via pointer / Strukturparameter über Zeiger übergeben */
    Point2D* p = (Point2D*)pack->params[0].value.ptr_val;
    if (p == NULL) {
        return 0;
    }
    return p->x + p->y;
}

/* 可变参数测试 / Variadic parameter test / Variabler Parameter-Test */
NXLD_PLUGIN_EXPORT int32_t NXLD_PLUGIN_CALL VariadicSum(void* pack_ptr) {
    pt_param_pack_t* pack = (pt_param_pack_t*)pack_ptr;
    if (pack == NULL || pack->param_count < 1) {
        return 0;
    }
    int32_t count = pack->params[0].value.int32_val;
    int32_t sum = 0;
    for (int32_t i = 1; i <= count && i < pack->param_count; i++) {
        sum += pack->params[i].value.int32_val;
    }
    return sum;
}

