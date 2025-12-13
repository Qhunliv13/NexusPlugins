/**
 * @file pointer_transfer_interface.h
 * @brief 指针传递插件接口调用 / Pointer Transfer Plugin Interface Invocation / Zeigerübertragungs-Plugin-Schnittstellenaufruf
 */

#ifndef POINTER_TRANSFER_INTERFACE_H
#define POINTER_TRANSFER_INTERFACE_H

#include "pointer_transfer_types.h"
#include "nxld_plugin_interface.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* NXLD标准调用约定 / NXLD standard calling convention / NXLD-Standard-Aufrufkonvention */

/* call_function_generic位于pointer_transfer_platform.c，替代函数为pt_platform_safe_call / call_function_generic is located in pointer_transfer_platform.c, replacement function is pt_platform_safe_call / call_function_generic befindet sich in pointer_transfer_platform.c, Ersatzfunktion ist pt_platform_safe_call */

/**
 * @brief 查找目标接口状态（不创建）/ Find target interface state (without creating) / Ziel-Schnittstellenstatus suchen (ohne Erstellung)
 * @param plugin_name 插件名称 / Plugin name / Plugin-Name
 * @param interface_name 接口名称 / Interface name / Schnittstellenname
 * @return 成功返回接口状态指针，不存在返回NULL / Returns interface state pointer on success, NULL if not found / Gibt Schnittstellenstatus-Zeiger bei Erfolg zurück, NULL wenn nicht gefunden
 */
target_interface_state_t* find_interface_state(const char* plugin_name, const char* interface_name);

/**
 * @brief 查找或创建目标接口状态 / Find or create target interface state / Ziel-Schnittstellenstatus suchen oder erstellen
 * @param plugin_name 插件名称 / Plugin name / Plugin-Name
 * @param interface_name 接口名称 / Interface name / Schnittstellenname
 * @param handle 插件句柄 / Plugin handle / Plugin-Handle
 * @param func_ptr 函数指针 / Function pointer / Funktionszeiger
 * @return 成功返回接口状态指针，失败返回NULL / Returns interface state pointer on success, NULL on failure / Gibt Schnittstellenstatus-Zeiger bei Erfolg zurück, NULL bei Fehler
 */
target_interface_state_t* find_or_create_interface_state(const char* plugin_name, const char* interface_name, void* handle, void* func_ptr);

/* 接口信息获取相关函数 / Interface information retrieval functions / Schnittstelleninformationen-Abruf-Funktionen */
int get_plugin_interface_functions(void* handle, void** get_interface_count_out, void** get_interface_info_out,
                                    void** get_param_count_out, void** get_param_info_out);
int find_interface_index(void* get_interface_count, void* get_interface_info, const char* interface_name,
                         size_t* interface_index_out, pt_return_type_t* inferred_return_type_out, char** saved_desc_buf_out);
int get_parameter_count_info(void* get_param_count, size_t interface_index,
                              nxld_param_count_type_t* param_count_type_out, int32_t* min_count_out, int32_t* max_count_out);

/* 接口状态创建相关函数 / Interface state creation functions / Schnittstellenstatus-Erstellungsfunktionen */
int calculate_param_count(nxld_param_count_type_t param_count_type, int32_t min_count, int32_t max_count, int* is_variadic_out);
int allocate_parameter_arrays(target_interface_state_t* state, int param_count);
void free_parameter_arrays(target_interface_state_t* state);
int initialize_parameter_types(target_interface_state_t* state, void* get_param_info, size_t interface_index, int param_count);
int initialize_interface_state_basic(target_interface_state_t* state, const char* plugin_name, const char* interface_name,
                                      void* handle, void* func_ptr, int param_count, int is_variadic, int min_param_count, pt_return_type_t return_type);

/**
 * @brief 调用目标插件接口 / Call target plugin interface / Ziel-Plugin-Schnittstelle aufrufen
 * @param rule 传递规则 / Transfer rule / Übertragungsregel
 * @param ptr 要传递的指针 / Pointer to transfer / Zu übertragender Zeiger
 * @return 成功返回0，失败返回非0 / Returns 0 on success, non-zero on failure / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
 */
int call_target_plugin_interface(const pointer_transfer_rule_t* rule, void* ptr);

