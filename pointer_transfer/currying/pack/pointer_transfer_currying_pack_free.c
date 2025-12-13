/**
 * @file pointer_transfer_currying_pack_free.c
 * @brief 参数包释放 / Parameter Pack Freeing / Parameterpaket-Freigabe
 */

#include "pointer_transfer_currying.h"
#include <stdlib.h>

/**
 * @brief 释放参数包 / Free parameter pack / Parameterpaket freigeben
 * @param pack 参数包指针 / Parameter pack pointer / Parameterpaket-Zeiger
 */
void pt_free_param_pack(pt_param_pack_t* pack) {
    if (pack == NULL) {
        return;
    }
    
    if (pack->params != NULL) {
        for (int i = 0; i < pack->param_count; i++) {
            /* 只释放通过malloc分配的内存（size > sizeof(void*)且size > 0） / Only free memory allocated via malloc (size > sizeof(void*) and size > 0) / Nur Speicher freigeben, der über malloc zugewiesen wurde (size > sizeof(void*) und size > 0) */
            if (pack->params[i].type != NXLD_PARAM_TYPE_POINTER &&
                pack->params[i].type != NXLD_PARAM_TYPE_STRING &&
                pack->params[i].type != NXLD_PARAM_TYPE_VARIADIC &&
                pack->params[i].type != NXLD_PARAM_TYPE_ANY &&
                pack->params[i].type != NXLD_PARAM_TYPE_UNKNOWN &&
                pack->params[i].value.ptr_val != NULL &&
                pack->params[i].size > sizeof(void*) &&
                pack->params[i].size > 0) { /* 确保size > 0，避免释放未分配的内存 / Ensure size > 0 to avoid freeing unallocated memory / Sicherstellen, dass size > 0, um Freigabe von nicht zugewiesenem Speicher zu vermeiden */
                free(pack->params[i].value.ptr_val);
            }
        }
        free(pack->params);
    }
    
    free(pack);
}

