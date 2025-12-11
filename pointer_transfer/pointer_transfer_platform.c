/**
 * @file pointer_transfer_platform.c
 * @brief 平台抽象层 / Platform Abstraction Layer / Plattform-Abstraktionsebene
 */

#include "pointer_transfer_platform.h"
#include "pointer_transfer_utils.h"
#include "nxld_plugin_interface.h"
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include "pointer_transfer_currying.h"

/**
 * @brief 加载动态库 / Load dynamic library / Dynamische Bibliothek laden
 */
void* pt_platform_load_library(const char* plugin_path) {
    if (plugin_path == NULL) {
        return NULL;
    }
    
#ifdef _WIN32
    return (void*)LoadLibraryA(plugin_path);
#else
    return dlopen(plugin_path, RTLD_LAZY);
#endif
}

/**
 * @brief 获取动态库符号 / Get dynamic library symbol / Dynamisches Bibliothekssymbol abrufen
 */
void* pt_platform_get_symbol(void* handle, const char* symbol_name) {
    if (handle == NULL || symbol_name == NULL) {
        return NULL;
    }
    
#ifdef _WIN32
    return (void*)GetProcAddress((HMODULE)handle, symbol_name);
#else
    return dlsym(handle, symbol_name);
#endif
}

/**
 * @brief 关闭动态库 / Close dynamic library / Dynamische Bibliothek schließen
 */
int32_t pt_platform_close_library(void* handle) {
    if (handle == NULL) {
        return 0;
    }
    
#ifdef _WIN32
    return FreeLibrary((HMODULE)handle) ? 0 : 1;
#else
    return dlclose(handle);
#endif
}

/**
 * @brief 动态函数调用 / Dynamic function call / Dynamischer Funktionsaufruf
 * @details 通过静态验证预防错误，不使用异常处理 / Prevents errors through static validation, no exception handling / Verhindert Fehler durch statische Validierung, keine Ausnahmebehandlung
 */
static int32_t call_function_dynamic(void* func_ptr, int param_count, nxld_param_type_t* param_types, void** param_values, size_t* param_sizes,
                                      pt_return_type_t return_type, size_t return_size, int64_t* result_int, double* result_float, void* result_struct) {
    /* 静态参数验证 / Static parameter validation / Statische Parametervalidierung */
    if (func_ptr == NULL) {
        return -1;
    }
    if (param_count < 0) {
        return -1;
    }
    if (param_count > 0 && (param_types == NULL || param_values == NULL)) {
        return -1;
    }
    if (result_int == NULL || result_float == NULL) {
        return -1;
    }
    
    /* 验证参数类型数组的有效性 / Validate parameter types array validity / Gültigkeit des Parametertyp-Arrays validieren */
    if (param_count > 0) {
        for (int i = 0; i < param_count; i++) {
            if (param_types[i] < NXLD_PARAM_TYPE_VOID || param_types[i] > NXLD_PARAM_TYPE_UNKNOWN) {
                return -1;
            }
            /* 验证指针类型参数的大小 / Validate size for pointer type parameters / Größe für Zeigertyp-Parameter validieren */
            if ((param_types[i] == NXLD_PARAM_TYPE_POINTER || 
                 param_types[i] == NXLD_PARAM_TYPE_STRING ||
                 param_types[i] == NXLD_PARAM_TYPE_ANY) && 
                param_sizes != NULL && param_sizes[i] == 0 && param_values[i] != NULL) {
                return -1;
            }
        }
    }
    
    /* 验证返回值类型 / Validate return type / Rückgabetyp validieren */
    if (return_type < PT_RETURN_TYPE_INTEGER || return_type > PT_RETURN_TYPE_STRUCT_VAL) {
        return -1;
    }
    
    /* 验证结构体返回值缓冲区 / Validate struct return value buffer / Struktur-Rückgabewert-Puffer validieren */
    /* PT_RETURN_TYPE_STRUCT_PTR: 小结构体通过指针返回（RAX包含指针），return_size和result_struct可选 / Small struct returned via pointer (RAX contains pointer), return_size and result_struct optional / Kleine Struktur über Zeiger zurückgegeben (RAX enthält Zeiger), return_size und result_struct optional */
    /* PT_RETURN_TYPE_STRUCT_VAL: 大结构体通过隐藏指针参数返回，需要return_size和result_struct / Large struct returned via hidden pointer parameter, requires return_size and result_struct / Große Struktur über versteckten Zeigerparameter zurückgegeben, benötigt return_size und result_struct */
    if (return_type == PT_RETURN_TYPE_STRUCT_VAL) {
        if (return_size == 0) {
            return -1;
        }
        if (result_struct == NULL) {
            return -1;
        }
    }
    
    pt_param_pack_t* pack = pt_create_param_pack(param_count, param_types, param_values, param_sizes);
    if (pack == NULL) {
        return -1;
    }
    
    /* 验证参数包 / Validate parameter pack / Parameterpaket validieren */
    if (pt_validate_param_pack(pack) != 0) {
        pt_free_param_pack(pack);
        return -1;
    }
    
    int32_t result = pt_call_with_currying(func_ptr, pack, return_type, return_size, result_int, result_float, result_struct);
    
    pt_free_param_pack(pack);
    return result;
}

/**
 * @brief 平台函数调用（通过静态验证预防错误）/ Platform function call (prevent errors through static validation) / Plattform-Funktionsaufruf (Fehler durch statische Validierung verhindern)
 * @details 通过严格的参数验证和静态检查预防错误，不使用异常处理机制 / Prevents errors through strict parameter validation and static checks, no exception handling mechanism / Verhindert Fehler durch strenge Parametervalidierung und statische Prüfungen, kein Ausnahmebehandlungsmechanismus
 */
int32_t pt_platform_safe_call(void* func_ptr, int param_count, void* param_types, void** param_values, void* param_sizes,
                               pt_return_type_t return_type, size_t return_size, int64_t* result_int, double* result_float, void* result_struct) {
    /* 直接调用动态函数，内部已包含完整的静态验证 / Directly call dynamic function, which includes complete static validation / Direkter Aufruf der dynamischen Funktion, die vollständige statische Validierung enthält */
    return call_function_dynamic(func_ptr, param_count, (nxld_param_type_t*)param_types, param_values, (size_t*)param_sizes,
                                 return_type, return_size, result_int, result_float, result_struct);
}