/**
 * @brief 验证和设置参数值 / Validate and set parameter value / Parameterwert validieren und setzen
 * @param rule 传递规则 / Transfer rule / Übertragungsregel
 * @param state 接口状态 / Interface state / Schnittstellenstatus
 * @param ptr 指针值 / Pointer value / Zeigerwert
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int validate_and_set_parameter(const pointer_transfer_rule_t* rule, target_interface_state_t* state, void* ptr);

/**
 * @brief 验证参数就绪状态 / Validate parameter readiness / Parameterbereitschaft validieren
 * @param rule 传递规则 / Transfer rule / Übertragungsregel
 * @param state 接口状态 / Interface state / Schnittstellenstatus
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int validate_parameter_readiness(const pointer_transfer_rule_t* rule, target_interface_state_t* state);

/**
 * @brief 计算实际参数数量 / Calculate actual parameter count / Tatsächliche Parameteranzahl berechnen
 * @param rule 传递规则 / Transfer rule / Übertragungsregel
 * @param state 接口状态 / Interface state / Schnittstellenstatus
 * @return 实际参数数量 / Actual parameter count / Tatsächliche Parameteranzahl
 */
int calculate_actual_param_count(const pointer_transfer_rule_t* rule, target_interface_state_t* state);

/* 参数验证相关函数 / Parameter validation functions / Parameter-Validierungsfunktionen */
int validate_parameter_index(const pointer_transfer_rule_t* rule, target_interface_state_t* state);
int validate_parameter_pointer(const pointer_transfer_rule_t* rule, target_interface_state_t* state, void* ptr);
int validate_parameter_arrays(const pointer_transfer_rule_t* rule, target_interface_state_t* state);

/* 参数设置相关函数 / Parameter setting functions / Parameter-Einstellungsfunktionen */
int set_parameter_from_const_string(const pointer_transfer_rule_t* rule, target_interface_state_t* state, void* ptr);
int set_parameter_from_pointer(const pointer_transfer_rule_t* rule, target_interface_state_t* state, void* ptr);
int apply_constant_value_rules(const pointer_transfer_rule_t* rule, target_interface_state_t* state);

/* 参数就绪状态验证相关函数 / Parameter readiness validation functions / Parameterbereitschaft-Validierungsfunktionen */
int calculate_variadic_ready_count(target_interface_state_t* state);
void build_unready_params_string(target_interface_state_t* state, int start_index, int end_index,
                                  char* unready_params_out, size_t max_size);
int validate_variadic_parameter_readiness(const pointer_transfer_rule_t* rule, target_interface_state_t* state);
int validate_fixed_parameter_readiness(const pointer_transfer_rule_t* rule, target_interface_state_t* state);

/* 参数数量计算相关函数 / Parameter count calculation functions / Parameteranzahl-Berechnungsfunktionen */
int calculate_variadic_base_ready_count(target_interface_state_t* state);
int check_intermediate_parameters_ready(target_interface_state_t* state, int start_index, int end_index);
int update_variadic_count_from_extra_rules(const pointer_transfer_rule_t* rule, target_interface_state_t* state, int base_count);
void validate_and_log_variadic_count(const pointer_transfer_rule_t* rule, target_interface_state_t* state, int actual_param_count);

/**
 * @brief 检测调用循环 / Detect call cycle / Aufrufzyklus erkennen
 * @param rule 传递规则 / Transfer rule / Übertragungsregel
 * @param call_chain 调用链 / Call chain / Aufrufkette
 * @param call_chain_size 调用链大小 / Call chain size / Aufrufketten-Größe
 * @return 检测到循环返回-1，否则返回0 / Returns -1 if cycle detected, 0 otherwise / Gibt -1 zurück, wenn Zyklus erkannt, sonst 0
 */
int detect_call_cycle(const pointer_transfer_rule_t* rule, const char* call_chain[], size_t call_chain_size);

/**
 * @brief 检查递归深度 / Check recursion depth / Rekursionstiefe prüfen
 * @param recursion_depth 递归深度 / Recursion depth / Rekursionstiefe
 */
void check_recursion_depth(int recursion_depth);

