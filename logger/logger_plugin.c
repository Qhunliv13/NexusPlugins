/**
 * @file logger_plugin.c
 * @brief 日志插件实现 / Logger Plugin Implementation / Logger-Plugin-Implementierung
 * @details 通用的日志写入插件，仅进行文件写入操作 / Generic log writing plugin, only performs file write operations / Generisches Log-Schreib-Plugin, führt nur Dateischreibvorgänge durch
 */

#include "nxld_plugin_interface.h"
#include "pointer_transfer_plugin_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#ifdef _WIN32
#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <windows.h>
#endif

#ifdef _WIN32
#define NXLD_PLUGIN_EXPORT __declspec(dllexport)
#define NXLD_PLUGIN_CALL __cdecl
#else
#define NXLD_PLUGIN_EXPORT __attribute__((visibility("default")))
#define NXLD_PLUGIN_CALL
#endif

#define PLUGIN_NAME "LoggerPlugin"
#define PLUGIN_VERSION "1.0.0"
#define INTERFACE_COUNT 1

#ifdef _WIN32
#define DEFAULT_LOG_FILE_TEMPLATE "%USERPROFILE%\\Desktop\\nexus_engine.log"
#else
#define DEFAULT_LOG_FILE_TEMPLATE "nexus_engine.log"
#endif
#define MAX_LOG_MESSAGE_SIZE 8192
#define MAX_FILE_PATH_SIZE 4096

static const char* interface_name = "WriteLog";
static const char* interface_description = "Write log message to file (message: string, file_path: optional string), returns int32 (0=success, -1=failure)";
static const char* interface_version = "1.0.0";

/**
 * @brief 获取插件名称 / Get plugin name / Plugin-Namen abrufen
 */
NXLD_PLUGIN_EXPORT int32_t NXLD_PLUGIN_CALL nxld_plugin_get_name(char* name, size_t name_size) {
    if (name == NULL || name_size == 0) {
        return -1;
    }
    size_t len = strlen(PLUGIN_NAME);
    if (len >= name_size) {
        len = name_size - 1;
    }
    memcpy(name, PLUGIN_NAME, len);
    name[len] = '\0';
    return 0;
}

/**
 * @brief 获取插件版本 / Get plugin version / Plugin-Version abrufen
 */
NXLD_PLUGIN_EXPORT int32_t NXLD_PLUGIN_CALL nxld_plugin_get_version(char* version, size_t version_size) {
    if (version == NULL || version_size == 0) {
        return -1;
    }
    size_t len = strlen(PLUGIN_VERSION);
    if (len >= version_size) {
        len = version_size - 1;
    }
    memcpy(version, PLUGIN_VERSION, len);
    version[len] = '\0';
    return 0;
}

/**
 * @brief 获取接口数量 / Get interface count / Schnittstellenanzahl abrufen
 */
NXLD_PLUGIN_EXPORT int32_t NXLD_PLUGIN_CALL nxld_plugin_get_interface_count(size_t* count) {
    if (count == NULL) {
        return -1;
    }
    *count = INTERFACE_COUNT;
    return 0;
}

/**
 * @brief 获取接口信息 / Get interface information / Schnittstelleninformationen abrufen
 */
NXLD_PLUGIN_EXPORT int32_t NXLD_PLUGIN_CALL nxld_plugin_get_interface_info(size_t index, 
                                                        char* name, size_t name_size,
                                                        char* description, size_t desc_size,
                                                        char* version, size_t version_size) {
    if (index >= INTERFACE_COUNT) {
        return -1;
    }
    
    if (name != NULL && name_size > 0) {
        size_t len = strlen(interface_name);
        if (len >= name_size) {
            len = name_size - 1;
        }
        memcpy(name, interface_name, len);
        name[len] = '\0';
    }
    
    if (description != NULL && desc_size > 0) {
        size_t len = strlen(interface_description);
        if (len >= desc_size) {
            len = desc_size - 1;
        }
        memcpy(description, interface_description, len);
        description[len] = '\0';
    }
    
    if (version != NULL && version_size > 0) {
        size_t len = strlen(interface_version);
        if (len >= version_size) {
            len = version_size - 1;
        }
        memcpy(version, interface_version, len);
        version[len] = '\0';
    }
    
    return 0;
}

/**
 * @brief 获取接口参数数量 / Get interface parameter count / Schnittstellenparameteranzahl abrufen
 */
NXLD_PLUGIN_EXPORT int32_t NXLD_PLUGIN_CALL nxld_plugin_get_interface_param_count(size_t index,
                                                               nxld_param_count_type_t* count_type,
                                                               int32_t* min_count, int32_t* max_count) {
    if (index >= INTERFACE_COUNT || count_type == NULL || min_count == NULL || max_count == NULL) {
        return -1;
    }
    
    *count_type = NXLD_PARAM_COUNT_VARIABLE;
    *min_count = 1;
    *max_count = 2;
    
    return 0;
}

/**
 * @brief 获取接口参数信息 / Get interface parameter information / Schnittstellenparameterinformationen abrufen
 */
NXLD_PLUGIN_EXPORT int32_t NXLD_PLUGIN_CALL nxld_plugin_get_interface_param_info(size_t index, int32_t param_index,
                                                              char* param_name, size_t name_size,
                                                              nxld_param_type_t* param_type,
                                                              char* type_name, size_t type_name_size) {
    if (index >= INTERFACE_COUNT || param_index < 0 || param_index >= 2 ||
        param_name == NULL || name_size == 0 || param_type == NULL) {
        return -1;
    }
    
    static const char* param_names[] = { "message", "file_path" };
    static const nxld_param_type_t param_types[] = { 
        NXLD_PARAM_TYPE_STRING,
        NXLD_PARAM_TYPE_STRING
    };
    
    size_t len = strlen(param_names[param_index]);
    if (len >= name_size) {
        len = name_size - 1;
    }
    memcpy(param_name, param_names[param_index], len);
    param_name[len] = '\0';
    
    *param_type = param_types[param_index];
    
    if (type_name != NULL && type_name_size > 0) {
        const char* type_name_str = "string";
        len = strlen(type_name_str);
        if (len >= type_name_size) {
            len = type_name_size - 1;
        }
        memcpy(type_name, type_name_str, len);
        type_name[len] = '\0';
    }
    
    return 0;
}

