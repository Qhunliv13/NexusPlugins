/**
 * @file pointer_transfer_currying_pack.c
 * @brief 参数包创建和释放 / Parameter Pack Creation and Freeing / Parameterpaket-Erstellung und -Freigabe
 */

#include "pointer_transfer_currying.h"
#include "pointer_transfer_utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/**
 * @brief 创建参数包 / Create parameter pack / Parameterpaket erstellen
 * @param param_count 参数数量 / Parameter count / Parameteranzahl
 * @param param_types 参数类型数组 / Parameter types array / Parametertyp-Array
 * @param param_values 参数值数组 / Parameter values array / Parameterwerte-Array
 * @param param_sizes 参数大小数组 / Parameter sizes array / Parametergrößen-Array
 * @return 成功返回参数包指针，失败返回NULL / Returns parameter pack pointer on success, NULL on failure / Gibt Parameterpaket-Zeiger bei Erfolg zurück, NULL bei Fehler
 */
pt_param_pack_t* pt_create_param_pack(int param_count, nxld_param_type_t* param_types, void** param_values, size_t* param_sizes) {
    if (param_count < 0 || param_types == NULL || param_values == NULL) {
        return NULL;
    }
    
    pt_param_pack_t* pack = (pt_param_pack_t*)malloc(sizeof(pt_param_pack_t));
    if (pack == NULL) {
        return NULL;
    }
    
    pack->param_count = param_count;
    pack->params = NULL;
    
    if (param_count > 0) {
        pack->params = (pt_curried_param_t*)calloc(param_count, sizeof(pt_curried_param_t));
        if (pack->params == NULL) {
            free(pack);
            return NULL;
        }
        
        for (int i = 0; i < param_count; i++) {
            pack->params[i].type = param_types[i];
            pack->params[i].size = param_sizes != NULL ? param_sizes[i] : 0;
            
            void* value_ptr = param_values[i];
            if (value_ptr == NULL) {
                memset(&pack->params[i].value, 0, sizeof(pack->params[i].value));
                continue;
            }
            
            switch (param_types[i]) {
                case NXLD_PARAM_TYPE_INT32:
                    pack->params[i].value.int32_val = *(int32_t*)value_ptr;
                    break;
                case NXLD_PARAM_TYPE_INT64:
                    pack->params[i].value.int64_val = *(int64_t*)value_ptr;
                    break;
                case NXLD_PARAM_TYPE_FLOAT:
                    pack->params[i].value.float_val = *(float*)value_ptr;
                    break;
                case NXLD_PARAM_TYPE_DOUBLE:
                    pack->params[i].value.double_val = *(double*)value_ptr;
                    break;
                case NXLD_PARAM_TYPE_CHAR:
                    pack->params[i].value.char_val = *(char*)value_ptr;
                    break;
                case NXLD_PARAM_TYPE_POINTER:
                case NXLD_PARAM_TYPE_STRING:
                case NXLD_PARAM_TYPE_VARIADIC:
                case NXLD_PARAM_TYPE_ANY:
                case NXLD_PARAM_TYPE_UNKNOWN:
                    pack->params[i].value.ptr_val = value_ptr;
                    break;
                default:
                    if (param_sizes != NULL && param_sizes[i] > 0) {
                        void* struct_data = malloc(param_sizes[i]);
                        if (struct_data != NULL) {
                            memcpy(struct_data, value_ptr, param_sizes[i]);
                            pack->params[i].value.ptr_val = struct_data;
                        } else {
                            /* 内存分配失败，回退到直接使用指针 / Memory allocation failed, fallback to direct pointer / Speicherzuweisung fehlgeschlagen, Fallback auf direkten Zeiger */
                            internal_log_write("WARNING", "Failed to allocate memory for struct parameter %d (size=%zu), using pointer directly", i, param_sizes[i]);
                            pack->params[i].value.ptr_val = value_ptr;
                            pack->params[i].size = 0; /* 标记为未分配 / Mark as not allocated / Als nicht zugewiesen markieren */
                        }
                    } else {
                        pack->params[i].value.ptr_val = value_ptr;
                    }
                    break;
            }
        }
    }
    
    return pack;
}

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

