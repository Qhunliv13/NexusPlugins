/**
 * @file file_timestamp.c
 * @brief 文件时间戳操作 / File timestamp operations / Datei-Zeitstempel-Operationen
 */

#include "pointer_transfer_platform.h"
#include <stddef.h>
#include <stdint.h>

#ifdef _WIN32
#include <sys/stat.h>
#include <io.h>
#else
#include <sys/stat.h>
#include <time.h>
#endif

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

