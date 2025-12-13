/**
 * @file pointer_transfer_plugin_metadata_name.c
 * @brief 插件名称元数据实现 / Plugin Name Metadata Implementation / Plugin-Namen-Metadaten-Implementierung
 */

#include "nxld_plugin_interface.h"
#include <string.h>
#include <stddef.h>

/**
 * @brief 获取插件名称 / Get plugin name / Plugin-Namen abrufen
 */
NXLD_PLUGIN_EXPORT int32_t NXLD_PLUGIN_CALL nxld_plugin_get_name(char* name, size_t name_size) {
    if (name == NULL || name_size == 0) {
        return -1;
    }
    
    extern const char* get_plugin_name(void);
    const char* plugin_name = get_plugin_name();
    if (plugin_name == NULL) {
        return -1;
    }
    
    size_t len = strlen(plugin_name);
    if (len >= name_size) {
        len = name_size - 1;
    }
    
    memcpy(name, plugin_name, len);
    name[len] = '\0';
    
    return 0;
}