/**
 * @brief 从参数中提取字符串 / Extract string from parameter / Zeichenfolge aus Parameter extrahieren
 */
static const char* extract_string(const pt_curried_param_t* param) {
    if (param == NULL) {
        return NULL;
    }
    
    if (param->type == NXLD_PARAM_TYPE_STRING || param->type == NXLD_PARAM_TYPE_POINTER) {
        if (param->value.ptr_val != NULL) {
            return (const char*)param->value.ptr_val;
        }
    }
    
    return NULL;
}

/**
 * @brief 展开环境变量路径 / Expand environment variable path / Umgebungsvariablen-Pfad erweitern
 */
static void expand_path(char* output, size_t output_size, const char* input) {
    if (output == NULL || input == NULL || output_size == 0) {
        return;
    }
    
#ifdef _WIN32
    DWORD expanded_size = ExpandEnvironmentStringsA(input, output, (DWORD)output_size);
    if (expanded_size == 0 || expanded_size > output_size) {
        // 如果展开失败，直接复制原路径 / If expansion fails, copy original path / Wenn Erweiterung fehlschlägt, ursprünglichen Pfad kopieren
        strncpy(output, input, output_size - 1);
        output[output_size - 1] = '\0';
    }
#else
    strncpy(output, input, output_size - 1);
    output[output_size - 1] = '\0';
#endif
}

/**
 * @brief 获取当前时间戳字符串 / Get current timestamp string / Aktuellen Zeitstempel-String abrufen
 */
static void get_timestamp(char* buffer, size_t buffer_size) {
    if (buffer == NULL || buffer_size == 0) {
        return;
    }
    
    time_t now = time(NULL);
    struct tm* timeinfo = localtime(&now);
    
    if (timeinfo != NULL) {
        snprintf(buffer, buffer_size, "%04d-%02d-%02d %02d:%02d:%02d",
                 timeinfo->tm_year + 1900,
                 timeinfo->tm_mon + 1,
                 timeinfo->tm_mday,
                 timeinfo->tm_hour,
                 timeinfo->tm_min,
                 timeinfo->tm_sec);
    } else {
        snprintf(buffer, buffer_size, "0000-00-00 00:00:00");
    }
}

/**
 * @brief 写入日志函数 / Write log function / Log-Schreibfunktion
 * @param pack_ptr 参数包指针 / Parameter pack pointer / Parameterpaket-Zeiger
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
NXLD_PLUGIN_EXPORT int32_t NXLD_PLUGIN_CALL WriteLog(void* pack_ptr) {
    pt_param_pack_t* pack = (pt_param_pack_t*)pack_ptr;
    
    if (pack == NULL || pack->param_count < 1 || pack->params == NULL) {
        return -1;
    }
    
    // 提取日志消息 / Extract log message / Log-Nachricht extrahieren
    const char* message = extract_string(&pack->params[0]);
    if (message == NULL) {
        // 尝试直接使用指针值 / Try to use pointer value directly / Versuchen, Zeigerwert direkt zu verwenden
        if (pack->params[0].type == NXLD_PARAM_TYPE_STRING || pack->params[0].type == NXLD_PARAM_TYPE_POINTER) {
            if (pack->params[0].value.ptr_val != NULL) {
                message = (const char*)pack->params[0].value.ptr_val;
            }
        }
        if (message == NULL) {
            return -1;
        }
    }
    
    // 确定日志文件路径 / Determine log file path / Log-Dateipfad bestimmen
    char file_path[MAX_FILE_PATH_SIZE];
    const char* path_template = DEFAULT_LOG_FILE_TEMPLATE;
    if (pack->param_count >= 2) {
        const char* custom_path = extract_string(&pack->params[1]);
        if (custom_path != NULL && strlen(custom_path) > 0) {
            path_template = custom_path;
        }
    }
    
    // 展开环境变量 / Expand environment variables / Umgebungsvariablen erweitern
    expand_path(file_path, sizeof(file_path), path_template);
    
    /* 打开文件进行追加写入 / Open file for append write / Datei zum Anhängen öffnen */
#ifdef _WIN32
    FILE* file = NULL;
    errno_t err = fopen_s(&file, file_path, "a");
    if (err != 0 || file == NULL) {
        return -1;
    }
#else
    FILE* file = fopen(file_path, "a");
    if (file == NULL) {
        return -1;
    }
#endif
    
    /* 获取时间戳 / Get timestamp / Zeitstempel abrufen */
    char timestamp[32];
    get_timestamp(timestamp, sizeof(timestamp));
    
    /* 写入日志条目（UTF-8编码）/ Write log entry (UTF-8 encoding) / Log-Eintrag schreiben (UTF-8-Kodierung) */
    int write_result = fprintf(file, "[%s] %s\n", timestamp, message);
    
    // 刷新缓冲区确保立即写入 / Flush buffer to ensure immediate write / Puffer leeren, um sofortiges Schreiben sicherzustellen
    fflush(file);
    
    // 关闭文件 / Close file / Datei schließen
    fclose(file);
    
    // 检查写入是否成功 / Check if write was successful / Prüfen, ob Schreiben erfolgreich war
    if (write_result < 0) {
        return -1;
    }
    
    return 0;
}