/**
 * @brief 加载目标插件并获取函数指针 / Load target plugin and get function pointer / Ziel-Plugin laden und Funktionszeiger abrufen
 * @param rule 传递规则 / Transfer rule / Übertragungsregel
 * @param handle_out 输出插件句柄 / Output plugin handle / Ausgabe-Plugin-Handle
 * @param func_ptr_out 输出函数指针 / Output function pointer / Ausgabe-Funktionszeiger
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int load_plugin_and_get_function(const pointer_transfer_rule_t* rule, void** handle_out, void** func_ptr_out);

/**
 * @brief 准备返回值类型和缓冲区 / Prepare return type and buffer / Rückgabetyp und Puffer vorbereiten
 * @param state 接口状态 / Interface state / Schnittstellenstatus
 * @param return_type_out 输出返回值类型 / Output return type / Ausgabe-Rückgabetyp
 * @param return_size_out 输出返回值大小 / Output return size / Ausgabe-Rückgabegröße
 * @param struct_buffer_out 输出结构体缓冲区 / Output struct buffer / Ausgabe-Strukturpuffer
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int prepare_return_type_and_buffer(target_interface_state_t* state, pt_return_type_t* return_type_out, 
                                    size_t* return_size_out, void** struct_buffer_out);

/**
 * @brief 调用函数并获取返回值 / Call function and get return value / Funktion aufrufen und Rückgabewert abrufen
 * @param state 接口状态 / Interface state / Schnittstellenstatus
 * @param actual_param_count 实际参数数量 / Actual parameter count / Tatsächliche Parameteranzahl
 * @param return_type 返回值类型 / Return type / Rückgabetyp
 * @param return_size 返回值大小 / Return size / Rückgabegröße
 * @param struct_buffer 结构体缓冲区 / Struct buffer / Strukturpuffer
 * @param result_int_out 输出整数结果 / Output integer result / Ausgabe-Ganzzahlergebnis
 * @param result_float_out 输出浮点数结果 / Output float result / Ausgabe-Gleitkommaergebnis
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int call_function_and_get_result(target_interface_state_t* state, int actual_param_count,
                                  pt_return_type_t return_type, size_t return_size, void* struct_buffer,
                                  int64_t* result_int_out, double* result_float_out);

/**
 * @brief 记录返回值 / Log return value / Rückgabewert protokollieren
 * @param plugin_name 插件名称 / Plugin name / Plugin-Name
 * @param interface_name 接口名称 / Interface name / Schnittstellenname
 * @param return_type 返回值类型 / Return type / Rückgabetyp
 * @param return_size 返回值大小 / Return size / Rückgabegröße
 * @param result_int 整数结果 / Integer result / Ganzzahlergebnis
 * @param result_float 浮点数结果 / Float result / Gleitkommaergebnis
 * @param struct_buffer 结构体缓冲区 / Struct buffer / Strukturpuffer
 */
void log_return_value(const char* plugin_name, const char* interface_name, pt_return_type_t return_type,
                      size_t return_size, int64_t result_int, double result_float, void* struct_buffer);

/**
 * @brief 根据返回值类型选择传递参数 / Select transfer parameter by return type / Übertragungsparameter nach Rückgabetyp auswählen
 * @param return_type 返回值类型 / Return type / Rückgabetyp
 * @param return_size 返回值大小 / Return size / Rückgabegröße
 * @param result_int 整数结果 / Integer result / Ganzzahlergebnis
 * @param result_float 浮点数结果 / Float result / Gleitkommaergebnis
 * @param struct_buffer 结构体缓冲区 / Struct buffer / Strukturpuffer
 * @param ctx 上下文 / Context / Kontext
 * @return 传递参数指针 / Transfer parameter pointer / Übertragungsparameter-Zeiger
 */
void* select_transfer_parameter_by_return_type(pt_return_type_t return_type, size_t return_size,
                                                int64_t result_int, double result_float, void* struct_buffer,
                                                pointer_transfer_context_t* ctx);

/**
 * @brief 收集匹配的返回值传递规则 / Collect matching return value transfer rules / Passende Rückgabewert-Übertragungsregeln sammeln
 * @param ctx 上下文 / Context / Kontext
 * @param source_plugin 源插件名称 / Source plugin name / Quell-Plugin-Name
 * @param source_interface 源接口名称 / Source interface name / Quell-Schnittstellenname
 * @param matched_rules 输出匹配的规则索引数组 / Output matched rule indices array / Ausgabe-Array mit passenden Regelindizes
 * @param max_matched 最大匹配数量 / Maximum match count / Maximale Übereinstimmungsanzahl
 * @return 匹配的规则数量 / Number of matched rules / Anzahl der passenden Regeln
 */
