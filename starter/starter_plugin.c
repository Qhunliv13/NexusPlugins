/**
 * @file starter_plugin.c
 * @brief 启动插件实现 / Starter Plugin Implementation / Starter-Plugin-Implementierung
 * @details 入口插件，提供启动接口用于触发调用链 / Entry plugin providing start interface to trigger call chain / Einstiegs-Plugin mit Start-Schnittstelle zum Auslösen der Aufrufkette
 */

#include "nxld_plugin_interface.h"
#include "pointer_transfer_plugin_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef _WIN32
#define NXLD_PLUGIN_EXPORT __declspec(dllexport)
#define NXLD_PLUGIN_CALL __cdecl
#else
#define NXLD_PLUGIN_EXPORT __attribute__((visibility("default")))
#define NXLD_PLUGIN_CALL
#endif

/* 插件名称 / Plugin name / Plugin-Name */
#define PLUGIN_NAME "StarterPlugin"
#define PLUGIN_VERSION "1.0.0"
#define INTERFACE_COUNT 1

/* 接口名称 / Interface name / Schnittstellenname */
static const char* interface_name = "Start";

/* 接口描述 / Interface description / Schnittstellenbeschreibung */
static const char* interface_description = "Start execution chain (no parameters), returns int32 (0=success, -1=failure)";

/* 接口版本 / Interface version / Schnittstellenversion */
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
    
    /* Start 接口无参数 / Start interface has no parameters / Start-Schnittstelle hat keine Parameter */
    *count_type = NXLD_PARAM_COUNT_FIXED;
    *min_count = 0;
    *max_count = 0;
    
    return 0;
}

/**
 * @brief 获取接口参数信息 / Get interface parameter information / Schnittstellenparameterinformationen abrufen
 */
NXLD_PLUGIN_EXPORT int32_t NXLD_PLUGIN_CALL nxld_plugin_get_interface_param_info(size_t index, int32_t param_index,
                                                              char* param_name, size_t name_size,
                                                              nxld_param_type_t* param_type,
                                                              char* type_name, size_t type_name_size) {
    /* Start 接口无参数 / Start interface has no parameters / Start-Schnittstelle hat keine Parameter */
    if (index >= INTERFACE_COUNT || param_index < 0) {
        return -1;
    }
    
    return -1;
}

/**
 * @brief 启动执行链 / Start execution chain / Ausführungskette starten
 * @param pack_ptr 参数包指针 / Parameter pack pointer / Parameterpaket-Zeiger
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 * 
 * @details
 * 该接口用于触发调用链的启动。系统会根据配置文件中的 StartInterface 信息，
 * 通过调度器（PointerTransferPlugin）的 CallPlugin 接口来启动目标插件。
 * 
 * This interface is used to trigger the start of the call chain. The system will
 * use the StartInterface information in the configuration file to start the
 * target plugin through the scheduler's (PointerTransferPlugin) CallPlugin interface.
 * 
 * Diese Schnittstelle wird verwendet, um den Start der Aufrufkette auszulösen. Das System
 * verwendet die StartInterface-Informationen in der Konfigurationsdatei, um das Ziel-Plugin
 * über die CallPlugin-Schnittstelle des Schedulers (PointerTransferPlugin) zu starten.
 */
NXLD_PLUGIN_EXPORT int32_t NXLD_PLUGIN_CALL Start(void* pack_ptr) {
    /* Start 接口无参数，直接返回成功 / Start interface has no parameters, return success directly / Start-Schnittstelle hat keine Parameter, Erfolg direkt zurückgeben */
    return 0;
}

