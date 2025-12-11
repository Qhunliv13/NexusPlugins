/**
 * @file pointer_transfer_currying.h
 * @brief 柯里化参数传递机制 / Currying Parameter Transfer Mechanism / Currying-Parameterübertragungsmechanismus
 */

#ifndef POINTER_TRANSFER_CURRYING_H
#define POINTER_TRANSFER_CURRYING_H

#include "nxld_plugin_interface.h"
#include "pointer_transfer_types.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 柯里化参数结构体 / Curried Parameter Structure / Currying-Parameter-Struktur
 */
typedef struct {
    nxld_param_type_t type;        /**< 参数类型 / Parameter type / Parametertyp */
    size_t size;                    /**< 参数大小 / Parameter size / Parametergröße */
    union {
        int32_t int32_val;          /**< 32位整数值 / 32-bit integer value / 32-Bit-Ganzzahlwert */
        int64_t int64_val;          /**< 64位整数值 / 64-bit integer value / 64-Bit-Ganzzahlwert */
        float float_val;            /**< 单精度浮点值 / Single-precision floating-point value / Einfachgenauigkeits-Gleitkommawert */
        double double_val;          /**< 双精度浮点值 / Double-precision floating-point value / Doppelgenauigkeits-Gleitkommawert */
        char char_val;              /**< 字符值 / Character value / Zeichenwert */
        void* ptr_val;              /**< 指针值 / Pointer value / Zeigerwert */
    } value;
} pt_curried_param_t;

/**
 * @brief 参数包结构体 / Parameter Pack Structure / Parameterpaket-Struktur
 */
typedef struct {
    int param_count;                /**< 参数数量 / Parameter count / Parameteranzahl */
    pt_curried_param_t* params;     /**< 参数数组 / Parameter array / Parameter-Array */
} pt_param_pack_t;

/**
 * @brief 创建参数包 / Create parameter pack / Parameterpaket erstellen
 * @param param_count 参数数量 / Parameter count / Parameteranzahl
 * @param param_types 参数类型数组 / Parameter types array / Parametertyp-Array
 * @param param_values 参数值数组 / Parameter values array / Parameterwerte-Array
 * @param param_sizes 参数大小数组 / Parameter sizes array / Parametergrößen-Array
 * @return 成功返回参数包指针，失败返回NULL / Returns parameter pack pointer on success, NULL on failure / Gibt Parameterpaket-Zeiger bei Erfolg zurück, NULL bei Fehler
 */
pt_param_pack_t* pt_create_param_pack(int param_count, nxld_param_type_t* param_types, void** param_values, size_t* param_sizes);

/**
 * @brief 释放参数包 / Free parameter pack / Parameterpaket freigeben
 * @param pack 参数包指针 / Parameter pack pointer / Parameterpaket-Zeiger
 */
void pt_free_param_pack(pt_param_pack_t* pack);

/**
 * @brief 序列化参数包为单个指针 / Serialize parameter pack to single pointer / Parameterpaket in einzelnen Zeiger serialisieren
 * @param pack 参数包指针 / Parameter pack pointer / Parameterpaket-Zeiger
 * @return 成功返回序列化后的指针，失败返回NULL / Returns serialized pointer on success, NULL on failure / Gibt serialisierten Zeiger bei Erfolg zurück, NULL bei Fehler
 */
void* pt_serialize_param_pack(pt_param_pack_t* pack);

/**
 * @brief 反序列化指针为参数包 / Deserialize pointer to parameter pack / Zeiger in Parameterpaket deserialisieren
 * @param data 序列化数据指针 / Serialized data pointer / Serialisierter Datenzeiger
 * @return 成功返回参数包指针，失败返回NULL / Returns parameter pack pointer on success, NULL on failure / Gibt Parameterpaket-Zeiger bei Erfolg zurück, NULL bei Fehler
 */
pt_param_pack_t* pt_deserialize_param_pack(void* data);

/**
 * @brief 释放序列化的参数包 / Free serialized parameter pack / Serialisiertes Parameterpaket freigeben
 * @param data 序列化数据指针（由pt_serialize_param_pack返回）/ Serialized data pointer (returned by pt_serialize_param_pack) / Serialisierter Datenzeiger (von pt_serialize_param_pack zurückgegeben)
 */
void pt_free_serialized_param_pack(void* data);

/**
 * @brief 验证参数包结构符合ABI约定 / Validate parameter pack structure conforms to ABI convention / Parameterpaket-Strukturvalidierung gemäß ABI-Konvention
 * @param pack 参数包指针 / Parameter pack pointer / Parameterpaket-Zeiger
 * @return 有效返回0，无效返回-1 / Returns 0 if valid, -1 if invalid / Gibt 0 zurück wenn gültig, -1 wenn ungültig
 */
int32_t pt_validate_param_pack(pt_param_pack_t* pack);

/**
 * @brief 验证插件函数兼容性 / Validate plugin function compatibility / Plugin-Funktionskompatibilität validieren
 * @param func_ptr 插件函数指针 / Plugin function pointer / Plugin-Funktionszeiger
 * @param expected_param_count 期望的参数数量 / Expected parameter count / Erwartete Parameteranzahl
 * @param return_type 返回值类型 / Return type / Rückgabetyp
 * @return 验证通过返回0，失败返回-1 / Returns 0 if validation passes, -1 on failure / Gibt 0 zurück wenn Validierung erfolgreich, -1 bei Fehler
 */
int32_t pt_validate_plugin_function(void* func_ptr, int expected_param_count, pt_return_type_t return_type);

/**
 * @brief 柯里化调用函数 / Call function using currying / Funktion mit Currying aufrufen
 * @param func_ptr 函数指针 / Function pointer / Funktionszeiger
 * @param pack 参数包指针 / Parameter pack pointer / Parameterpaket-Zeiger
 * @param return_type 返回值类型 / Return type / Rückgabetyp
 * @param return_size 返回值大小 / Return size / Rückgabegröße
 * @param result_int 输出整数返回值指针 / Output integer return value pointer / Ausgabe-Integer-Rückgabewert-Zeiger
 * @param result_float 输出浮点返回值指针 / Output floating-point return value pointer / Ausgabe-Gleitkomma-Rückgabewert-Zeiger
 * @param result_struct 输出结构体返回值缓冲区 / Output struct return value buffer / Ausgabe-Struktur-Rückgabewert-Puffer
 * @return 成功返回0，失败返回非0值 / Returns 0 on success, non-zero on failure / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
 */
int32_t pt_call_with_currying(void* func_ptr, pt_param_pack_t* pack, 
                               pt_return_type_t return_type, size_t return_size,
                               int64_t* result_int, double* result_float, void* result_struct);

#ifdef __cplusplus
}
#endif

#endif /* POINTER_TRANSFER_CURRYING_H */

