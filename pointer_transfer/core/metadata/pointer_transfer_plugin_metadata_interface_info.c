/**
 * @file pointer_transfer_plugin_metadata_interface_info.c
 * @brief 插件接口信息元数据实现 / Plugin Interface Info Metadata Implementation / Plugin-Schnittstelleninformationen-Metadaten-Implementierung
 */

#include "nxld_plugin_interface.h"
#include <string.h>
#include <stddef.h>

/**
 * @brief 获取接口信息 / Get interface information / Schnittstelleninformationen abrufen
 */
NXLD_PLUGIN_EXPORT int32_t NXLD_PLUGIN_CALL nxld_plugin_get_interface_info(size_t index, 
                                                        char* name, size_t name_size,
                                                        char* description, size_t desc_size,
                                                        char* version, size_t version_size) {
    extern size_t get_interface_count(void);
    extern const char* get_interface_name(size_t index);
    extern const char* get_interface_description(size_t index);
    extern const char* get_interface_version(size_t index);
    
    if (index >= get_interface_count()) {
        return -1;
    }
    
    if (name != NULL && name_size > 0) {
        const char* interface_name = get_interface_name(index);
        if (interface_name != NULL) {
            size_t len = strlen(interface_name);
            if (len >= name_size) {
                len = name_size - 1;
            }
            memcpy(name, interface_name, len);
            name[len] = '\0';
        }
    }
    
    if (description != NULL && desc_size > 0) {
        const char* interface_desc = get_interface_description(index);
        if (interface_desc != NULL) {
            size_t len = strlen(interface_desc);
            if (len >= desc_size) {
                len = desc_size - 1;
            }
            memcpy(description, interface_desc, len);
            description[len] = '\0';
        }
    }
    
    if (version != NULL && version_size > 0) {
        const char* interface_ver = get_interface_version(index);
        if (interface_ver != NULL) {
            size_t len = strlen(interface_ver);
            if (len >= version_size) {
                len = version_size - 1;
            }
            memcpy(version, interface_ver, len);
            version[len] = '\0';
        }
    }
    
    return 0;
}

