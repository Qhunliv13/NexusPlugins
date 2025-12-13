/**
 * @file path_dll.c
 * @brief 获取当前DLL文件路径函数 / Get current DLL file path function / Aktuellen DLL-Dateipfad abrufen Funktion
 */

#include "pointer_transfer_utils.h"
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

/**
 * @brief 获取当前DLL文件路径 / Get current DLL file path / Aktuellen DLL-Dateipfad abrufen
 * @param path_buffer 输出路径缓冲区 / Output path buffer / Ausgabe-Pfad-Puffer
 * @param buffer_size 缓冲区大小 / Buffer size / Puffergröße
 * @return 成功返回0，失败返回非0 / Returns 0 on success, non-zero on failure / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
 */
int get_current_dll_path(char* path_buffer, size_t buffer_size) {
    if (path_buffer == NULL || buffer_size == 0) {
        return -1;
    }
    
#ifdef _WIN32
    HMODULE hModule = NULL;
    if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                           (LPCSTR)&get_current_dll_path, &hModule) == 0) {
        hModule = GetModuleHandleA(NULL);
        if (hModule == NULL) {
            return -1;
        }
    }
    
    DWORD result = GetModuleFileNameA(hModule, path_buffer, (DWORD)buffer_size);
    if (result == 0 || result >= buffer_size) {
        return -1;
    }
    return 0;
#else
    Dl_info info;
    if (dladdr((void*)&get_current_dll_path, &info) == 0) {
        return -1;
    }
    
    size_t len = strlen(info.dli_fname);
    if (len >= buffer_size) {
        len = buffer_size - 1;
    }
    memcpy(path_buffer, info.dli_fname, len);
    path_buffer[len] = '\0';
    return 0;
#endif
}

