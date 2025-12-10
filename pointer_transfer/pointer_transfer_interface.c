/**
 * @file pointer_transfer_interface.c
 * @brief 指针传递插件接口调用 / Pointer Transfer Plugin Interface Invocation / Zeigerübertragungs-Plugin-Schnittstellenaufruf
 */

#include "pointer_transfer_interface.h"
#include "pointer_transfer_context.h"
#include "pointer_transfer_config.h"
#include "pointer_transfer_plugin_loader.h"
#include "pointer_transfer_utils.h"
#include "pointer_transfer_types.h"
#include "pointer_transfer_platform.h"
#include "pointer_transfer_currying.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <stdint.h>

/* call_function_generic已迁移至pointer_transfer_platform.c，替代函数为pt_platform_safe_call / call_function_generic migrated to pointer_transfer_platform.c, replaced by pt_platform_safe_call / call_function_generic nach pointer_transfer_platform.c migriert, ersetzt durch pt_platform_safe_call */

/**
 * @brief 查找或创建目标接口状态 / Find or create target interface state / Ziel-Schnittstellenstatus suchen oder erstellen
 */
target_interface_state_t* find_or_create_interface_state(const char* plugin_name, const char* interface_name, void* handle, void* func_ptr) {
    if (plugin_name == NULL || interface_name == NULL || handle == NULL || func_ptr == NULL) {
        return NULL;
    }
    
    pointer_transfer_context_t* ctx = get_global_context();
    for (size_t i = 0; i < ctx->interface_state_count; i++) {
        target_interface_state_t* state = &ctx->interface_states[i];
        if (state->plugin_name != NULL && state->interface_name != NULL &&
            strcmp(state->plugin_name, plugin_name) == 0 &&
            strcmp(state->interface_name, interface_name) == 0) {
            return state;
        }
    }
    
    typedef int32_t (NXLD_PLUGIN_CALL *get_interface_count_func)(size_t*);
    typedef int32_t (NXLD_PLUGIN_CALL *get_interface_info_func)(size_t, char*, size_t, char*, size_t, char*, size_t);
    typedef int32_t (NXLD_PLUGIN_CALL *get_interface_param_count_func)(size_t, nxld_param_count_type_t*, int32_t*, int32_t*);
    typedef int32_t (NXLD_PLUGIN_CALL *get_interface_param_info_func)(size_t, int32_t, char*, size_t, nxld_param_type_t*, char*, size_t);
    
    get_interface_count_func get_interface_count = (get_interface_count_func)pt_platform_get_symbol(handle, "nxld_plugin_get_interface_count");
    get_interface_info_func get_interface_info = (get_interface_info_func)pt_platform_get_symbol(handle, "nxld_plugin_get_interface_info");
    get_interface_param_count_func get_param_count = (get_interface_param_count_func)pt_platform_get_symbol(handle, "nxld_plugin_get_interface_param_count");
    get_interface_param_info_func get_param_info = (get_interface_param_info_func)pt_platform_get_symbol(handle, "nxld_plugin_get_interface_param_info");
    
    if (get_interface_count == NULL || get_interface_info == NULL || get_param_count == NULL || get_param_info == NULL) {
        internal_log_write("WARNING", "Failed to get plugin interface functions for %s", plugin_name);
        return NULL;
    }
    
    size_t interface_count = 0;
    if (get_interface_count(&interface_count) != 0) {
        return NULL;
    }
    
    size_t target_interface_index = SIZE_MAX;
    size_t iface_name_buf_size = 512;
    char* iface_name = (char*)malloc(iface_name_buf_size);
    if (iface_name == NULL) {
        return NULL;
    }
    
    size_t desc_buf_size = 512;
    char* desc_buf = (char*)malloc(desc_buf_size);
    if (desc_buf == NULL) {
        free(iface_name);
        return NULL;
    }
    
    for (size_t i = 0; i < interface_count; i++) {
        if (get_interface_info(i, iface_name, iface_name_buf_size, desc_buf, desc_buf_size, NULL, 0) == 0) {
            if (strcmp(iface_name, interface_name) == 0) {
                target_interface_index = i;
                break;
            }
        }
    }
    
    pt_return_type_t inferred_return_type = infer_return_type_from_description(desc_buf);
    char* saved_desc_buf = NULL;
    if (desc_buf != NULL && strlen(desc_buf) > 0) {
        saved_desc_buf = allocate_string(desc_buf);
        if (inferred_return_type == PT_RETURN_TYPE_STRUCT_PTR) {
            internal_log_write("INFO", "Struct return type detected for %s.%s from description, return type set to pointer", plugin_name, interface_name);
        }
    }
    
    free(iface_name);
    free(desc_buf);
    
    if (target_interface_index == SIZE_MAX) {
        if (saved_desc_buf != NULL) {
            free(saved_desc_buf);
        }
        internal_log_write("WARNING", "Interface %s not found in plugin %s", interface_name, plugin_name);
        return NULL;
    }
    
    nxld_param_count_type_t param_count_type;
    int32_t min_count = 0;
    int32_t max_count = 0;
    if (get_param_count(target_interface_index, &param_count_type, &min_count, &max_count) != 0) {
        return NULL;
    }
    
    int param_count = (int)min_count;
    int is_variadic = 0;
    if (param_count_type == NXLD_PARAM_COUNT_FIXED && max_count > 0) {
        param_count = max_count;
    } else if (param_count_type == NXLD_PARAM_COUNT_VARIABLE) {
        param_count = min_count > 0 ? min_count : 0;
        is_variadic = 1;
        internal_log_write("INFO", "Interface %s.%s has variadic parameters (min=%d, max=%d)", 
                     interface_name, plugin_name, min_count, max_count);
    }
    
    if (ctx->interface_state_count >= ctx->interface_state_capacity) {
        if (expand_interface_states_capacity() != 0) {
            internal_log_write("ERROR", "Failed to expand interface states capacity");
            return NULL;
        }
    }
    
    target_interface_state_t* state = &ctx->interface_states[ctx->interface_state_count];
    state->plugin_name = allocate_string(plugin_name);
    state->interface_name = allocate_string(interface_name);
    state->handle = handle;
    state->func_ptr = func_ptr;
    state->param_count = param_count;
    state->is_variadic = is_variadic;
    state->actual_param_count = param_count;
    state->return_type = PT_RETURN_TYPE_INTEGER;
    state->return_size = 0;
    state->in_use = 0;
    state->validation_done = 0;
    
    if (param_count > 0) {
        state->param_ready = (int*)calloc(param_count, sizeof(int));
        state->param_values = (void**)calloc(param_count, sizeof(void*));
        state->param_types = (nxld_param_type_t*)calloc(param_count, sizeof(nxld_param_type_t));
        state->param_sizes = (size_t*)calloc(param_count, sizeof(size_t));
        state->param_int_values = (int64_t*)calloc(param_count, sizeof(int64_t));
        state->param_float_values = (double*)calloc(param_count, sizeof(double));
        
        if (state->param_ready == NULL || state->param_values == NULL ||
            state->param_types == NULL || state->param_sizes == NULL || 
            state->param_int_values == NULL || state->param_float_values == NULL) {
            if (state->plugin_name != NULL) free(state->plugin_name);
            if (state->interface_name != NULL) free(state->interface_name);
            if (state->param_ready != NULL) free(state->param_ready);
            if (state->param_values != NULL) free(state->param_values);
            if (state->param_types != NULL) free(state->param_types);
            if (state->param_sizes != NULL) free(state->param_sizes);
            if (state->param_int_values != NULL) free(state->param_int_values);
            if (state->param_float_values != NULL) free(state->param_float_values);
            return NULL;
        }
        
        size_t param_name_buf_size = 512;
        char* param_name = (char*)malloc(param_name_buf_size);
        size_t type_name_buf_size = 512;
        char* type_name = (char*)malloc(type_name_buf_size);
        
        if (param_name != NULL && type_name != NULL) {
            for (int i = 0; i < param_count; i++) {
                nxld_param_type_t param_type;
                if (get_param_info(target_interface_index, i, param_name, param_name_buf_size,
                                  &param_type, type_name, type_name_buf_size) == 0) {
                    state->param_types[i] = param_type;
                }
            }
        }
        
        if (param_name != NULL) free(param_name);
        if (type_name != NULL) free(type_name);
    } else {
        state->param_ready = NULL;
        state->param_values = NULL;
        state->param_types = NULL;
        state->param_sizes = NULL;
        state->param_int_values = NULL;
        state->param_float_values = NULL;
    }
    
    if (state->plugin_name == NULL || state->interface_name == NULL) {
        if (state->plugin_name != NULL) free(state->plugin_name);
        if (state->interface_name != NULL) free(state->interface_name);
        if (state->param_ready != NULL) free(state->param_ready);
        if (state->param_values != NULL) free(state->param_values);
        if (state->param_types != NULL) free(state->param_types);
        if (state->param_sizes != NULL) free(state->param_sizes);
        if (state->param_int_values != NULL) free(state->param_int_values);
        if (state->param_float_values != NULL) free(state->param_float_values);
        return NULL;
    }
    
    if (state->param_count > 0 && state->param_ready != NULL && state->param_values != NULL) {
        for (int i = 0; i < state->param_count; i++) {
            state->param_ready[i] = 0;
            state->param_values[i] = NULL;
        }
    }
    
    ctx->interface_state_count++;
    
    if (state->return_type == PT_RETURN_TYPE_INTEGER && saved_desc_buf != NULL && strlen(saved_desc_buf) > 0) {
        pt_return_type_t inferred = infer_return_type_from_description(saved_desc_buf);
        if (inferred == PT_RETURN_TYPE_FLOAT || inferred == PT_RETURN_TYPE_DOUBLE) {
            state->return_type = inferred;
        }
        free(saved_desc_buf);
        saved_desc_buf = NULL;
    } else if (saved_desc_buf != NULL) {
        free(saved_desc_buf);
        saved_desc_buf = NULL;
    }
    
    internal_log_write("INFO", "Created interface state for %s.%s with %d parameters, return_type=%d", 
                 plugin_name, interface_name, param_count, state->return_type);
    
    return state;
}

