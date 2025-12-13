/**
 * @file pointer_transfer_interface_state_create.c
 * @brief 接口状态创建 / Interface State Creation / Schnittstellenstatus-Erstellung
 */

#include "pointer_transfer_interface.h"
#include "pointer_transfer_context.h"
#include "pointer_transfer_utils.h"
#include "nxld_plugin_interface.h"
#include "pointer_transfer_types.h"
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

/**
 * @brief 计算参数计数 / Calculate parameter count / Parameteranzahl berechnen
 * @param param_count_type 参数计数类型 / Parameter count type / Parameteranzahl-Typ
 * @param min_count 最小数量 / Minimum count / Mindestanzahl
 * @param max_count 最大数量 / Maximum count / Maximalanzahl
 * @param is_variadic_out 输出是否可变参数 / Output whether variadic / Ausgabe, ob variabel
 * @return 参数计数 / Parameter count / Parameteranzahl
 */
int calculate_param_count(nxld_param_count_type_t param_count_type, int32_t min_count, int32_t max_count,
                          int* is_variadic_out) {
    if (is_variadic_out == NULL) {
        return 0;
    }
    
    int param_count = (int)min_count;
    int is_variadic = 0;
    
    if (param_count_type == NXLD_PARAM_COUNT_FIXED && max_count > 0) {
        param_count = max_count;
    } else if (param_count_type == NXLD_PARAM_COUNT_VARIABLE) {
        param_count = max_count > 0 ? max_count : min_count;
        is_variadic = 1;
    }
    
    *is_variadic_out = is_variadic;
    return param_count;
}

/**
 * @brief 分配参数数组 / Allocate parameter arrays / Parameterarrays zuweisen
 * @param state 接口状态 / Interface state / Schnittstellenstatus
 * @param param_count 参数数量 / Parameter count / Parameteranzahl
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int allocate_parameter_arrays(target_interface_state_t* state, int param_count) {
    if (state == NULL || param_count <= 0) {
        return 0;
    }
    
    state->param_ready = (int*)calloc(param_count, sizeof(int));
    state->param_values = (void**)calloc(param_count, sizeof(void*));
    state->param_types = (nxld_param_type_t*)calloc(param_count, sizeof(nxld_param_type_t));
    state->param_sizes = (size_t*)calloc(param_count, sizeof(size_t));
    state->param_int_values = (int64_t*)calloc(param_count, sizeof(int64_t));
    state->param_float_values = (double*)calloc(param_count, sizeof(double));
    
    if (state->param_ready == NULL || state->param_values == NULL ||
        state->param_types == NULL || state->param_sizes == NULL || 
        state->param_int_values == NULL || state->param_float_values == NULL) {
        return -1;
    }
    
    return 0;
}

/**
 * @brief 释放参数数组 / Free parameter arrays / Parameterarrays freigeben
 * @param state 接口状态 / Interface state / Schnittstellenstatus
 */
void free_parameter_arrays(target_interface_state_t* state) {
    if (state == NULL) {
        return;
    }
    
    if (state->param_ready != NULL) {
        free(state->param_ready);
        state->param_ready = NULL;
    }
    if (state->param_values != NULL) {
        free(state->param_values);
        state->param_values = NULL;
    }
    if (state->param_types != NULL) {
        free(state->param_types);
        state->param_types = NULL;
    }
    if (state->param_sizes != NULL) {
        free(state->param_sizes);
        state->param_sizes = NULL;
    }
    if (state->param_int_values != NULL) {
        free(state->param_int_values);
        state->param_int_values = NULL;
    }
    if (state->param_float_values != NULL) {
        free(state->param_float_values);
        state->param_float_values = NULL;
    }
}

/**
 * @brief 初始化参数类型 / Initialize parameter types / Parametertypen initialisieren
 * @param state 接口状态 / Interface state / Schnittstellenstatus
 * @param get_param_info 参数信息函数 / Parameter info function / Parameter-Info-Funktion
 * @param interface_index 接口索引 / Interface index / Schnittstellenindex
 * @param param_count 参数数量 / Parameter count / Parameteranzahl
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int initialize_parameter_types(target_interface_state_t* state, void* get_param_info,
                                size_t interface_index, int param_count) {
    if (state == NULL || get_param_info == NULL || param_count <= 0) {
        return 0;
    }
    
    typedef int32_t (NXLD_PLUGIN_CALL *get_interface_param_info_func)(size_t, int32_t, char*, size_t, nxld_param_type_t*, char*, size_t);
    get_interface_param_info_func info_func = (get_interface_param_info_func)get_param_info;
    
    size_t param_name_buf_size = 512;
    char* param_name = (char*)malloc(param_name_buf_size);
    if (param_name == NULL) {
        return -1;
    }
    
    size_t type_name_buf_size = 512;
    char* type_name = (char*)malloc(type_name_buf_size);
    if (type_name == NULL) {
        free(param_name);
        return -1;
    }
    
    if (state->param_types != NULL) {
        for (int i = 0; i < param_count; i++) {
            nxld_param_type_t param_type;
            if (info_func(interface_index, i, param_name, param_name_buf_size,
                          &param_type, type_name, type_name_buf_size) == 0) {
                state->param_types[i] = param_type;
            }
        }
    }
    
    free(param_name);
    free(type_name);
    
    return 0;
}

/**
 * @brief 初始化接口状态基本字段 / Initialize interface state basic fields / Grundfelder des Schnittstellenstatus initialisieren
 * @param state 接口状态 / Interface state / Schnittstellenstatus
 * @param plugin_name 插件名称 / Plugin name / Plugin-Name
 * @param interface_name 接口名称 / Interface name / Schnittstellenname
 * @param handle 插件句柄 / Plugin handle / Plugin-Handle
 * @param func_ptr 函数指针 / Function pointer / Funktionszeiger
 * @param param_count 参数数量 / Parameter count / Parameteranzahl
 * @param is_variadic 是否可变参数 / Whether variadic / Ob variabel
 * @param min_param_count 最小参数数量 / Minimum parameter count / Mindestparameteranzahl
 * @param return_type 返回类型 / Return type / Rückgabetyp
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int initialize_interface_state_basic(target_interface_state_t* state,
                                      const char* plugin_name, const char* interface_name,
                                      void* handle, void* func_ptr,
                                      int param_count, int is_variadic, int min_param_count,
                                      pt_return_type_t return_type) {
    if (state == NULL || plugin_name == NULL || interface_name == NULL) {
        return -1;
    }
    
    state->plugin_name = allocate_string(plugin_name);
    state->interface_name = allocate_string(interface_name);
    state->handle = handle;
    state->func_ptr = func_ptr;
    state->param_count = param_count;
    state->is_variadic = is_variadic;
    state->min_param_count = min_param_count;
    state->actual_param_count = param_count;
    state->return_type = return_type;
    state->return_size = 0;
    state->in_use = 0;
    state->validation_done = 0;
    
    if (state->plugin_name == NULL || state->interface_name == NULL) {
        return -1;
    }
    
    if (param_count > 0 && state->param_ready != NULL && state->param_values != NULL) {
        for (int i = 0; i < param_count; i++) {
            state->param_ready[i] = 0;
            state->param_values[i] = NULL;
        }
    }
    
    return 0;
}