size_t collect_matching_return_value_rules(pointer_transfer_context_t* ctx, const char* source_plugin, 
                                            const char* source_interface, size_t* matched_rules, size_t max_matched);

/**
 * @brief 检查是否有完全相同的目标位置 / Check if exact duplicate target location exists / Prüfen, ob exakt doppelte Zielposition existiert
 * @param ctx 上下文 / Context / Kontext
 * @param rule 当前规则 / Current rule / Aktuelle Regel
 * @param source_plugin 源插件名称 / Source plugin name / Quell-Plugin-Name
 * @param source_interface 源接口名称 / Source interface name / Quell-Schnittstellenname
 * @param start_index 开始搜索的索引 / Start search index / Start-Suchindex
 * @return 找到返回1，否则返回0 / Returns 1 if found, 0 otherwise / Gibt 1 zurück, wenn gefunden, sonst 0
 */
int check_exact_duplicate_target(const pointer_transfer_context_t* ctx, const pointer_transfer_rule_t* rule,
                                  const char* source_plugin, const char* source_interface, size_t start_index);

/**
 * @brief 处理参数值传递规则 / Process parameter value transfer rules / Parameterwert-Übertragungsregeln verarbeiten
 * @param ctx 上下文 / Context / Kontext
 * @param rule 当前规则 / Current rule / Aktuelle Regel
 * @param state 接口状态 / Interface state / Schnittstellenstatus
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int process_parameter_value_transfer_rules(pointer_transfer_context_t* ctx, const pointer_transfer_rule_t* rule,
                                            target_interface_state_t* state);

/**
 * @brief 构建新的调用链 / Build new call chain / Neue Aufrufkette erstellen
 * @param call_chain 原始调用链 / Original call chain / Ursprüngliche Aufrufkette
 * @param call_chain_size 原始调用链大小 / Original call chain size / Ursprüngliche Aufrufketten-Größe
 * @param plugin_name 插件名称 / Plugin name / Plugin-Name
 * @param interface_name 接口名称 / Interface name / Schnittstellenname
 * @param new_call_chain_out 输出新调用链 / Output new call chain / Ausgabe-Neue Aufrufkette
 * @param new_call_chain_size_out 输出新调用链大小 / Output new call chain size / Ausgabe-Neue Aufrufketten-Größe
 */
void build_new_call_chain(const char* call_chain[], size_t call_chain_size,
                          const char* plugin_name, const char* interface_name,
                          const char* new_call_chain_out[64], size_t* new_call_chain_size_out);

/**
 * @brief 清理接口状态参数 / Cleanup interface state parameters / Schnittstellenstatus-Parameter bereinigen
 * @param state 接口状态 / Interface state / Schnittstellenstatus
 */
void cleanup_interface_state_parameters(target_interface_state_t* state);

/**
 * @brief 清理SetGroup目标接口的参数状态 / Cleanup parameter state of SetGroup target interface / Parameterstatus der SetGroup-Zielschnittstelle bereinigen
 * @param ctx 上下文 / Context / Kontext
 * @param plugin_name 插件名称 / Plugin name / Plugin-Name
 * @param interface_name 接口名称 / Interface name / Schnittstellenname
 */
void cleanup_setgroup_target_interface_parameters(pointer_transfer_context_t* ctx, const char* plugin_name, const char* interface_name);

/**
 * @brief 收集同一SetGroup中的所有规则 / Collect all rules in same SetGroup / Alle Regeln in derselben SetGroup sammeln
 * @param ctx 上下文 / Context / Kontext
 * @param matched_rules 匹配的规则索引数组 / Matched rule indices array / Array mit passenden Regelindizes
 * @param matched_count 匹配的规则数量 / Matched rule count / Anzahl der passenden Regeln
 * @param processed 已处理标记数组 / Processed flags array / Array mit Verarbeitungsmarkierungen
 * @param active_rule 当前活动规则 / Current active rule / Aktuelle aktive Regel
 * @param group_rules_out 输出组规则索引数组 / Output group rule indices array / Ausgabe-Array mit Gruppenregelindizes
 * @param max_group_rules 最大组规则数量 / Maximum group rules count / Maximale Gruppenregelanzahl
 * @return 组规则数量 / Group rule count / Gruppenregelanzahl
 */
