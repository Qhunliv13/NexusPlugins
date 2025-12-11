/**
 * @file pointer_transfer_platform.h
 * @brief 平台抽象层 / Platform Abstraction Layer / Plattform-Abstraktionsebene
 * @details 动态库操作接口 / Dynamic library operation interface / Dynamische Bibliotheksoperationsschnittstelle
 */

#ifndef POINTER_TRANSFER_PLATFORM_H
#define POINTER_TRANSFER_PLATFORM_H

#include "pointer_transfer_types.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 加载动态库 / Load dynamic library / Dynamische Bibliothek laden
 * @param plugin_path 插件文件路径 / Plugin file path / Plugin-Dateipfad
 * @return 动态库句柄，失败返回NULL / Dynamic library handle, NULL on failure / Dynamisches Bibliothekshandle, NULL bei Fehler
 */
void* pt_platform_load_library(const char* plugin_path);

/**
 * @brief 获取动态库符号 / Get dynamic library symbol / Dynamisches Bibliothekssymbol abrufen
 * @param handle 动态库句柄 / Dynamic library handle / Dynamisches Bibliothekshandle
 * @param symbol_name 符号名称 / Symbol name / Symbolname
 * @return 符号地址，失败返回NULL / Symbol address, NULL on failure / Symboladresse, NULL bei Fehler
 */
void* pt_platform_get_symbol(void* handle, const char* symbol_name);

/**
 * @brief 关闭动态库 / Close dynamic library / Dynamische Bibliothek schließen
 * @param handle 动态库句柄 / Dynamic library handle / Dynamisches Bibliothekshandle
 * @return 成功返回0，失败返回非0 / Returns 0 on success, non-zero on failure / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
 */
int32_t pt_platform_close_library(void* handle);

/**
 * @brief 平台函数调用（通过静态验证预防错误）/ Platform function call (prevent errors through static validation) / Plattform-Funktionsaufruf (Fehler durch statische Validierung verhindern)
 * @param func_ptr 函数指针 / Function pointer / Funktionszeiger
 * @param param_count 参数数量 / Parameter count / Parameteranzahl
 * @param param_types 参数类型数组 / Parameter types array / Parametertyp-Array
 * @param param_values 参数值数组 / Parameter values array / Parameterwerte-Array
 * @param param_sizes 参数大小数组，可为NULL / Parameter sizes array, can be NULL / Parametergrößen-Array, kann NULL sein
 * @param return_type 返回值类型 / Return value type / Rückgabewerttyp
 * @param return_size 返回值大小 / Return value size / Rückgabewertgröße
 * @param result_int 输出整数返回值指针 / Output integer return value pointer / Ausgabe-Integer-Rückgabewert-Zeiger
 * @param result_float 输出浮点返回值指针 / Output floating-point return value pointer / Ausgabe-Gleitkomma-Rückgabewert-Zeiger
 * @param result_struct 输出结构体返回值缓冲区，可为NULL / Output struct return value buffer, can be NULL / Ausgabe-Struktur-Rückgabewert-Puffer, kann NULL sein
 * @return 成功返回0，错误返回非0 / Returns 0 on success, non-zero on error / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
 * @details 通过严格的参数验证和静态检查预防错误，不使用异常处理机制 / Prevents errors through strict parameter validation and static checks, no exception handling mechanism / Verhindert Fehler durch strenge Parametervalidierung und statische Prüfungen, kein Ausnahmebehandlungsmechanismus
 */
int32_t pt_platform_safe_call(void* func_ptr, int param_count, void* param_types, void** param_values, void* param_sizes,
                               pt_return_type_t return_type, size_t return_size, int64_t* result_int, double* result_float, void* result_struct);

#ifdef __cplusplus
}
#endif

#endif /* POINTER_TRANSFER_PLATFORM_H */

