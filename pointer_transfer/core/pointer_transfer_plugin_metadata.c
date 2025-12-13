/**
 * @file pointer_transfer_plugin_metadata.c
 * @brief 插件元数据和接口信息 / Plugin Metadata and Interface Information / Plugin-Metadaten und Schnittstelleninformationen
 */

#include "nxld_plugin_interface.h"
#include "pointer_transfer_plugin.h"
#include "pointer_transfer_types.h"
#include <string.h>
#include <stddef.h>
#include <stdint.h>

/* 插件常量定义 / Plugin constant definitions / Plugin-Konstantendefinitionen */
#define PLUGIN_NAME "PointerTransferPlugin"
#define PLUGIN_VERSION "1.2.0"
#define INTERFACE_COUNT 2

/* 接口名称和描述 / Interface names and descriptions / Schnittstellennamen und -beschreibungen */
static const char* interface_names[] = {
    "TransferPointer",
    "CallPlugin"
};

static const char* interface_descriptions[] = {
    "传递指针 / Transfer pointer / Zeiger übertragen",
    "调用目标插件接口 / Call target plugin interface / Ziel-Plugin-Schnittstelle aufrufen"
};

static const char* interface_versions[] = {
    "1.2.0",
    "1.2.0"
};

/* TransferPointer接口参数信息 / TransferPointer interface parameter information / TransferPointer-Schnittstellenparameterinformationen */
#define TRANSFERPOINTER_PARAM_COUNT 7
static const char* transferpointer_param_names[] = {
    "source_plugin_name",
    "source_interface_name",
    "source_param_index",
    "ptr",
    "expected_type",
    "type_name",
    "data_size"
};

static const nxld_param_type_t transferpointer_param_types[] = {
    NXLD_PARAM_TYPE_STRING,
    NXLD_PARAM_TYPE_STRING,
    NXLD_PARAM_TYPE_INT32,
    NXLD_PARAM_TYPE_POINTER,
    NXLD_PARAM_TYPE_INT32,
    NXLD_PARAM_TYPE_STRING,
    NXLD_PARAM_TYPE_INT64
};

static const char* transferpointer_param_type_names[] = {
    "const char*",
    "const char*",
    "int",
    "void*",
    "nxld_param_type_t",
    "const char*",
    "size_t"
};

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
        size_t len = strlen(interface_names[index]);
        if (len >= name_size) {
            len = name_size - 1;
        }
        memcpy(name, interface_names[index], len);
        name[len] = '\0';
    }
    
    if (description != NULL && desc_size > 0) {
        size_t len = strlen(interface_descriptions[index]);
        if (len >= desc_size) {
            len = desc_size - 1;
        }
        memcpy(description, interface_descriptions[index], len);
        description[len] = '\0';
    }
    
    if (version != NULL && version_size > 0) {
        size_t len = strlen(interface_versions[index]);
        if (len >= version_size) {
            len = version_size - 1;
        }
        memcpy(version, interface_versions[index], len);
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
    
    if (index == 0) {
        *count_type = NXLD_PARAM_COUNT_FIXED;
        *min_count = (int32_t)TRANSFERPOINTER_PARAM_COUNT;
        *max_count = (int32_t)TRANSFERPOINTER_PARAM_COUNT;
    } else if (index == 1) {
        *count_type = NXLD_PARAM_COUNT_FIXED;
        *min_count = 4;
        *max_count = 4;
    } else {
        return -1;
    }
    
    return 0;
}

/**
 * @brief 获取接口参数信息 / Get interface parameter information / Schnittstellenparameterinformationen abrufen
 */
NXLD_PLUGIN_EXPORT int32_t NXLD_PLUGIN_CALL nxld_plugin_get_interface_param_info(size_t index, int32_t param_index,
                                                              char* param_name, size_t name_size,
                                                              nxld_param_type_t* param_type,
                                                              char* type_name, size_t type_name_size) {
    if (index >= INTERFACE_COUNT || param_name == NULL || name_size == 0 || param_type == NULL) {
        return -1;
    }
    
    if (index == 0) {
        if (param_index < 0 || param_index >= (int32_t)TRANSFERPOINTER_PARAM_COUNT) {
            return -1;
        }
        
        size_t len = strlen(transferpointer_param_names[param_index]);
        if (len >= name_size) {
            len = name_size - 1;
        }
        memcpy(param_name, transferpointer_param_names[param_index], len);
        param_name[len] = '\0';
        
        *param_type = transferpointer_param_types[param_index];
        
        if (type_name != NULL && type_name_size > 0) {
            len = strlen(transferpointer_param_type_names[param_index]);
            if (len >= type_name_size) {
                len = type_name_size - 1;
            }
            memcpy(type_name, transferpointer_param_type_names[param_index], len);
            type_name[len] = '\0';
        }
    } else if (index == 1) {
        static const char* callplugin_param_names[] = {
            "source_plugin_name",
            "source_interface_name",
            "param_index",
            "param_value"
        };
        static const nxld_param_type_t callplugin_param_types[] = {
            NXLD_PARAM_TYPE_STRING,
            NXLD_PARAM_TYPE_STRING,
            NXLD_PARAM_TYPE_INT32,
            NXLD_PARAM_TYPE_POINTER
        };
        static const char* callplugin_param_type_names[] = {
            "const char*",
            "const char*",
            "int",
            "void*"
        };
        
        if (param_index < 0 || param_index >= 4) {
            return -1;
        }
        
        size_t len = strlen(callplugin_param_names[param_index]);
        if (len >= name_size) {
            len = name_size - 1;
        }
        memcpy(param_name, callplugin_param_names[param_index], len);
        param_name[len] = '\0';
        
        *param_type = callplugin_param_types[param_index];
        
        if (type_name != NULL && type_name_size > 0) {
            len = strlen(callplugin_param_type_names[param_index]);
            if (len >= type_name_size) {
                len = type_name_size - 1;
            }
            memcpy(type_name, callplugin_param_type_names[param_index], len);
            type_name[len] = '\0';
        }
    } else {
        return -1;
    }
    
    return 0;
}