size_t collect_setgroup_rules(pointer_transfer_context_t* ctx, const size_t* matched_rules, size_t matched_count,
                               int* processed, const pointer_transfer_rule_t* active_rule,
                               size_t* group_rules_out, size_t max_group_rules);

/**
 * @brief 按target_param_index排序规则 / Sort rules by target_param_index / Regeln nach target_param_index sortieren
 * @param ctx 上下文 / Context / Kontext
 * @param group_rules 组规则索引数组 / Group rule indices array / Array mit Gruppenregelindizes
 * @param group_count 组规则数量 / Group rule count / Gruppenregelanzahl
 */
void sort_setgroup_rules_by_param_index(pointer_transfer_context_t* ctx, size_t* group_rules, size_t group_count);

/**
 * @brief 重新调用源接口获取新的返回值 / Re-call source interface to get new return value / Quellschnittstelle neu aufrufen, um neuen Rückgabewert zu erhalten
 * @param group_rule SetGroup规则 / SetGroup rule / SetGroup-Regel
 * @param result_int_out 输出整数结果 / Output integer result / Ausgabe-Ganzzahlergebnis
 * @param result_float_out 输出浮点数结果 / Output float result / Ausgabe-Gleitkommaergebnis
 * @param return_type_out 输出返回值类型 / Output return type / Ausgabe-Rückgabetyp
 * @param return_size_out 输出返回值大小 / Output return size / Ausgabe-Rückgabegröße
 * @param struct_buffer_out 输出结构体缓冲区 / Output struct buffer / Ausgabe-Strukturpuffer
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int recall_source_interface_for_setgroup(const pointer_transfer_rule_t* group_rule,
                                          int64_t* result_int_out, double* result_float_out,
                                          pt_return_type_t* return_type_out, size_t* return_size_out,
                                          void** struct_buffer_out);

/**
 * @brief 检查SetGroup参数就绪状态 / Check SetGroup parameter readiness / SetGroup-Parameterbereitschaft prüfen
 * @param group_rule SetGroup规则 / SetGroup rule / SetGroup-Regel
 * @return 可以应用返回1，否则返回0 / Returns 1 if can apply, 0 otherwise / Gibt 1 zurück, wenn anwendbar, sonst 0
 */
int check_setgroup_parameter_readiness(const pointer_transfer_rule_t* group_rule);

/**
 * @brief 检查SetGroup中是否有更多规则要处理 / Check if there are more rules in SetGroup to process / Prüfen, ob noch weitere Regeln in SetGroup zu verarbeiten sind
 * @param ctx 上下文 / Context / Kontext
 * @param group_rules 组规则索引数组 / Group rule indices array / Array mit Gruppenregelindizes
 * @param group_count 组规则数量 / Group rule count / Gruppenregelanzahl
 * @param group_idx 当前组索引 / Current group index / Aktueller Gruppenindex
 * @param group_rule 当前组规则 / Current group rule / Aktuelle Gruppenregel
 * @return 有更多规则返回1，否则返回0 / Returns 1 if more rules, 0 otherwise / Gibt 1 zurück, wenn weitere Regeln, sonst 0
 */
int check_more_rules_in_setgroup(pointer_transfer_context_t* ctx, const size_t* group_rules, size_t group_count,
                                  size_t group_idx, const pointer_transfer_rule_t* group_rule);

