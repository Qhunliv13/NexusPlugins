/**
 * @file pointer_transfer_utils.h
 * @brief 指针传递插件工具函数 / Pointer Transfer Plugin Utility Functions / Zeigerübertragungs-Plugin-Hilfsfunktionen
 */

#ifndef POINTER_TRANSFER_UTILS_H
#define POINTER_TRANSFER_UTILS_H

#include "nxld_plugin_interface.h"
#include "pointer_transfer_types.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 内部日志输出函数 / Internal log output function / Interne Protokollausgabefunktion
 * @param level 日志级别字符串 / Log level string / Protokollierungsebenen-Zeichenfolge
 * @param format 格式化字符串 / Format string / Formatzeichenfolge
 * @param ... 可变参数 / Variable arguments / Variable Argumente
 */
void internal_log_write(const char* level, const char* format, ...);

/**
 * @brief 动态分配字符串 / Dynamically allocate string / Zeichenfolge dynamisch zuweisen
 * @param str 源字符串 / Source string / Quellzeichenfolge
 * @return 成功返回分配的字符串指针，失败返回NULL / Returns allocated string pointer on success, NULL on failure / Gibt zugewiesenen Zeichenfolgenzeiger bei Erfolg zurück, NULL bei Fehler
 */
char* allocate_string(const char* str);

/**
 * @brief 去除字符串前后空白字符 / Trim whitespace from string / Leerzeichen von Zeichenfolge entfernen
 * @param str 输入字符串 / Input string / Eingabezeichenfolge
 * @return 处理后的字符串指针 / Pointer to processed string / Zeiger auf verarbeitete Zeichenfolge
 */
char* trim_string(char* str);

/**
 * @brief 解析键值对 / Parse key-value pair / Schlüssel-Wert-Paar parsen
 * @param line 输入行 / Input line / Eingabezeile
 * @param key 输出键缓冲区 / Output key buffer / Ausgabe-Schlüssel-Puffer
 * @param key_size 键缓冲区大小 / Key buffer size / Schlüssel-Puffergröße
 * @param value 输出值缓冲区 / Output value buffer / Ausgabe-Wert-Puffer
 * @param value_size 值缓冲区大小 / Value buffer size / Wert-Puffergröße
 * @return 成功返回1，失败返回0 / Returns 1 on success, 0 on failure / Gibt 1 bei Erfolg zurück, 0 bei Fehler
 */
int parse_key_value_simple(const char* line, char* key, size_t key_size, char* value, size_t value_size);

/**
 * @brief 获取参数类型名称字符串 / Get parameter type name string / Parametertypnamen-Zeichenfolge abrufen
 * @param type 参数类型 / Parameter type / Parametertyp
 * @return 类型名称字符串 / Type name string / Typnamen-Zeichenfolge
 */
const char* get_type_name_string(nxld_param_type_t type);

/**
 * @brief 检查类型兼容性 / Check type compatibility / Typkompatibilität prüfen
 * @param actual_type 实际类型 / Actual type / Tatsächlicher Typ
 * @param expected_type 期望类型 / Expected type / Erwarteter Typ
 * @return 兼容返回1，不兼容返回0 / Returns 1 if compatible, 0 if not / Gibt 1 zurück wenn kompatibel, 0 wenn nicht
 */
int check_type_compatibility(nxld_param_type_t actual_type, nxld_param_type_t expected_type);

/**
 * @brief 检查传递条件 / Check transfer condition / Übertragungsbedingung prüfen
 * @param condition 条件字符串 / Condition string / Bedingungszeichenfolge
 * @param param_value 参数值指针 / Parameter value pointer / Parameterwert-Zeiger
 * @return 满足条件返回1，否则返回0 / Returns 1 if condition is met, 0 otherwise / Gibt 1 zurück, wenn Bedingung erfüllt ist, sonst 0
 */
int check_condition(const char* condition, void* param_value);

