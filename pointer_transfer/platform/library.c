/**
 * @file library.c
 * @brief 动态库加载和符号获取函数 / Dynamic library loading and symbol retrieval functions / Dynamische Bibliothekslade- und Symbolabruf-Funktionen
 */

#include "pointer_transfer_platform.h"
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

/**
 * @brief 加载动态库文件 / Load dynamic library file / Dynamische Bibliotheksdatei laden
 * @param plugin_path 插件文件路径 / Plugin file path / Plugin-Dateipfad
 * @return 动态库句柄，失败返回NULL / Dynamic library handle, NULL on failure / Dynamisches Bibliothekshandle, NULL bei Fehler
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
 * @brief 获取动态库中的符号地址 / Get symbol address in dynamic library / Symboladresse in dynamischer Bibliothek abrufen
 * @param handle 动态库句柄 / Dynamic library handle / Dynamisches Bibliothekshandle
 * @param symbol_name 符号名称 / Symbol name / Symbolname
 * @return 符号地址，失败返回NULL / Symbol address, NULL on failure / Symboladresse, NULL bei Fehler
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
 * @param handle 动态库句柄 / Dynamic library handle / Dynamisches Bibliothekshandle
 * @return 成功返回0，失败返回非0 / Returns 0 on success, non-zero on failure / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
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

