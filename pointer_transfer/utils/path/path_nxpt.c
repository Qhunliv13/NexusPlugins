/**
 * @file path_nxpt.c
 * @brief 构建.nxpt配置文件路径函数 / Build .nxpt configuration file path function / .nxpt-Konfigurationsdateipfad erstellen Funktion
 */

#include "pointer_transfer_utils.h"
#include <string.h>

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

