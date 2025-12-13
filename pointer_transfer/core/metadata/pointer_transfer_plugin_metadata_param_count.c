/**
 * @file pointer_transfer_plugin_metadata_param_count.c
 * @brief 插件参数数量元数据实现 / Plugin Parameter Count Metadata Implementation / Plugin-Parameteranzahl-Metadaten-Implementierung
 */

#include "nxld_plugin_interface.h"
#include <stddef.h>
#include <stdint.h>

/**
 * @brief 获取接口参数数量 / Get interface parameter count / Schnittstellenparameteranzahl abrufen
 */
NXLD_PLUGIN_EXPORT int32_t NXLD_PLUGIN_CALL nxld_plugin_get_interface_param_count(size_t index,
                                                               nxld_param_count_type_t* count_type,
                                                               int32_t* min_count, int32_t* max_count) {
    extern size_t get_interface_count(void);
    extern size_t get_transferpointer_param_count(void);
    
    if (index >= get_interface_count() || count_type == NULL || min_count == NULL || max_count == NULL) {
        return -1;
    }
    
    if (index == 0) {
        *count_type = NXLD_PARAM_COUNT_FIXED;
        *min_count = (int32_t)get_transferpointer_param_count();
        *max_count = (int32_t)get_transferpointer_param_count();
    } else if (index == 1) {
        *count_type = NXLD_PARAM_COUNT_FIXED;
        *min_count = 4;
        *max_count = 4;
    } else {
        return -1;
    }
    
    return 0;
}

