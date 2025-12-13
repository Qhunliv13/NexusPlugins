/**
 * @file pointer_transfer_plugin_metadata_version.c
 * @brief 插件版本元数据实现 / Plugin Version Metadata Implementation / Plugin-Version-Metadaten-Implementierung
 */

#include "nxld_plugin_interface.h"
#include <string.h>
#include <stddef.h>

/**
 * @brief 获取插件版本 / Get plugin version / Plugin-Version abrufen
 */
NXLD_PLUGIN_EXPORT int32_t NXLD_PLUGIN_CALL nxld_plugin_get_version(char* version, size_t version_size) {
    if (version == NULL || version_size == 0) {
        return -1;
    }
    
    extern const char* get_plugin_version(void);
    const char* plugin_version = get_plugin_version();
    if (plugin_version == NULL) {
        return -1;
    }
    
    size_t len = strlen(plugin_version);
    if (len >= version_size) {
        len = version_size - 1;
    }
    
    memcpy(version, plugin_version, len);
    version[len] = '\0';
    
    return 0;
}

