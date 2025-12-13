/**
 * @file pointer_transfer_plugin_metadata_param_info.c
 * @brief 插件参数信息元数据实现 / Plugin Parameter Info Metadata Implementation / Plugin-Parameterinformationen-Metadaten-Implementierung
 */

#include "nxld_plugin_interface.h"
#include "pointer_transfer_types.h"
#include <string.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief 获取接口参数信息 / Get interface parameter information / Schnittstellenparameterinformationen abrufen
 */
NXLD_PLUGIN_EXPORT int32_t NXLD_PLUGIN_CALL nxld_plugin_get_interface_param_info(size_t index, int32_t param_index,
                                                              char* param_name, size_t name_size,
                                                              nxld_param_type_t* param_type,
                                                              char* type_name, size_t type_name_size) {
    extern size_t get_interface_count(void);
    extern size_t get_transferpointer_param_count(void);
    extern const char* get_transferpointer_param_name(int32_t param_index);
    extern nxld_param_type_t get_transferpointer_param_type(int32_t param_index);
    extern const char* get_transferpointer_param_type_name(int32_t param_index);
    extern const char* get_callplugin_param_name(int32_t param_index);
    extern nxld_param_type_t get_callplugin_param_type(int32_t param_index);
    extern const char* get_callplugin_param_type_name(int32_t param_index);
    
    if (index >= get_interface_count() || param_name == NULL || name_size == 0 || param_type == NULL) {
        return -1;
    }
    
    if (index == 0) {
        if (param_index < 0 || param_index >= (int32_t)get_transferpointer_param_count()) {
            return -1;
        }
        
        const char* name = get_transferpointer_param_name(param_index);
        if (name != NULL) {
            size_t len = strlen(name);
            if (len >= name_size) {
                len = name_size - 1;
            }
            memcpy(param_name, name, len);
            param_name[len] = '\0';
        }
        
        *param_type = get_transferpointer_param_type(param_index);
        
        if (type_name != NULL && type_name_size > 0) {
            const char* type = get_transferpointer_param_type_name(param_index);
            if (type != NULL) {
                size_t len = strlen(type);
                if (len >= type_name_size) {
                    len = type_name_size - 1;
                }
                memcpy(type_name, type, len);
                type_name[len] = '\0';
            }
        }
    } else if (index == 1) {
        if (param_index < 0 || param_index >= 4) {
            return -1;
        }
        
        const char* name = get_callplugin_param_name(param_index);
        if (name != NULL) {
            size_t len = strlen(name);
            if (len >= name_size) {
                len = name_size - 1;
            }
            memcpy(param_name, name, len);
            param_name[len] = '\0';
        }
        
        *param_type = get_callplugin_param_type(param_index);
        
        if (type_name != NULL && type_name_size > 0) {
            const char* type = get_callplugin_param_type_name(param_index);
            if (type != NULL) {
                size_t len = strlen(type);
                if (len >= type_name_size) {
                    len = type_name_size - 1;
                }
                memcpy(type_name, type, len);
                type_name[len] = '\0';
            }
        }
    } else {
        return -1;
    }
    
    return 0;
}

