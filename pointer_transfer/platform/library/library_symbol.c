/**
 * @file library_symbol.c
 * @brief 动态库符号获取 / Dynamic library symbol retrieval / Dynamisches Bibliothekssymbol-Abruf
 */

#include "pointer_transfer_platform.h"
#include <stddef.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

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

