/**
 * @file pointer_transfer_plugin_metadata_interface_count.c
 * @brief 插件接口数量元数据实现 / Plugin Interface Count Metadata Implementation / Plugin-Schnittstellenanzahl-Metadaten-Implementierung
 */

#include "nxld_plugin_interface.h"
#include <stddef.h>

/**
 * @brief 获取接口数量 / Get interface count / Schnittstellenanzahl abrufen
 */
NXLD_PLUGIN_EXPORT int32_t NXLD_PLUGIN_CALL nxld_plugin_get_interface_count(size_t* count) {
    if (count == NULL) {
        return -1;
    }
    
    extern size_t get_interface_count(void);
    *count = get_interface_count();
    return 0;
}