/**
 * @brief 检查SetGroup设置组状态 / Check SetGroup set group status / SetGroup-Set-Gruppenstatus prüfen
 * @param ctx 上下文 / Context / Kontext
 * @param group_rule SetGroup规则 / SetGroup rule / SetGroup-Regel
 * @param rule_idx 规则索引 / Rule index / Regelindex
 * @param should_check_group_out 输出是否需要检查组 / Output whether need to check group / Ausgabe, ob Gruppe geprüft werden muss
 * @param is_min_param_index_out 输出是否是最小参数索引 / Output whether is minimum parameter index / Ausgabe, ob minimaler Parameterindex
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int check_setgroup_set_group_status(pointer_transfer_context_t* ctx, const pointer_transfer_rule_t* group_rule,
                                     size_t rule_idx, int* should_check_group_out, int* is_min_param_index_out);

/**
 * @brief 调用目标插件接口（内部实现） / Call target plugin interface (internal implementation) / Ziel-Plugin-Schnittstelle aufrufen (interne Implementierung)
 * @param rule 传递规则 / Transfer rule / Übertragungsregel
 * @param ptr 要传递的指针 / Pointer to transfer / Zu übertragender Zeiger
 * @param recursion_depth 递归深度 / Recursion depth / Rekursionstiefe
 * @param call_chain 调用链 / Call chain / Aufrufkette
 * @param call_chain_size 调用链大小 / Call chain size / Aufrufketten-Größe
 * @param skip_param_cleanup 是否跳过参数清理 / Whether to skip parameter cleanup / Ob Parameterbereinigung übersprungen werden soll
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int call_target_plugin_interface_internal(const pointer_transfer_rule_t* rule, void* ptr, int recursion_depth, const char* call_chain[], size_t call_chain_size, int skip_param_cleanup);

/**
 * @brief 执行SetGroup规则 / Execute SetGroup rule / SetGroup-Regel ausführen
 * @param group_rule SetGroup规则 / SetGroup rule / SetGroup-Regel
 * @param rule 当前规则 / Current rule / Aktuelle Regel
 * @param group_result_int 组整数结果 / Group integer result / Gruppen-Ganzzahlergebnis
 * @param group_result_float 组浮点数结果 / Group float result / Gruppen-Gleitkommaergebnis
 * @param group_return_type 组返回值类型 / Group return type / Gruppen-Rückgabetyp
 * @param group_return_size 组返回值大小 / Group return size / Gruppen-Rückgabegröße
 * @param group_struct_buffer 组结构体缓冲区 / Group struct buffer / Gruppen-Strukturpuffer
 * @param call_chain 调用链 / Call chain / Aufrufkette
 * @param call_chain_size 调用链大小 / Call chain size / Aufrufketten-Größe
 * @param recursion_depth 递归深度 / Recursion depth / Rekursionstiefe
 * @param rule_idx 规则索引 / Rule index / Regelindex
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int execute_setgroup_rule(const pointer_transfer_rule_t* group_rule, const pointer_transfer_rule_t* rule,
                          int64_t group_result_int, double group_result_float,
                          pt_return_type_t group_return_type, size_t group_return_size, void* group_struct_buffer,
                          const char* call_chain[], size_t call_chain_size, int recursion_depth, size_t rule_idx);

/**
 * @brief 执行非SetGroup规则 / Execute non-SetGroup rule / Nicht-SetGroup-Regel ausführen
 * @param active_rule 活动规则 / Active rule / Aktive Regel
 * @param rule 当前规则 / Current rule / Aktuelle Regel
 * @param return_type 返回值类型 / Return type / Rückgabetyp
 * @param return_size 返回值大小 / Return size / Rückgabegröße
 * @param result_int 整数结果 / Integer result / Ganzzahlergebnis
 * @param result_float 浮点数结果 / Float result / Gleitkommaergebnis
 * @param struct_buffer 结构体缓冲区 / Struct buffer / Strukturpuffer
 * @param call_chain 调用链 / Call chain / Aufrufkette
 * @param call_chain_size 调用链大小 / Call chain size / Aufrufketten-Größe
 * @param recursion_depth 递归深度 / Recursion depth / Rekursionstiefe
 * @param rule_idx 规则索引 / Rule index / Regelindex
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int execute_non_setgroup_rule(const pointer_transfer_rule_t* active_rule, const pointer_transfer_rule_t* rule,
                               pt_return_type_t return_type, size_t return_size,
                               int64_t result_int, double result_float, void* struct_buffer,
                               const char* call_chain[], size_t call_chain_size, int recursion_depth, size_t rule_idx);

/**
 * @brief 验证可变参数接口的最小参数要求 / Validate minimum parameter requirement for variadic interface / Mindestparameteranforderung für variablen Parameter-Interface validieren
 * @param state 接口状态 / Interface state / Schnittstellenstatus
 * @param actual_param_count 实际参数数量 / Actual parameter count / Tatsächliche Parameteranzahl
 * @param plugin_name 插件名称 / Plugin name / Plugin-Name
 * @param interface_name 接口名称 / Interface name / Schnittstellenname
 * @return 验证通过返回0，失败返回-1 / Returns 0 if validation passes, -1 on failure / Gibt 0 zurück, wenn Validierung erfolgreich, -1 bei Fehler
 */
