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
 * @param data 序列化数据指针（由pt_serialize_param_pack返回的连续内存块）/ Serialized data pointer (contiguous memory block returned by pt_serialize_param_pack) / Serialisierter Datenzeiger (zusammenhängender Speicherblock, zurückgegeben von pt_serialize_param_pack)
 * @return 成功返回参数包指针，失败返回NULL / Returns parameter pack pointer on success, NULL on failure / Gibt Parameterpaket-Zeiger bei Erfolg zurück, NULL bei Fehler
 * @note 序列化后的数据格式与pt_param_pack_t完全兼容，支持直接类型转换。此函数主要用于设置params指针（当其为NULL时）并验证数据有效性。插件可将void*直接转换为pt_param_pack_t*使用 / Serialized data format is fully compatible with pt_param_pack_t and supports direct type casting. This function is primarily used to set params pointer (when NULL) and validate data validity. Plugins may directly cast void* to pt_param_pack_t* / Serialisiertes Datenformat ist vollständig mit pt_param_pack_t kompatibel und unterstützt direkte Typumwandlung. Diese Funktion wird hauptsächlich verwendet, um params-Zeiger zu setzen (wenn NULL) und Datenvalidität zu überprüfen. Plugins können void* direkt in pt_param_pack_t* umwandeln
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
 * @param plugin_path 插件文件路径 / Plugin file path / Plugin-Dateipfad
 * @param interface_name 接口名称 / Interface name / Schnittstellenname
 * @param expected_param_count 期望的参数数量 / Expected parameter count / Erwartete Parameteranzahl
 * @param return_type 返回值类型 / Return type / Rückgabetyp
 * @return 验证通过返回0，失败返回-1 / Returns 0 if validation passes, -1 on failure / Gibt 0 zurück wenn Validierung erfolgreich, -1 bei Fehler
 * @note 当验证已通过且插件文件时间戳未变更时，跳过重复验证 / When validation has passed and plugin file timestamp is unchanged, skip repeated validation / Wenn Validierung erfolgreich war und Plugin-Datei-Zeitstempel unverändert ist, wiederholte Validierung überspringen
 */
int32_t pt_validate_plugin_function(void* func_ptr, const char* plugin_path, const char* interface_name, int expected_param_count, pt_return_type_t return_type);

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

/* 内部函数声明 / Internal function declarations / Interne Funktionsdeklarationen */

/**
 * @brief 构建.nxpv文件路径 / Build .nxpv file path / .nxpv-Dateipfad erstellen
 */
int pt_build_nxpv_path(const char* plugin_path, char* nxpv_path, size_t buffer_size);

/**
 * @brief 从.nxpv文件读取验证信息 / Read validation info from .nxpv file / Validierungsinformationen aus .nxpv-Datei lesen
 */
int32_t pt_read_validation_from_nxpv(const char* nxpv_path, int64_t* timestamp, int* is_valid);

/**
 * @brief 生成.nxpv验证文件 / Generate .nxpv validation file / .nxpv-Validierungsdatei generieren
 */
int32_t pt_generate_nxpv_file(const char* plugin_path, int64_t timestamp, int is_valid);

/**
 * @brief 检查验证缓存是否有效 / Check if validation cache is valid / Prüfen, ob Validierungs-Cache gültig ist
 */
int32_t pt_check_validation_cache(const char* plugin_path, int* cache_valid);

/**
 * @brief 处理目录中其他DLL文件的验证文件生成 / Handle validation file generation for other DLL files in directory / Validierungsdatei-Generierung für andere DLL-Dateien im Verzeichnis behandeln
 */
int32_t pt_process_directory_dll_validation(const char* plugin_path);

/**
 * @brief 创建测试参数包 / Create test parameter pack / Testparameterpaket erstellen
 */
pt_param_pack_t* pt_create_test_param_pack(int expected_param_count);

/**
 * @brief 调用返回FLOAT类型的函数 / Call function returning FLOAT type / Funktion aufrufen, die FLOAT-Typ zurückgibt
 */
int32_t pt_call_curried_func_float(void* func_ptr, void* serialized_pack, double* result_float, int64_t* result_int);

/**
 * @brief 调用返回DOUBLE类型的函数 / Call function returning DOUBLE type / Funktion aufrufen, die DOUBLE-Typ zurückgibt
 */
int32_t pt_call_curried_func_double(void* func_ptr, void* serialized_pack, double* result_float, int64_t* result_int);

/**
 * @brief 调用返回STRUCT_PTR类型的函数 / Call function returning STRUCT_PTR type / Funktion aufrufen, die STRUCT_PTR-Typ zurückgibt
 */
int32_t pt_call_curried_func_struct_ptr(void* func_ptr, void* serialized_pack, void* result_struct, size_t return_size, int64_t* result_int, double* result_float);

/**
 * @brief 调用返回STRUCT_VAL类型的函数 / Call function returning STRUCT_VAL type / Funktion aufrufen, die STRUCT_VAL-Typ zurückgibt
 */
int32_t pt_call_curried_func_struct_val(void* func_ptr, void* serialized_pack, void* result_struct, int64_t* result_int, double* result_float);

/**
 * @brief 调用返回整数类型的函数 / Call function returning integer type / Funktion aufrufen, die Integer-Typ zurückgibt
 */
int32_t pt_call_curried_func_int(void* func_ptr, void* serialized_pack, int64_t* result_int, double* result_float);

#ifdef __cplusplus
}
#endif

#endif /* POINTER_TRANSFER_CURRYING_H */

