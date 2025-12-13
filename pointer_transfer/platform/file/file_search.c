/**
 * @file file_search.c
 * @brief 文件搜索操作 / File search operations / Dateisuche-Operationen
 */

#include "pointer_transfer_platform.h"
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <io.h>
#else
#include <sys/stat.h>
#include <dirent.h>
#endif

/**
 * @brief 递归查找目录下的所有动态链接库文件 / Recursively find all dynamic library files in directory / Alle dynamischen Bibliotheksdateien im Verzeichnis rekursiv finden
 * @param dir_path 目录路径 / Directory path / Verzeichnispfad
 * @param dll_files 输出DLL文件路径数组 / Output DLL file paths array / Ausgabe-DLL-Dateipfad-Array
 * @param max_files 最大文件数量 / Maximum file count / Maximale Dateianzahl
 * @param file_count 输出找到的文件数量 / Output found file count / Ausgabe gefundene Dateianzahl
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
static int find_dll_files_recursive(const char* dir_path, char dll_files[][1024], int max_files, int* file_count) {
    if (dir_path == NULL || dll_files == NULL || max_files <= 0 || file_count == NULL) {
        return -1;
    }
    
#ifdef _WIN32
    char search_path[4096];
    size_t dir_len = strlen(dir_path);
    if (dir_len >= sizeof(search_path) - 3) {
        return -1;
    }
    
    memcpy(search_path, dir_path, dir_len);
    if (dir_path[dir_len - 1] != '\\' && dir_path[dir_len - 1] != '/') {
        search_path[dir_len++] = '\\';
    }
    search_path[dir_len++] = '*';
    search_path[dir_len] = '\0';
    
    struct _finddata_t file_info;
    intptr_t handle = _findfirst(search_path, &file_info);
    if (handle == -1) {
        return 0;
    }
    
    do {
        if (strcmp(file_info.name, ".") == 0 || strcmp(file_info.name, "..") == 0) {
            continue;
        }
        
        char full_path[4096];
        size_t path_len = dir_len - 1;
        if (path_len >= sizeof(full_path)) {
            continue;
        }
        memcpy(full_path, search_path, path_len);
        full_path[path_len] = '\0';
        
        size_t name_len = strlen(file_info.name);
        if (path_len + name_len + 1 >= sizeof(full_path)) {
            continue;
        }
        memcpy(full_path + path_len, file_info.name, name_len + 1);
        
        if (file_info.attrib & _A_SUBDIR) {
            if (*file_count < max_files) {
                find_dll_files_recursive(full_path, dll_files, max_files - *file_count, file_count);
            }
        } else {
            size_t name_len_check = strlen(file_info.name);
            if (name_len_check >= 4) {
                const char* ext = file_info.name + name_len_check - 4;
                if (_stricmp(ext, ".dll") == 0) {
                    if (*file_count < max_files) {
                        size_t copy_len = strlen(full_path);
                        if (copy_len < sizeof(dll_files[*file_count])) {
                            memcpy(dll_files[*file_count], full_path, copy_len + 1);
                            (*file_count)++;
                        }
                    }
                }
            }
        }
    } while (_findnext(handle, &file_info) == 0 && *file_count < max_files);
    
    _findclose(handle);
#else
    DIR* dir = opendir(dir_path);
    if (dir == NULL) {
        return 0;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL && *file_count < max_files) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        char full_path[4096];
        size_t dir_len = strlen(dir_path);
        size_t name_len = strlen(entry->d_name);
        
        if (dir_len + name_len + 2 >= sizeof(full_path)) {
            continue;
        }
        
        memcpy(full_path, dir_path, dir_len);
        if (dir_path[dir_len - 1] != '/') {
            full_path[dir_len++] = '/';
        }
        memcpy(full_path + dir_len, entry->d_name, name_len + 1);
        
        struct stat st;
        if (stat(full_path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                find_dll_files_recursive(full_path, dll_files, max_files - *file_count, file_count);
            } else if (S_ISREG(st.st_mode)) {
                size_t name_len_check = strlen(entry->d_name);
                if (name_len_check >= 3) {
                    const char* ext = entry->d_name + name_len_check - 3;
                    if (strcmp(ext, ".so") == 0) {
                        if (*file_count < max_files) {
                            size_t copy_len = strlen(full_path);
                            if (copy_len < sizeof(dll_files[*file_count])) {
                                memcpy(dll_files[*file_count], full_path, copy_len + 1);
                                (*file_count)++;
                            }
                        }
                    }
                }
            }
        }
    }
    
    closedir(dir);
#endif
    
    return 0;
}

/**
 * @brief 从插件路径获取目录并递归查找所有DLL文件 / Get directory from plugin path and recursively find all DLL files / Verzeichnis aus Plugin-Pfad abrufen und alle DLL-Dateien rekursiv finden
 * @param plugin_path 插件文件路径 / Plugin file path / Plugin-Dateipfad
 * @param dll_files 输出DLL文件路径数组 / Output DLL file paths array / Ausgabe-DLL-Dateipfad-Array
 * @param max_files 最大文件数量 / Maximum file count / Maximale Dateianzahl
 * @param file_count 输出找到的文件数量 / Output found file count / Ausgabe gefundene Dateianzahl
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int32_t pt_platform_find_all_dll_files(const char* plugin_path, char dll_files[][1024], int max_files, int* file_count) {
    if (plugin_path == NULL || dll_files == NULL || max_files <= 0 || file_count == NULL) {
        return -1;
    }
    
    *file_count = 0;
    
    char dir_path[4096];
    const char* last_slash = strrchr(plugin_path, '/');
#ifdef _WIN32
    const char* last_backslash = strrchr(plugin_path, '\\');
    if (last_backslash != NULL && (last_slash == NULL || last_backslash > last_slash)) {
        last_slash = last_backslash;
    }
#endif
    
    if (last_slash == NULL) {
        dir_path[0] = '.';
        dir_path[1] = '\0';
    } else {
        size_t dir_len = last_slash - plugin_path;
        if (dir_len >= sizeof(dir_path)) {
            dir_len = sizeof(dir_path) - 1;
        }
        memcpy(dir_path, plugin_path, dir_len);
        dir_path[dir_len] = '\0';
    }
    
    return find_dll_files_recursive(dir_path, dll_files, max_files, file_count);
}

