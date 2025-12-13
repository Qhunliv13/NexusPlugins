/**
 * @file pointer_transfer_interface_state.c
 * @brief 接口状态管理 / Interface State Management / Schnittstellenstatus-Verwaltung
 * 
 * 此文件现在作为接口状态管理的入口点，具体实现已拆分到：
 * - pointer_transfer_interface_state_find.c: 接口状态查找
 * - pointer_transfer_interface_state_info.c: 接口信息获取
 * - pointer_transfer_interface_state_create.c: 接口状态创建
 */

#include "pointer_transfer_interface.h"
#include "pointer_transfer_context.h"
#include "pointer_transfer_utils.h"
#include "pointer_transfer_types.h"
#include "pointer_transfer_platform.h"
#include "nxld_plugin_interface.h"
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <limits.h>

/**
 * @brief 查找或创建目标接口状态 / Find or create target interface state / Ziel-Schnittstellenstatus suchen oder erstellen
 */
target_interface_state_t* find_or_create_interface_state(const char* plugin_name, const char* interface_name, void* handle, void* func_ptr) {
    if (plugin_name == NULL || interface_name == NULL || handle == NULL || func_ptr == NULL) {
        return NULL;
    }
    
    /* 首先尝试查找已存在的接口状态 / First try to find existing interface state / Zuerst versuchen, vorhandenen Schnittstellenstatus zu finden */
    target_interface_state_t* existing_state = find_interface_state(plugin_name, interface_name);
    if (existing_state != NULL) {
        return existing_state;
    }
    
    pointer_transfer_context_t* ctx = get_global_context();
    
    void* get_interface_count = NULL;
    void* get_interface_info = NULL;
    void* get_param_count = NULL;
    void* get_param_info = NULL;
    
    if (get_plugin_interface_functions(handle, &get_interface_count, &get_interface_info,
                                       &get_param_count, &get_param_info) != 0) {
        internal_log_write("WARNING", "Failed to get plugin interface functions for %s", plugin_name);
        return NULL;
    }
    
    size_t target_interface_index = SIZE_MAX;
    pt_return_type_t inferred_return_type = PT_RETURN_TYPE_INTEGER;
    char* saved_desc_buf = NULL;
    
    if (find_interface_index(get_interface_count, get_interface_info, interface_name,
                             &target_interface_index, &inferred_return_type, &saved_desc_buf) != 0) {
        if (saved_desc_buf != NULL) {
            free(saved_desc_buf);
        }
        internal_log_write("WARNING", "Interface %s not found in plugin %s", interface_name, plugin_name);
        return NULL;
    }
    
    nxld_param_count_type_t param_count_type;
    int32_t min_count = 0;
    int32_t max_count = 0;
    if (get_parameter_count_info(get_param_count, target_interface_index,
                                  &param_count_type, &min_count, &max_count) != 0) {
        if (saved_desc_buf != NULL) {
            free(saved_desc_buf);
        }
        return NULL;
    }
    
    int is_variadic = 0;
    int param_count = calculate_param_count(param_count_type, min_count, max_count, &is_variadic);
    if (is_variadic) {
        internal_log_write("INFO", "Interface %s.%s has variadic parameters (min=%d, max=%d)", 
                      plugin_name, interface_name, min_count, max_count);
    }
    
    if (ctx->interface_state_count >= ctx->interface_state_capacity) {
        if (expand_interface_states_capacity() != 0) {
            internal_log_write("ERROR", "Failed to expand interface states capacity");
            if (saved_desc_buf != NULL) {
                free(saved_desc_buf);
            }
            return NULL;
        }
    }
    
    if (ctx->interface_states == NULL) {
        internal_log_write("ERROR", "Interface states array is NULL after capacity expansion");
        if (saved_desc_buf != NULL) {
            free(saved_desc_buf);
        }
        return NULL;
    }
    
    target_interface_state_t* state = &ctx->interface_states[ctx->interface_state_count];
    int min_param_count = is_variadic ? (int)min_count : param_count;
    
    if (initialize_interface_state_basic(state, plugin_name, interface_name, handle, func_ptr,
                                         param_count, is_variadic, min_param_count, inferred_return_type) != 0) {
        if (saved_desc_buf != NULL) {
            free(saved_desc_buf);
        }
        return NULL;
    }
    
    if (param_count > 0) {
        if (allocate_parameter_arrays(state, param_count) != 0) {
            if (state->plugin_name != NULL) free(state->plugin_name);
            if (state->interface_name != NULL) free(state->interface_name);
            free_parameter_arrays(state);
            if (saved_desc_buf != NULL) {
                free(saved_desc_buf);
            }
            return NULL;
        }
        
        if (initialize_parameter_types(state, get_param_info, target_interface_index, param_count) != 0) {
            if (state->plugin_name != NULL) free(state->plugin_name);
            if (state->interface_name != NULL) free(state->interface_name);
            free_parameter_arrays(state);
            if (saved_desc_buf != NULL) {
                free(saved_desc_buf);
            }
            return NULL;
        }
    } else {
        state->param_ready = NULL;
        state->param_values = NULL;
        state->param_types = NULL;
        state->param_sizes = NULL;
        state->param_int_values = NULL;
        state->param_float_values = NULL;
    }
    
    ctx->interface_state_count++;
    
    if (state->return_type == PT_RETURN_TYPE_STRUCT_PTR) {
        internal_log_write("INFO", "Struct return type detected for %s.%s from description, return type set to pointer", plugin_name, interface_name);
    }
    
    if (saved_desc_buf != NULL) {
        free(saved_desc_buf);
    }
    
    internal_log_write("INFO", "Created interface state for %s.%s with %d parameters, return_type=%d", 
                  plugin_name, interface_name, param_count, state->return_type);
    
    return state;
}

