/**
 * @file pointer_transfer_plugin_types.h
 * @brief 插件函数参数类型定义 / Plugin Function Parameter Type Definitions / Plugin-Funktionsparameter-Typdefinitionen
 * @details 
 *   插件函数签名：ReturnType PluginFunc(void* pack_ptr) / Plugin function signature: ReturnType PluginFunc(void* pack_ptr) / Plugin-Funktionssignatur: ReturnType PluginFunc(void* pack_ptr)
 *   - pack_ptr: pt_param_pack_t* / pack_ptr: pt_param_pack_t* / pack_ptr: pt_param_pack_t*
 *   - ABI约定：标准ABI / ABI convention: standard ABI / ABI-Konvention: Standard-ABI
 */

#ifndef POINTER_TRANSFER_PLUGIN_TYPES_H
#define POINTER_TRANSFER_PLUGIN_TYPES_H

#include "nxld_plugin_interface.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 参数值联合体 / Parameter Value Union / Parameterwert-Union
 */
typedef union {
    int32_t int32_val;              /**< 32位整数值 / 32-bit integer value / 32-Bit-Ganzzahlwert */
    int64_t int64_val;              /**< 64位整数值 / 64-bit integer value / 64-Bit-Ganzzahlwert */
    float float_val;                /**< 单精度浮点值 / Single-precision floating-point value / Einfachgenauigkeits-Gleitkommawert */
    double double_val;              /**< 双精度浮点值 / Double-precision floating-point value / Doppelgenauigkeits-Gleitkommawert */
    char char_val;                  /**< 字符值 / Character value / Zeichenwert */
    void* ptr_val;                  /**< 指针值 / Pointer value / Zeigerwert */
} pt_param_value_u;

/**
 * @brief 柯里化参数结构体 / Curried Parameter Structure / Currying-Parameter-Struktur
 */
typedef struct {
    nxld_param_type_t type;        /**< 参数类型 / Parameter type / Parametertyp */
    size_t size;                    /**< 参数大小 / Parameter size / Parametergröße */
    pt_param_value_u value;        /**< 参数值联合体 / Parameter value union / Parameterwert-Union */
} pt_curried_param_t;

/**
 * @brief 参数包结构体 / Parameter Pack Structure / Parameterpaket-Struktur
 */
typedef struct {
    int param_count;                /**< 参数数量 / Parameter count / Parameteranzahl */
    pt_curried_param_t* params;     /**< 参数数组 / Parameter array / Parameter-Array */
} pt_param_pack_t;

#ifdef __cplusplus
}
#endif

#endif /* POINTER_TRANSFER_PLUGIN_TYPES_H */

