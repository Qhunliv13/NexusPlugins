/**
 * @file pointer_transfer_currying_serialize.c
 * @brief 参数包序列化 / Parameter Pack Serialization / Parameterpaket-Serialisierung
 */

#include "pointer_transfer_currying.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/**
 * @brief 序列化参数包为单个指针 / Serialize parameter pack to single pointer / Parameterpaket in einzelnen Zeiger serialisieren
 * @param pack 参数包指针 / Parameter pack pointer / Parameterpaket-Zeiger
 * @return 成功返回序列化后的指针，失败返回NULL / Returns serialized pointer on success, NULL on failure / Gibt serialisierten Zeiger bei Erfolg zurück, NULL bei Fehler
 */
void* pt_serialize_param_pack(pt_param_pack_t* pack) {
    if (pack == NULL) {
        return NULL;
    }
    
    if (pack->param_count > 0 && pack->params == NULL) {
        return NULL;
    }
    
    size_t total_size = sizeof(pt_param_pack_t);
    size_t struct_data_size = 0;
    if (pack->params != NULL) {
        for (int i = 0; i < pack->param_count; i++) {
            if (pack->params[i].size > sizeof(void*) && 
                pack->params[i].value.ptr_val != NULL &&
                pack->params[i].type != NXLD_PARAM_TYPE_POINTER &&
                pack->params[i].type != NXLD_PARAM_TYPE_STRING) {
                struct_data_size += pack->params[i].size;
            }
        }
    }
    
    total_size += pack->param_count * sizeof(pt_curried_param_t);
    total_size += struct_data_size;
    
    uint8_t* data = (uint8_t*)malloc(total_size);
    if (data == NULL) {
        return NULL;
    }
    
    pt_param_pack_t* serialized_pack = (pt_param_pack_t*)data;
    serialized_pack->param_count = pack->param_count;
    serialized_pack->params = (pt_curried_param_t*)(data + sizeof(pt_param_pack_t));
    
    uint8_t* current_ptr = data + sizeof(pt_param_pack_t) + pack->param_count * sizeof(pt_curried_param_t);
    if (pack->params != NULL) {
        for (int i = 0; i < pack->param_count; i++) {
            serialized_pack->params[i] = pack->params[i];
            
            if (pack->params[i].size > sizeof(void*) && 
                pack->params[i].value.ptr_val != NULL &&
                pack->params[i].type != NXLD_PARAM_TYPE_POINTER &&
                pack->params[i].type != NXLD_PARAM_TYPE_STRING) {
                memcpy(current_ptr, pack->params[i].value.ptr_val, pack->params[i].size);
                serialized_pack->params[i].value.ptr_val = current_ptr;
                current_ptr += pack->params[i].size;
            }
        }
    }
    
    return data;
}

/**
 * @brief 释放序列化的参数包 / Free serialized parameter pack / Serialisiertes Parameterpaket freigeben
 * @note 序列化的参数包为连续内存块，需直接释放，不应调用pt_free_param_pack / Serialized parameter pack is a contiguous memory block and must be freed directly, not via pt_free_param_pack / Serialisiertes Parameterpaket ist ein zusammenhängender Speicherblock und muss direkt freigegeben werden, nicht über pt_free_param_pack
 */
void pt_free_serialized_param_pack(void* data) {
    if (data == NULL) {
        return;
    }
    free(data);
}
