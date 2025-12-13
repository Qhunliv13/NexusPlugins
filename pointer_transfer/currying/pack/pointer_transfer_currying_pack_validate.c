/**
 * @file pointer_transfer_currying_pack_validate.c
 * @brief 参数包验证 / Parameter Pack Validation / Parameterpaket-Validierung
 */

#include "pointer_transfer_currying.h"
#include "pointer_transfer_utils.h"
#include <stdint.h>

/**
 * @brief 验证参数包结构符合ABI约定 / Validate parameter pack structure conforms to ABI convention / Parameterpaket-Strukturvalidierung gemäß ABI-Konvention
 * @param pack 参数包指针 / Parameter pack pointer / Parameterpaket-Zeiger
 * @return 有效返回0，无效返回-1 / Returns 0 if valid, -1 if invalid / Gibt 0 zurück wenn gültig, -1 wenn ungültig
 */
int32_t pt_validate_param_pack(pt_param_pack_t* pack) {
    if (pack == NULL) {
        return -1;
    }
    
    if (pack->param_count < 0 || pack->param_count > 256) {
        internal_log_write("ERROR", "Invalid param pack: param_count=%d (must be 0-256)", pack->param_count);
        return -1;
    }
    
    if (pack->param_count > 0) {
        if (pack->params == NULL) {
            internal_log_write("ERROR", "Invalid param pack: params array is NULL but param_count=%d", pack->param_count);
            return -1;
        }
        
        for (int i = 0; i < pack->param_count; i++) {
            pt_curried_param_t* param = &pack->params[i];
            
            if (param->type < NXLD_PARAM_TYPE_VOID || param->type > NXLD_PARAM_TYPE_UNKNOWN) {
                internal_log_write("ERROR", "Invalid param pack: param[%d] has invalid type=%d", i, param->type);
                return -1;
            }
        }
    }
    
    return 0;
}

