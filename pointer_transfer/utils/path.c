/**
 * @file path.c
 * @brief 路径操作函数 / Path operation functions / Pfad-Operationsfunktionen
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

/**
 * @brief 构建.nxpt配置文件路径 / Build .nxpt configuration file path / .nxpt-Konfigurationsdateipfad erstellen
 * @param file_path 源文件路径 / Source file path / Quelldateipfad
 * @param nxpt_path 输出.nxpt路径缓冲区 / Output .nxpt path buffer / Ausgabe-.nxpt-Pfad-Puffer
 * @param buffer_size 缓冲区大小 / Buffer size / Puffergröße
 * @return 成功返回0，失败返回非0 / Returns 0 on success, non-zero on failure / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
 */
int build_nxpt_path(const char* file_path, char* nxpt_path, size_t buffer_size) {
    if (file_path == NULL || nxpt_path == NULL || buffer_size == 0) {
        return -1;
    }
    
    const char* ext_pos = strrchr(file_path, '.');
    
    size_t base_len;
    if (ext_pos != NULL) {
        base_len = ext_pos - file_path;
    } else {
        base_len = strlen(file_path);
    }
    
    if (base_len + 5 >= buffer_size) {
        return -1;
    }
    
    memcpy(nxpt_path, file_path, base_len);
    memcpy(nxpt_path + base_len, ".nxpt", 6);
    
    return 0;
}

