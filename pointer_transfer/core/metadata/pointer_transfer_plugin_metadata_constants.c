/**
 * @file pointer_transfer_plugin_metadata_constants.c
 * @brief 插件元数据常量定义 / Plugin Metadata Constants / Plugin-Metadaten-Konstanten
 */

#include "pointer_transfer_types.h"
#include <stddef.h>

/* 插件常量定义 / Plugin constant definitions / Plugin-Konstantendefinitionen */
#define PLUGIN_NAME "PointerTransferPlugin"
#define PLUGIN_VERSION "1.2.0"
#define INTERFACE_COUNT 2

/* 接口名称和描述 / Interface names and descriptions / Schnittstellennamen und -beschreibungen */
const char* get_plugin_name(void) {
    return PLUGIN_NAME;
}

const char* get_plugin_version(void) {
    return PLUGIN_VERSION;
}

size_t get_interface_count(void) {
    return INTERFACE_COUNT;
}

const char* get_interface_name(size_t index) {
    static const char* interface_names[] = {
        "TransferPointer",
        "CallPlugin"
    };
    if (index >= INTERFACE_COUNT) {
        return NULL;
    }
    return interface_names[index];
}

const char* get_interface_description(size_t index) {
    static const char* interface_descriptions[] = {
        "传递指针 / Transfer pointer / Zeiger übertragen",
        "调用目标插件接口 / Call target plugin interface / Ziel-Plugin-Schnittstelle aufrufen"
    };
    if (index >= INTERFACE_COUNT) {
        return NULL;
    }
    return interface_descriptions[index];
}

const char* get_interface_version(size_t index) {
    static const char* interface_versions[] = {
        "1.2.0",
        "1.2.0"
    };
    if (index >= INTERFACE_COUNT) {
        return NULL;
    }
    return interface_versions[index];
}

/* TransferPointer接口参数信息 / TransferPointer interface parameter information / TransferPointer-Schnittstellenparameterinformationen */
#define TRANSFERPOINTER_PARAM_COUNT 7

size_t get_transferpointer_param_count(void) {
    return TRANSFERPOINTER_PARAM_COUNT;
}

const char* get_transferpointer_param_name(int32_t param_index) {
    static const char* transferpointer_param_names[] = {
        "source_plugin_name",
        "source_interface_name",
        "source_param_index",
        "ptr",
        "expected_type",
        "type_name",
        "data_size"
    };
    if (param_index < 0 || param_index >= (int32_t)TRANSFERPOINTER_PARAM_COUNT) {
        return NULL;
    }
    return transferpointer_param_names[param_index];
}

nxld_param_type_t get_transferpointer_param_type(int32_t param_index) {
    static const nxld_param_type_t transferpointer_param_types[] = {
        NXLD_PARAM_TYPE_STRING,
        NXLD_PARAM_TYPE_STRING,
        NXLD_PARAM_TYPE_INT32,
        NXLD_PARAM_TYPE_POINTER,
        NXLD_PARAM_TYPE_INT32,
        NXLD_PARAM_TYPE_STRING,
        NXLD_PARAM_TYPE_INT64
    };
    if (param_index < 0 || param_index >= (int32_t)TRANSFERPOINTER_PARAM_COUNT) {
        return 0;
    }
    return transferpointer_param_types[param_index];
}

const char* get_transferpointer_param_type_name(int32_t param_index) {
    static const char* transferpointer_param_type_names[] = {
        "const char*",
        "const char*",
        "int",
        "void*",
        "nxld_param_type_t",
        "const char*",
        "size_t"
    };
    if (param_index < 0 || param_index >= (int32_t)TRANSFERPOINTER_PARAM_COUNT) {
        return NULL;
    }
    return transferpointer_param_type_names[param_index];
}

/* CallPlugin接口参数信息 / CallPlugin interface parameter information / CallPlugin-Schnittstellenparameterinformationen */
const char* get_callplugin_param_name(int32_t param_index) {
    static const char* callplugin_param_names[] = {
        "source_plugin_name",
        "source_interface_name",
        "param_index",
        "param_value"
    };
    if (param_index < 0 || param_index >= 4) {
        return NULL;
    }
    return callplugin_param_names[param_index];
}

nxld_param_type_t get_callplugin_param_type(int32_t param_index) {
    static const nxld_param_type_t callplugin_param_types[] = {
        NXLD_PARAM_TYPE_STRING,
        NXLD_PARAM_TYPE_STRING,
        NXLD_PARAM_TYPE_INT32,
        NXLD_PARAM_TYPE_POINTER
    };
    if (param_index < 0 || param_index >= 4) {
        return 0;
    }
    return callplugin_param_types[param_index];
}

const char* get_callplugin_param_type_name(int32_t param_index) {
    static const char* callplugin_param_type_names[] = {
        "const char*",
        "const char*",
        "int",
        "void*"
    };
    if (param_index < 0 || param_index >= 4) {
        return NULL;
    }
    return callplugin_param_type_names[param_index];
}