int validate_variadic_min_param_requirement(target_interface_state_t* state, int actual_param_count,
                                             const char* plugin_name, const char* interface_name);

/**
 * @brief 验证插件函数 / Validate plugin function / Plugin-Funktion validieren
 * @param state 接口状态 / Interface state / Schnittstellenstatus
 * @param plugin_path 插件路径 / Plugin path / Plugin-Pfad
 * @param interface_name 接口名称 / Interface name / Schnittstellenname
 * @param actual_param_count 实际参数数量 / Actual parameter count / Tatsächliche Parameteranzahl
 * @param return_type 返回值类型 / Return type / Rückgabetyp
 * @return 验证通过返回0，失败返回-1 / Returns 0 if validation passes, -1 on failure / Gibt 0 zurück, wenn Validierung erfolgreich, -1 bei Fehler
 */
int validate_plugin_function(target_interface_state_t* state, const char* plugin_path, const char* interface_name,
                              int actual_param_count, pt_return_type_t return_type);

/**
 * @brief 处理SetGroup规则组 / Process SetGroup rule group / SetGroup-Regelgruppe verarbeiten
 * @param ctx 上下文 / Context / Kontext
 * @param active_rule 活动规则 / Active rule / Aktive Regel
 * @param matched_rules 匹配的规则索引数组 / Matched rule indices array / Array mit passenden Regelindizes
 * @param matched_count 匹配的规则数量 / Matched rule count / Anzahl der passenden Regeln
 * @param processed 已处理标记数组 / Processed flags array / Array mit Verarbeitungsmarkierungen
 * @param rule 当前规则 / Current rule / Aktuelle Regel
 * @param return_type 返回值类型 / Return type / Rückgabetyp
 * @param return_size 返回值大小 / Return size / Rückgabegröße
 * @param result_int 整数结果 / Integer result / Ganzzahlergebnis
 * @param result_float 浮点数结果 / Float result / Gleitkommaergebnis
 * @param struct_buffer 结构体缓冲区 / Struct buffer / Strukturpuffer
 * @param call_chain 调用链 / Call chain / Aufrufkette
 * @param call_chain_size 调用链大小 / Call chain size / Aufrufketten-Größe
 * @param recursion_depth 递归深度 / Recursion depth / Rekursionstiefe
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int process_setgroup_rule_group(pointer_transfer_context_t* ctx, const pointer_transfer_rule_t* active_rule,
                                 const size_t* matched_rules, size_t matched_count, int* processed,
                                 const pointer_transfer_rule_t* rule, pt_return_type_t return_type, size_t return_size,
                                 int64_t result_int, double result_float, void* struct_buffer,
                                 const char* call_chain[], size_t call_chain_size, int recursion_depth);

/**
 * @brief 处理非SetGroup规则 / Process non-SetGroup rule / Nicht-SetGroup-Regel verarbeiten
 * @param active_rule 活动规则 / Active rule / Aktive Regel
 * @param rule 当前规则 / Current rule / Aktuelle Regel
 * @param return_type 返回值类型 / Return type / Rückgabetyp
 * @param return_size 返回值大小 / Return size / Rückgabegröße
 * @param result_int 整数结果 / Integer result / Ganzzahlergebnis
 * @param result_float 浮点数结果 / Float result / Gleitkommaergebnis
 * @param struct_buffer 结构体缓冲区 / Struct buffer / Strukturpuffer
 * @param call_chain 调用链 / Call chain / Aufrufkette
 * @param call_chain_size 调用链大小 / Call chain size / Aufrufketten-Größe
 * @param recursion_depth 递归深度 / Recursion depth / Rekursionstiefe
 * @param rule_idx 规则索引 / Rule index / Regelindex
 * @return 成功返回0，需要停止返回1，失败返回-1 / Returns 0 on success, 1 if should stop, -1 on failure / Gibt 0 bei Erfolg zurück, 1 wenn gestoppt werden soll, -1 bei Fehler
 */
int process_non_setgroup_rule(const pointer_transfer_rule_t* active_rule, const pointer_transfer_rule_t* rule,
                               pt_return_type_t return_type, size_t return_size,
                               int64_t result_int, double result_float, void* struct_buffer,
                               const char* call_chain[], size_t call_chain_size, int recursion_depth, size_t rule_idx);