/**
 * @brief 获取当前DLL路径 / Get current DLL path / Aktuellen DLL-Pfad abrufen
 * @param path_buffer 输出路径缓冲区 / Output path buffer / Ausgabe-Pfad-Puffer
 * @param buffer_size 缓冲区大小 / Buffer size / Puffergröße
 * @return 成功返回0，失败返回非0 / Returns 0 on success, non-zero on failure / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
 */
int get_current_dll_path(char* path_buffer, size_t buffer_size);

/**
 * @brief 构建.nxpt文件路径 / Build .nxpt file path / .nxpt-Dateipfad erstellen
 * @param file_path 源文件路径 / Source file path / Quelldateipfad
 * @param nxpt_path 输出.nxpt路径缓冲区 / Output .nxpt path buffer / Ausgabe-.nxpt-Pfad-Puffer
 * @param buffer_size 缓冲区大小 / Buffer size / Puffergröße
 * @return 成功返回0，失败返回非0 / Returns 0 on success, non-zero on failure / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
 */
int build_nxpt_path(const char* file_path, char* nxpt_path, size_t buffer_size);

/* 前向声明 / Forward declaration / Vorwärtsdeklaration */
struct target_interface_state_s;

/**
 * @brief 从常量字符串设置参数值 / Set parameter value from constant string / Parameterwert aus Konstantenzeichenfolge setzen
 * @param state 接口状态指针 / Interface state pointer / Schnittstellenstatus-Zeiger
 * @param param_index 参数索引 / Parameter index / Parameterindex
 * @param const_value 常量值字符串 / Constant value string / Konstantenwert-Zeichenfolge
 * @param plugin_name 插件名称 / Plugin name / Plugin-Name
 * @param interface_name 接口名称 / Interface name / Schnittstellenname
 * @return 成功返回1，失败返回0 / Returns 1 on success, 0 on failure / Gibt 1 bei Erfolg zurück, 0 bei Fehler
 */
int set_parameter_value_from_const_string(struct target_interface_state_s* state, int param_index, const char* const_value, const char* plugin_name, const char* interface_name);

/**
 * @brief 从指针设置参数值 / Set parameter value from pointer / Parameterwert aus Zeiger setzen
 * @param state 接口状态指针 / Interface state pointer / Schnittstellenstatus-Zeiger
 * @param param_index 参数索引 / Parameter index / Parameterindex
 * @param ptr 参数值指针 / Parameter value pointer / Parameterwert-Zeiger
 * @param stored_size 存储的数据大小 / Stored data size / Gespeicherte Datengröße
 * @param plugin_name 插件名称 / Plugin name / Plugin-Name
 * @param interface_name 接口名称 / Interface name / Schnittstellenname
 * @return 成功返回1，失败返回0 / Returns 1 on success, 0 on failure / Gibt 1 bei Erfolg zurück, 0 bei Fehler
 */
int set_parameter_value_from_pointer(struct target_interface_state_s* state, int param_index, void* ptr, size_t stored_size, const char* plugin_name, const char* interface_name);

/**
 * @brief 从接口描述推断返回值类型 / Infer return type from interface description / Rückgabetyp aus Schnittstellenbeschreibung ableiten
 * @param description 接口描述字符串 / Interface description string / Schnittstellenbeschreibungs-Zeichenfolge
 * @return 推断的返回值类型 / Inferred return type / Abgeleiteter Rückgabetyp
 */
pt_return_type_t infer_return_type_from_description(const char* description);

/**
 * @brief 释放单个传递规则的内存 / Free memory of single transfer rule / Speicher einer einzelnen Übertragungsregel freigeben
 * @param rule 规则指针 / Rule pointer / Regel-Zeiger
 */
void free_single_rule(pointer_transfer_rule_t* rule);

#ifdef __cplusplus
}
#endif

#endif /* POINTER_TRANSFER_UTILS_H */

