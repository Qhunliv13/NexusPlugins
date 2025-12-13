/**
 * @file pointer_transfer_currying_deserialize.c
 * @brief 参数包反序列化 / Parameter Pack Deserialization / Parameterpaket-Deserialisierung
 */

#include "pointer_transfer_currying.h"
#include <stdint.h>

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