/**
 * @brief 调用目标插件接口（内部实现） / Call target plugin interface (internal implementation) / Ziel-Plugin-Schnittstelle aufrufen (interne Implementierung)
 */
static int call_target_plugin_interface_internal(const pointer_transfer_rule_t* rule, void* ptr, int recursion_depth, const char* call_chain[], size_t call_chain_size) {
    if (recursion_depth > 32) {
        internal_log_write("WARNING", "High recursion depth detected in call_target_plugin_interface (depth=%d)", recursion_depth);
    }
    
    if (rule != NULL && rule->target_plugin != NULL && rule->target_interface != NULL && call_chain_size > 0) {
        char current_call[512];
        snprintf(current_call, sizeof(current_call), "%s.%s", rule->target_plugin, rule->target_interface);
        
        for (size_t i = 0; i < call_chain_size; i++) {
            if (call_chain[i] != NULL && strcmp(call_chain[i], current_call) == 0) {
                internal_log_write("WARNING", "Call cycle detected: %s -> ... -> %s", call_chain[0] != NULL ? call_chain[0] : "unknown", current_call);
                break;
            }
        }
    }
    
    if (rule == NULL || rule->target_plugin == NULL || rule->target_interface == NULL) {
        internal_log_write("ERROR", "Invalid parameters for call_target_plugin_interface");
        return -1;
    }
    
    pointer_transfer_context_t* ctx = get_global_context();
    
    if (rule->target_plugin_path == NULL || strlen(rule->target_plugin_path) == 0) {
        internal_log_write("WARNING", "No plugin path configured for %s", rule->target_plugin);
        return -1;
    }
    
    if (!is_nxpt_loaded(rule->target_plugin)) {
        chain_load_plugin_nxpt(rule->target_plugin, rule->target_plugin_path);
    }
    
    const char* plugin_path = rule->target_plugin_path;
    internal_log_write("INFO", "Using configured plugin path for %s: %s", rule->target_plugin, plugin_path);
    
    void* handle = load_target_plugin(rule->target_plugin, plugin_path);
    if (handle == NULL) {
        internal_log_write("ERROR", "Failed to load target plugin: %s from %s", rule->target_plugin, plugin_path);
        return -1;
    }
    
    void* func_ptr = pt_platform_get_symbol(handle, rule->target_interface);
    
    if (func_ptr == NULL) {
        internal_log_write("ERROR", "Function %s not found in plugin %s", rule->target_interface, rule->target_plugin);
        internal_log_write("ERROR", "Plugin %s does not export required function %s", 
                          rule->target_plugin, rule->target_interface);
        return -1;
    }
    
    target_interface_state_t* state = find_or_create_interface_state(rule->target_plugin, rule->target_interface, handle, func_ptr);
    if (state == NULL) {
        internal_log_write("ERROR", "Failed to create interface state for %s.%s", rule->target_plugin, rule->target_interface);
        return -1;
    }
    
    if (state->param_count > 0) {
        if (ptr == NULL && (rule->target_param_value == NULL || strlen(rule->target_param_value) == 0)) {
            internal_log_write("ERROR", "Invalid NULL pointer for interface %s.%s with %d parameters", 
                          rule->target_plugin, rule->target_interface, state->param_count);
            return -1;
        }
        if (rule->target_param_index < 0 || (!state->is_variadic && rule->target_param_index >= state->param_count)) {
            internal_log_write("ERROR", "Invalid parameter index %d for interface %s.%s (param_count=%d, is_variadic=%d)",
                          rule->target_param_index, rule->target_plugin, rule->target_interface, state->param_count, state->is_variadic);
            return -1;
        }
    }
    
    if (state->param_count > 0 && rule->target_param_index < state->param_count) {
        if (rule->target_param_value != NULL && strlen(rule->target_param_value) > 0) {
            if (!set_parameter_value_from_const_string((struct target_interface_state_s*)state, rule->target_param_index, 
                                                        rule->target_param_value, rule->target_plugin, rule->target_interface)) {
                internal_log_write("WARNING", "Failed to parse constant value for parameter %d of %s.%s", 
                    rule->target_param_index, rule->target_plugin, rule->target_interface);
                state->param_values[rule->target_param_index] = ptr;
                state->param_ready[rule->target_param_index] = 1;
            }
        } else {
            set_parameter_value_from_pointer((struct target_interface_state_s*)state, rule->target_param_index, 
                                            ptr, ctx->stored_size, rule->target_plugin, rule->target_interface);
        }
    }
    
    for (size_t i = 0; i < ctx->rule_count; i++) {
        pointer_transfer_rule_t* const_rule = &ctx->rules[i];
        if (!const_rule->enabled || const_rule->target_param_value == NULL || strlen(const_rule->target_param_value) == 0) {
            continue;
        }
        if (const_rule->target_plugin != NULL && const_rule->target_interface != NULL &&
            strcmp(const_rule->target_plugin, rule->target_plugin) == 0 &&
            strcmp(const_rule->target_interface, rule->target_interface) == 0) {
            if (const_rule->target_param_index >= 0 && const_rule->target_param_index < state->param_count &&
                !state->param_ready[const_rule->target_param_index]) {
                if (!set_parameter_value_from_const_string((struct target_interface_state_s*)state, const_rule->target_param_index, 
                                                            const_rule->target_param_value, const_rule->target_plugin, const_rule->target_interface)) {
                    internal_log_write("WARNING", "Failed to parse constant value for parameter %d of %s.%s", 
                        const_rule->target_param_index, const_rule->target_plugin, const_rule->target_interface);
                }
            }
        }
    }
    
    int required_param_count = state->param_count;
    if (state->is_variadic && state->param_count > 0) {
        required_param_count = state->param_count;
    }
    
    int all_required_ready = 1;
    for (int i = 0; i < required_param_count; i++) {
        if (!state->param_ready[i]) {
            all_required_ready = 0;
            break;
        }
    }
    
    if (!all_required_ready) {
        char unready_params[256] = {0};
        int first = 1;
        size_t pos = 0;
        for (int i = 0; i < required_param_count && pos < sizeof(unready_params) - 1; i++) {
            if (!state->param_ready[i]) {
                if (!first && pos < sizeof(unready_params) - 3) {
                    unready_params[pos++] = ',';
                    unready_params[pos++] = ' ';
                    unready_params[pos] = '\0';
                }
                char param_str[32];
                snprintf(param_str, sizeof(param_str), "%d", i);
                size_t param_len = strlen(param_str);
                if (pos + param_len < sizeof(unready_params) - 1) {
                    memcpy(unready_params + pos, param_str, param_len);
                    pos += param_len;
                    unready_params[pos] = '\0';
                }
                first = 0;
            }
        }
        internal_log_write("WARNING", "Required parameters not all ready for %s.%s (required_count=%d, unready_params=[%s], is_variadic=%d)", 
                     rule->target_plugin, rule->target_interface, required_param_count, unready_params, state->is_variadic);
        return -1;
    }
    
    int actual_param_count = state->param_count;
    if (state->is_variadic) {
        actual_param_count = 0;
        for (int i = 0; i < state->param_count; i++) {
            if (state->param_ready[i]) {
                actual_param_count = i + 1;
            } else {
                break;
            }
        }
        for (size_t i = 0; i < ctx->rule_count; i++) {
            pointer_transfer_rule_t* extra_rule = &ctx->rules[i];
            if (extra_rule->enabled && 
                extra_rule->target_plugin != NULL && 
                extra_rule->target_interface != NULL &&
                strcmp(extra_rule->target_plugin, rule->target_plugin) == 0 &&
                strcmp(extra_rule->target_interface, rule->target_interface) == 0 &&
                extra_rule->target_param_index >= actual_param_count) {
                int all_intermediate_ready = 1;
                for (int j = actual_param_count; j < extra_rule->target_param_index; j++) {
                    if (j < state->param_count && !state->param_ready[j]) {
                        all_intermediate_ready = 0;
                        break;
                    }
                }
                if (all_intermediate_ready) {
                    if (extra_rule->target_param_index == INT_MAX) {
                        internal_log_write("ERROR", "Parameter index overflow detected for %s.%s: target_param_index=%d (INT_MAX), cannot increment", 
                                     rule->target_plugin, rule->target_interface, extra_rule->target_param_index);
                        actual_param_count = INT_MAX;
                    } else {
                        actual_param_count = extra_rule->target_param_index + 1;
                    }
                } else {
                    internal_log_write("WARNING", "Variadic parameter gap detected for %s.%s: parameter %d is set but intermediate parameters are not ready (actual_param_count=%d)", 
                                 rule->target_plugin, rule->target_interface, extra_rule->target_param_index, actual_param_count);
                }
            }
        }
        state->actual_param_count = actual_param_count;
        internal_log_write("INFO", "Variadic interface %s.%s: using %d parameters (min=%d)", 
                     rule->target_plugin, rule->target_interface, actual_param_count, state->param_count);
    }
    
    pt_return_type_t return_type = state->return_type;
    size_t return_size = state->return_size;
    
    if (return_type == PT_RETURN_TYPE_STRUCT_PTR && return_size > 0) {
#ifdef _WIN32
        if (return_size > 8) {
            return_type = PT_RETURN_TYPE_STRUCT_VAL;
        }
#else
        if (return_size > 16) {
            return_type = PT_RETURN_TYPE_STRUCT_VAL;
        }
#endif
    }
    
    if (return_type == PT_RETURN_TYPE_INTEGER) {
        internal_log_write("INFO", "Return type not explicitly configured for %s.%s, return type set to integer", 
                     rule->target_plugin, rule->target_interface);
    }
    
    void* struct_buffer = NULL;
    if (return_type == PT_RETURN_TYPE_STRUCT_VAL && return_size > 0) {
        struct_buffer = malloc(return_size);
        if (struct_buffer == NULL) {
            internal_log_write("ERROR", "Failed to allocate buffer for struct return value (size=%zu)", return_size);
            return -1;
        }
        memset(struct_buffer, 0, return_size);
    }
    
    internal_log_write("INFO", "Calling %s.%s (param_count=%d, return_type=%d, return_size=%zu)", 
                 rule->target_plugin, rule->target_interface, actual_param_count, return_type, return_size);
    
    if (!state->validation_done) {
        pt_param_pack_t* test_pack = pt_create_param_pack(actual_param_count, state->param_types, state->param_values, state->param_sizes);
        if (test_pack != NULL) {
            int32_t validation_result = pt_validate_param_pack(test_pack);
            if (validation_result != 0) {
                internal_log_write("ERROR", "Plugin %s.%s validation failed: invalid param pack structure", 
                                  rule->target_plugin, rule->target_interface);
                pt_free_param_pack(test_pack);
                return -1;
            }
            pt_free_param_pack(test_pack);
            state->validation_done = 1;
            internal_log_write("INFO", "Plugin %s.%s validation passed: compatible with currying API", 
                              rule->target_plugin, rule->target_interface);
        } else {
            internal_log_write("WARNING", "Failed to create test param pack for validation of %s.%s", 
                              rule->target_plugin, rule->target_interface);
        }
    }
    
    state->in_use = 1;
    int64_t result_int = 0;
    double result_float = 0.0;
    int32_t call_result = pt_platform_safe_call(state->func_ptr, actual_param_count, 
                                                 (void*)state->param_types, state->param_values,
                                                 (void*)state->param_sizes,
                                                 return_type, return_size, &result_int, &result_float, struct_buffer);
    if (call_result != 0) {
        state->in_use = 0;
        if (struct_buffer != NULL) {
            free(struct_buffer);
        }
        internal_log_write("ERROR", "Call to %s.%s failed (error=%d)", 
                          rule->target_plugin, rule->target_interface, call_result);
        return -1;
    }
    
    if (return_type == PT_RETURN_TYPE_FLOAT) {
        internal_log_write("INFO", "Called %s.%s, result = %f (float)", 
                     rule->target_plugin, rule->target_interface, (float)result_float);
    } else if (return_type == PT_RETURN_TYPE_DOUBLE) {
        internal_log_write("INFO", "Called %s.%s, result = %lf (double)", 
                     rule->target_plugin, rule->target_interface, result_float);
    } else if (return_type == PT_RETURN_TYPE_STRUCT_PTR) {
        internal_log_write("INFO", "Called %s.%s, result = %p (struct pointer)", 
                     rule->target_plugin, rule->target_interface, (void*)result_int);
    } else if (return_type == PT_RETURN_TYPE_STRUCT_VAL) {
        internal_log_write("INFO", "Called %s.%s, result = %p (struct value, size=%zu)", 
                     rule->target_plugin, rule->target_interface, struct_buffer, return_size);
        if (struct_buffer != NULL) {
            free(struct_buffer);
        }
    } else {
        internal_log_write("INFO", "Called %s.%s, result = %lld (integer/pointer)", 
                     rule->target_plugin, rule->target_interface, (long long)result_int);
    }
    
    for (size_t i = 0; i < ctx->rule_count; i++) {
        pointer_transfer_rule_t* active_rule = &ctx->rules[i];
        if (!active_rule->enabled || active_rule->source_plugin == NULL || active_rule->source_interface == NULL) {
            continue;
        }
        
        if (active_rule->source_param_index == -1 &&
            strcmp(active_rule->source_plugin, rule->target_plugin) == 0 &&
            strcmp(active_rule->source_interface, rule->target_interface) == 0) {
            
            internal_log_write("INFO", "Found active call rule %zu: %s.%s -> %s.%s", 
                         i, active_rule->source_plugin, active_rule->source_interface,
                         active_rule->target_plugin != NULL ? active_rule->target_plugin : "unknown",
                         active_rule->target_interface != NULL ? active_rule->target_interface : "unknown");
            
            void* active_call_value = NULL;
            char* source_plugin_path = active_rule->target_plugin_path != NULL ? active_rule->target_plugin_path : rule->target_plugin_path;
            if (source_plugin_path != NULL && strlen(source_plugin_path) > 0) {
                void* source_handle = load_target_plugin(active_rule->source_plugin, source_plugin_path);
                if (source_handle != NULL) {
                    const char* export_interface_name = NULL;
                    
                    for (size_t j = 0; j < ctx->rule_count; j++) {
                        pointer_transfer_rule_t* export_rule = &ctx->rules[j];
                        if (export_rule->enabled && export_rule->source_plugin != NULL && 
                            strcmp(export_rule->source_plugin, active_rule->source_plugin) == 0 &&
                            export_rule->source_interface != NULL &&
                            export_rule->source_param_index == 0 &&
                            export_rule->target_plugin != NULL && active_rule->target_plugin != NULL &&
                            export_rule->target_interface != NULL && active_rule->target_interface != NULL &&
                            strcmp(export_rule->target_plugin, active_rule->target_plugin) == 0 &&
                            strcmp(export_rule->target_interface, active_rule->target_interface) == 0 &&
                            export_rule->target_param_index == active_rule->target_param_index) {
                            if (strcmp(export_rule->source_interface, active_rule->source_interface) != 0) {
                                export_interface_name = export_rule->source_interface;
                                break;
                            }
                        }
                    }
                    
                    if (export_interface_name != NULL) {
                        void* export_func = pt_platform_get_symbol(source_handle, export_interface_name);
                        if (export_func != NULL) {
                            int int_return_value = 0;
                            
                            target_interface_state_t* target_state = NULL;
                            void* target_handle = load_target_plugin(active_rule->target_plugin, active_rule->target_plugin_path);
                            if (target_handle != NULL) {
                                void* target_func = pt_platform_get_symbol(target_handle, active_rule->target_interface);
                                if (target_func != NULL) {
                                    target_state = find_or_create_interface_state(active_rule->target_plugin, active_rule->target_interface, target_handle, target_func);
                                }
                            }
                            
                            if (target_state != NULL && target_state->param_count > active_rule->target_param_index && 
                                target_state->param_types != NULL) {
                                nxld_param_type_t target_param_type = target_state->param_types[active_rule->target_param_index];
                                
                                if (target_param_type == NXLD_PARAM_TYPE_INT32 || target_param_type == NXLD_PARAM_TYPE_INT64) {
                                    typedef int32_t (NXLD_PLUGIN_CALL *ExportFuncInt)();
                                    ExportFuncInt func = (ExportFuncInt)export_func;
                                    int_return_value = func();
                                    active_call_value = &int_return_value;
                                    internal_log_write("INFO", "Got int value %d from source plugin export interface %s.%s", 
                                                int_return_value, active_rule->source_plugin, export_interface_name);
                                } else {
                                    typedef void* (NXLD_PLUGIN_CALL *ExportFuncPtr)();
                                    ExportFuncPtr func = (ExportFuncPtr)export_func;
                                    active_call_value = func();
                                    internal_log_write("INFO", "Got value from source plugin export interface %s.%s", 
                                                active_rule->source_plugin, export_interface_name);
                                }
                            } else {
                                typedef void* (NXLD_PLUGIN_CALL *ExportFuncPtr)();
                                ExportFuncPtr func = (ExportFuncPtr)export_func;
                                active_call_value = func();
                                internal_log_write("INFO", "Got value from source plugin export interface %s.%s (pointer return type)", 
                                            active_rule->source_plugin, export_interface_name);
                            }
                        } else {
                            internal_log_write("WARNING", "Export interface %s not found in plugin %s", 
                                        export_interface_name, active_rule->source_plugin);
                        }
                    }
                }
            }
            
            void* call_param = NULL;
            if (active_rule->target_param_value != NULL && strlen(active_rule->target_param_value) > 0) {
                call_param = (void*)active_rule->target_param_value;
            } else if (active_call_value != NULL) {
                call_param = active_call_value;
            }
            
            const char* new_call_chain[64];
            size_t new_call_chain_size = 0;
            if (call_chain_size < sizeof(new_call_chain) / sizeof(new_call_chain[0])) {
                for (size_t chain_idx = 0; chain_idx < call_chain_size; chain_idx++) {
                    new_call_chain[chain_idx] = call_chain[chain_idx];
                }
                new_call_chain_size = call_chain_size;
            } else {
                size_t start_idx = call_chain_size - (sizeof(new_call_chain) / sizeof(new_call_chain[0]) - 1);
                for (size_t chain_idx = start_idx; chain_idx < call_chain_size; chain_idx++) {
                    new_call_chain[chain_idx - start_idx] = call_chain[chain_idx];
                }
                new_call_chain_size = call_chain_size - start_idx;
            }
            
            if (active_rule->target_plugin != NULL && active_rule->target_interface != NULL && new_call_chain_size < sizeof(new_call_chain) / sizeof(new_call_chain[0])) {
                static char call_identifiers[64][512];
                static size_t call_id_index = 0;
                size_t current_id_index = call_id_index % (sizeof(call_identifiers) / sizeof(call_identifiers[0]));
                snprintf(call_identifiers[current_id_index], sizeof(call_identifiers[0]), "%s.%s", active_rule->target_plugin, active_rule->target_interface);
                new_call_chain[new_call_chain_size] = call_identifiers[current_id_index];
                new_call_chain_size++;
                call_id_index++;
            }
            
            int active_call_result = call_target_plugin_interface_internal(active_rule, call_param, recursion_depth + 1, new_call_chain, new_call_chain_size);
            if (active_call_result == 0) {
                internal_log_write("INFO", "Successfully executed active call rule %zu", i);
            } else {
                internal_log_write("WARNING", "Failed to execute active call rule %zu (error=%d)", i, active_call_result);
            }
            
            if (active_rule->transfer_mode == TRANSFER_MODE_UNICAST) {
                break;
            }
        }
    }
    
    for (size_t i = 0; i < ctx->rule_count; i++) {
        pointer_transfer_rule_t* export_rule = &ctx->rules[i];
        if (!export_rule->enabled || export_rule->source_plugin == NULL || export_rule->source_interface == NULL) {
            continue;
        }
        
        if (rule != NULL && rule->target_plugin != NULL && 
            export_rule->source_plugin != NULL && 
            strcmp(export_rule->source_plugin, rule->target_plugin) == 0 && 
            export_rule->source_param_index == 0) {
            internal_log_write("INFO", "Export interface rule found: %s.%s -> %s.%s (deferred call mode)", 
                         export_rule->source_plugin, export_rule->source_interface,
                         export_rule->target_plugin != NULL ? export_rule->target_plugin : "unknown",
                         export_rule->target_interface != NULL ? export_rule->target_interface : "unknown");
            break;
        }
    }
    
    if (state != NULL) {
        state->in_use = 0;
        
        if (state->param_count > 0 && state->param_ready != NULL && state->param_values != NULL) {
            for (int i = 0; i < state->param_count; i++) {
                state->param_ready[i] = 0;
                state->param_values[i] = NULL;
            }
        }
    }
    
    return 0;
}

/**
 * @brief 调用目标插件接口 / Call target plugin interface / Ziel-Plugin-Schnittstelle aufrufen
 */
int call_target_plugin_interface(const pointer_transfer_rule_t* rule, void* ptr) {
    const char* initial_call_chain[1] = {NULL};
    size_t initial_call_chain_size = 0;
    
    if (rule != NULL && rule->target_plugin != NULL && rule->target_interface != NULL) {
        static char initial_call_id[512];
        snprintf(initial_call_id, sizeof(initial_call_id), "%s.%s", rule->target_plugin, rule->target_interface);
        initial_call_chain[0] = initial_call_id;
        initial_call_chain_size = 1;
    }
    
    return call_target_plugin_interface_internal(rule, ptr, 0, initial_call_chain, initial_call_chain_size);
}