/**
 * @brief 准备接口调用 / Prepare interface call / Schnittstellenaufruf vorbereiten
 * @param rule 传递规则 / Transfer rule / Übertragungsregel
 * @param ptr 要传递的指针 / Pointer to transfer / Zu übertragender Zeiger
 * @param state_out 输出接口状态 / Output interface state / Ausgabe-Schnittstellenstatus
 * @param actual_param_count_out 输出实际参数数量 / Output actual parameter count / Ausgabe-Tatsächliche Parameteranzahl
 * @param return_type_out 输出返回值类型 / Output return type / Ausgabe-Rückgabetyp
 * @param return_size_out 输出返回值大小 / Output return size / Ausgabe-Rückgabegröße
 * @param struct_buffer_out 输出结构体缓冲区 / Output struct buffer / Ausgabe-Strukturpuffer
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int prepare_interface_call(const pointer_transfer_rule_t* rule, void* ptr,
                           target_interface_state_t** state_out, int* actual_param_count_out,
                           pt_return_type_t* return_type_out, size_t* return_size_out, void** struct_buffer_out);

/**
 * @brief 执行接口调用 / Execute interface call / Schnittstellenaufruf ausführen
 * @param state 接口状态 / Interface state / Schnittstellenstatus
 * @param rule 传递规则 / Transfer rule / Übertragungsregel
 * @param actual_param_count 实际参数数量 / Actual parameter count / Tatsächliche Parameteranzahl
 * @param return_type 返回值类型 / Return type / Rückgabetyp
 * @param return_size 返回值大小 / Return size / Rückgabegröße
 * @param struct_buffer 结构体缓冲区 / Struct buffer / Strukturpuffer
 * @param result_int_out 输出整数结果 / Output integer result / Ausgabe-Ganzzahlergebnis
 * @param result_float_out 输出浮点数结果 / Output float result / Ausgabe-Gleitkommaergebnis
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int execute_interface_call(target_interface_state_t* state, const pointer_transfer_rule_t* rule,
                           int actual_param_count, pt_return_type_t return_type, size_t return_size, void* struct_buffer,
                           int64_t* result_int_out, double* result_float_out);

/**
 * @brief 处理返回值传递规则 / Process return value transfer rules / Rückgabewert-Übertragungsregeln verarbeiten
 * @param ctx 上下文 / Context / Kontext
 * @param rule 当前规则 / Current rule / Aktuelle Regel
 * @param return_type 返回值类型 / Return type / Rückgabetyp
 * @param return_size 返回值大小 / Return size / Rückgabegröße
 * @param result_int 整数结果 / Integer result / Ganzzahlergebnis
 * @param result_float 浮点数结果 / Float result / Gleitkommaergebnis
 * @param struct_buffer 结构体缓冲区 / Struct buffer / Strukturpuffer
 * @param call_chain 调用链 / Call chain / Aufrufkette
 * @param call_chain_size 调用链大小 / Call chain size / Aufrufketten-Größe
 * @param recursion_depth 递归深度 / Recursion depth / Rekursionstiefe
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int process_return_value_transfer_rules(pointer_transfer_context_t* ctx, const pointer_transfer_rule_t* rule,
                                         pt_return_type_t return_type, size_t return_size,
                                         int64_t result_int, double result_float, void* struct_buffer,
                                         const char* call_chain[], size_t call_chain_size, int recursion_depth);

/**
 * @brief 清理接口调用资源 / Cleanup interface call resources / Schnittstellenaufruf-Ressourcen bereinigen
 * @param return_type 返回值类型 / Return type / Rückgabetyp
 * @param struct_buffer 结构体缓冲区 / Struct buffer / Strukturpuffer
 * @param state 接口状态 / Interface state / Schnittstellenstatus
 * @param skip_param_cleanup 是否跳过参数清理 / Whether to skip parameter cleanup / Ob Parameterbereinigung übersprungen werden soll
 */
void cleanup_interface_call_resources(pt_return_type_t return_type, void* struct_buffer,
                                      target_interface_state_t* state, int skip_param_cleanup);

#ifdef __cplusplus
}
#endif

#endif /* POINTER_TRANSFER_INTERFACE_H */

