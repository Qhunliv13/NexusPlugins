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
#include <signal.h>
#include <setjmp.h>

/* 信号处理跳转缓冲区 / Signal handling jump buffer / Sprungpuffer für Signalbehandlung */
static sigjmp_buf g_jmp_buf;
static volatile int g_signal_caught = 0;

/**
 * @brief 信号处理函数 / Signal handler function / Signalhandler-Funktion
 */
static void signal_handler(int sig) {
    (void)sig;
    g_signal_caught = 1;
    siglongjmp(g_jmp_buf, 1);
}
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
 */
static int32_t call_function_dynamic(void* func_ptr, int param_count, nxld_param_type_t* param_types, void** param_values, size_t* param_sizes,
                                      pt_return_type_t return_type, size_t return_size, int64_t* result_int, double* result_float, void* result_struct) {
    if (func_ptr == NULL || param_count < 0 || param_values == NULL || result_int == NULL || result_float == NULL) {
        return -1;
    }
    
    pt_param_pack_t* pack = pt_create_param_pack(param_count, param_types, param_values, param_sizes);
    if (pack == NULL) {
        return -1;
    }
    
    int32_t result = pt_call_with_currying(func_ptr, pack, return_type, return_size, result_int, result_float, result_struct);
    
    pt_free_param_pack(pack);
    return result;
}

/**
 * @brief 带异常处理的函数调用 / Function call with exception handling / Funktionsaufruf mit Ausnahmebehandlung
 */
int32_t pt_platform_safe_call(void* func_ptr, int param_count, void* param_types, void** param_values, void* param_sizes,
                               pt_return_type_t return_type, size_t return_size, int64_t* result_int, double* result_float, void* result_struct) {
    if (func_ptr == NULL || result_int == NULL || result_float == NULL) {
        return -1;
    }
    
#ifdef _WIN32
    __try {
        return call_function_dynamic(func_ptr, param_count, (nxld_param_type_t*)param_types, param_values, (size_t*)param_sizes,
                                     return_type, return_size, result_int, result_float, result_struct);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        *result_int = 0;
        *result_float = 0.0;
        return -1;
    }
#else
    struct sigaction old_action;
    struct sigaction new_action;
    
    memset(&new_action, 0, sizeof(new_action));
    new_action.sa_handler = signal_handler;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = SA_NODEFER;
    
    if (sigaction(SIGSEGV, &new_action, &old_action) != 0) {
        return call_function_dynamic(func_ptr, param_count, (nxld_param_type_t*)param_types, param_values, (size_t*)param_sizes,
                                     return_type, return_size, result_int, result_float, result_struct);
    }
    
    g_signal_caught = 0;
    if (sigsetjmp(g_jmp_buf, 1) == 0) {
        int32_t ret = call_function_dynamic(func_ptr, param_count, (nxld_param_type_t*)param_types, param_values, (size_t*)param_sizes,
                                             return_type, return_size, result_int, result_float, result_struct);
        sigaction(SIGSEGV, &old_action, NULL);
        return ret;
    } else {
        sigaction(SIGSEGV, &old_action, NULL);
        *result_int = 0;
        *result_float = 0.0;
        return -1;
    }
#endif
}

