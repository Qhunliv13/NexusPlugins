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
#include <sys/stat.h>
#include <io.h>
#else
#include <dlfcn.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
#endif

#include "pointer_transfer_currying.h"

/**
 * @brief 加载动态库 / Load dynamic library / Dynamische Bibliothek laden
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
 * @brief 获取动态库符号 / Get dynamic library symbol / Dynamisches Bibliothekssymbol abrufen
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

/**
 * @brief 动态函数调用 / Dynamic function call / Dynamischer Funktionsaufruf
 * @param func_ptr 函数指针 / Function pointer / Funktionszeiger
 * @param param_count 参数数量 / Parameter count / Parameteranzahl
 * @param param_types 参数类型数组 / Parameter types array / Parametertyp-Array
 * @param param_values 参数值数组 / Parameter values array / Parameterwerte-Array
 * @param param_sizes 参数大小数组，可为NULL / Parameter sizes array, can be NULL / Parametergrößen-Array, kann NULL sein
 * @param return_type 返回值类型 / Return value type / Rückgabewerttyp
 * @param return_size 返回值大小 / Return value size / Rückgabewertgröße
 * @param result_int 输出整数返回值指针 / Output integer return value pointer / Ausgabe-Integer-Rückgabewert-Zeiger
 * @param result_float 输出浮点返回值指针 / Output floating-point return value pointer / Ausgabe-Gleitkomma-Rückgabewert-Zeiger
 * @param result_struct 输出结构体返回值缓冲区，可为NULL / Output struct return value buffer, can be NULL / Ausgabe-Struktur-Rückgabewert-Puffer, kann NULL sein
 * @return 成功返回0，错误返回非0 / Returns 0 on success, non-zero on error / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
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
    
    /* 验证参数类型数组 / Validate parameter types array / Parametertyp-Array validieren */
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
 * @brief 平台函数调用 / Platform function call / Plattform-Funktionsaufruf
 * @param func_ptr 函数指针 / Function pointer / Funktionszeiger
 * @param param_count 参数数量 / Parameter count / Parameteranzahl
 * @param param_types 参数类型数组 / Parameter types array / Parametertyp-Array
 * @param param_values 参数值数组 / Parameter values array / Parameterwerte-Array
 * @param param_sizes 参数大小数组，可为NULL / Parameter sizes array, can be NULL / Parametergrößen-Array, kann NULL sein
 * @param return_type 返回值类型 / Return value type / Rückgabewerttyp
 * @param return_size 返回值大小 / Return value size / Rückgabewertgröße
 * @param result_int 输出整数返回值指针 / Output integer return value pointer / Ausgabe-Integer-Rückgabewert-Zeiger
 * @param result_float 输出浮点返回值指针 / Output floating-point return value pointer / Ausgabe-Gleitkomma-Rückgabewert-Zeiger
 * @param result_struct 输出结构体返回值缓冲区，可为NULL / Output struct return value buffer, can be NULL / Ausgabe-Struktur-Rückgabewert-Puffer, kann NULL sein
 * @return 成功返回0，错误返回非0 / Returns 0 on success, non-zero on error / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
 */
int32_t pt_platform_safe_call(void* func_ptr, int param_count, void* param_types, void** param_values, void* param_sizes,
                               pt_return_type_t return_type, size_t return_size, int64_t* result_int, double* result_float, void* result_struct) {
    /* 调用动态函数 / Call dynamic function / Dynamische Funktion aufrufen */
    return call_function_dynamic(func_ptr, param_count, (nxld_param_type_t*)param_types, param_values, (size_t*)param_sizes,
                                 return_type, return_size, result_int, result_float, result_struct);
}

/**
 * @brief 获取文件修改时间戳 / Get file modification timestamp / Dateiänderungszeitstempel abrufen
 * @param file_path 文件路径 / File path / Dateipfad
 * @param timestamp 输出时间戳指针 / Output timestamp pointer / Ausgabe-Zeitstempel-Zeiger
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int32_t pt_platform_get_file_timestamp(const char* file_path, int64_t* timestamp) {
    if (file_path == NULL || timestamp == NULL) {
        return -1;
    }
    
#ifdef _WIN32
    struct _stat64 file_stat;
    if (_stat64(file_path, &file_stat) != 0) {
        return -1;
    }
    *timestamp = (int64_t)file_stat.st_mtime;
#else
    struct stat file_stat;
    if (stat(file_path, &file_stat) != 0) {
        return -1;
    }
    *timestamp = (int64_t)file_stat.st_mtime;
#endif
    
    return 0;
}

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
        return 0; /* 目录为空或不存在，返回0 / Directory is empty or doesn't exist, returns 0 / Verzeichnis ist leer oder existiert nicht, gibt 0 zurück */
    }
    
    do {
        if (strcmp(file_info.name, ".") == 0 || strcmp(file_info.name, "..") == 0) {
            continue;
        }
        
        char full_path[4096];
        size_t path_len = dir_len - 1; /* 移除通配符 '*' / Remove wildcard '*' / Platzhalter '*' entfernen */
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
            /* 递归查找子目录 / Recursively search subdirectory / Unterverzeichnis rekursiv durchsuchen */
            if (*file_count < max_files) {
                find_dll_files_recursive(full_path, dll_files, max_files - *file_count, file_count);
            }
        } else {
            /* 检查DLL文件扩展名 / Check DLL file extension / DLL-Dateierweiterung prüfen */
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
        return 0; /* 目录不存在或无法打开，返回0 / Directory doesn't exist or can't be opened, returns 0 / Verzeichnis existiert nicht oder kann nicht geöffnet werden, gibt 0 zurück */
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
                /* 递归查找子目录 / Recursively search subdirectory / Unterverzeichnis rekursiv durchsuchen */
                find_dll_files_recursive(full_path, dll_files, max_files - *file_count, file_count);
            } else if (S_ISREG(st.st_mode)) {
                /* 检查.so文件扩展名 / Check .so file extension / .so-Dateierweiterung prüfen */
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
    
    /* 从插件路径提取目录 / Extract directory from plugin path / Verzeichnis aus Plugin-Pfad extrahieren */
    char dir_path[4096];
    const char* last_slash = strrchr(plugin_path, '/');
#ifdef _WIN32
    const char* last_backslash = strrchr(plugin_path, '\\');
    if (last_backslash != NULL && (last_slash == NULL || last_backslash > last_slash)) {
        last_slash = last_backslash;
    }
#endif
    
    if (last_slash == NULL) {
        /* 没有路径分隔符，使用当前目录 / No path separator, use current directory / Kein Pfadtrennzeichen, aktuelles Verzeichnis verwenden */
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

