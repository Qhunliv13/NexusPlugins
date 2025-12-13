/**
 * @file library_close.c
 * @brief 动态库关闭 / Dynamic library closing / Dynamische Bibliotheksschließung
 */

#include "pointer_transfer_platform.h"
#include <stddef.h>
#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

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

