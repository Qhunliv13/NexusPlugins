/**
 * @file pointer_transfer_currying_serialize.c
 * @brief 参数包序列化和反序列化 / Parameter Pack Serialization and Deserialization / Parameterpaket-Serialisierung und -Deserialisierung
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
 * @brief 反序列化指针为参数包 / Deserialize pointer to parameter pack / Zeiger in Parameterpaket deserialisieren
 * @param data 序列化数据指针（由pt_serialize_param_pack返回的连续内存块）/ Serialized data pointer (contiguous memory block returned by pt_serialize_param_pack) / Serialisierter Datenzeiger (zusammenhängender Speicherblock, zurückgegeben von pt_serialize_param_pack)
 * @return 成功返回参数包指针，失败返回NULL / Returns parameter pack pointer on success, NULL on failure / Gibt Parameterpaket-Zeiger bei Erfolg zurück, NULL bei Fehler
 * @note 序列化后的数据格式与pt_param_pack_t完全兼容，支持直接类型转换。此函数主要用于设置params指针（当其为NULL时）并验证数据有效性。插件可将void*直接转换为pt_param_pack_t*使用 / Serialized data format is fully compatible with pt_param_pack_t and supports direct type casting. This function is primarily used to set params pointer (when NULL) and validate data validity. Plugins may directly cast void* to pt_param_pack_t* / Serialisiertes Datenformat ist vollständig mit pt_param_pack_t kompatibel und unterstützt direkte Typumwandlung. Diese Funktion wird hauptsächlich verwendet, um params-Zeiger zu setzen (wenn NULL) und Datenvalidität zu überprüfen. Plugins können void* direkt in pt_param_pack_t* umwandeln
 */
pt_param_pack_t* pt_deserialize_param_pack(void* data) {
    if (data == NULL) {
        return NULL;
    }
    
    pt_param_pack_t* pack = (pt_param_pack_t*)data;
    
    if (pack->param_count < 0 || pack->param_count > 256) {
        return NULL;
    }
    
    /* 当params为NULL时，表示数据为序列化格式，需设置正确的指针位置 / If params is NULL, this indicates serialized data format, need to set correct pointer position / Wenn params NULL ist, bedeutet dies serialisiertes Datenformat, korrekte Zeigerposition muss gesetzt werden */
    if (pack->params == NULL) {
        pack->params = (pt_curried_param_t*)((uint8_t*)data + sizeof(pt_param_pack_t));
    }
    
    return pack;
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

