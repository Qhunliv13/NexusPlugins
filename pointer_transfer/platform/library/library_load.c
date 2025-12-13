/**
 * @file library_load.c
 * @brief 动态库加载 / Dynamic library loading / Dynamische Bibliotheksladung
 */

#include "pointer_transfer_platform.h"
#include <stddef.h>

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

