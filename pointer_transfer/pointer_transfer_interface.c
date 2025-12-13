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
 * @brief 查找目标接口状态（不创建）/ Find target interface state (without creating) / Ziel-Schnittstellenstatus suchen (ohne Erstellung)
 */
target_interface_state_t* find_interface_state(const char* plugin_name, const char* interface_name) {
    if (plugin_name == NULL || interface_name == NULL) {
        return NULL;
    }
    
    pointer_transfer_context_t* ctx = get_global_context();
    if (ctx->interface_states != NULL) {
        for (size_t i = 0; i < ctx->interface_state_count; i++) {
            target_interface_state_t* state = &ctx->interface_states[i];
            if (state->plugin_name != NULL && state->interface_name != NULL &&
                strcmp(state->plugin_name, plugin_name) == 0 &&
                strcmp(state->interface_name, interface_name) == 0) {
                return state;
            }
        }
    }
    
    return NULL;
}

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
     
     pt_return_type_t inferred_return_type = PT_RETURN_TYPE_INTEGER; /* 默认返回值类型 / Default return type / Standard-Rückgabetyp */
     char* saved_desc_buf = NULL;
     
     for (size_t i = 0; i < interface_count; i++) {
         if (get_interface_info(i, iface_name, iface_name_buf_size, desc_buf, desc_buf_size, NULL, 0) == 0) {
             if (strcmp(iface_name, interface_name) == 0) {
                 target_interface_index = i;
                 /* 找到匹配接口后立即推断返回类型，此时desc_buf已填充 / Infer return type immediately after finding matching interface, desc_buf is already filled / Rückgabetyp sofort nach Auffinden der passenden Schnittstelle ableiten, desc_buf ist bereits gefüllt */
                 inferred_return_type = infer_return_type_from_description(desc_buf);
                 if (desc_buf != NULL && strlen(desc_buf) > 0) {
                     saved_desc_buf = allocate_string(desc_buf);
                 }
                 break;
             }
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
         if (saved_desc_buf != NULL) {
             free(saved_desc_buf);
         }
         return NULL;
     }
     
    int param_count = (int)min_count;
    int is_variadic = 0;
    if (param_count_type == NXLD_PARAM_COUNT_FIXED && max_count > 0) {
        param_count = max_count;
    } else if (param_count_type == NXLD_PARAM_COUNT_VARIABLE) {
        param_count = max_count > 0 ? max_count : min_count;
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
    
    if (ctx->interface_states == NULL) {
        internal_log_write("ERROR", "Interface states array is NULL after capacity expansion");
        return NULL;
    }
    
    target_interface_state_t* state = &ctx->interface_states[ctx->interface_state_count];
     state->plugin_name = allocate_string(plugin_name);
     state->interface_name = allocate_string(interface_name);
     state->handle = handle;
     state->func_ptr = func_ptr;
    state->param_count = param_count;
    state->is_variadic = is_variadic;
    state->min_param_count = is_variadic ? (int)min_count : param_count;
    state->actual_param_count = param_count;
    state->return_type = inferred_return_type;
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
        if (param_name == NULL) {
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
        
        size_t type_name_buf_size = 512;
        char* type_name = (char*)malloc(type_name_buf_size);
        if (type_name == NULL) {
            free(param_name);
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
        
        if (state->param_types != NULL) {
            for (int i = 0; i < param_count; i++) {
                nxld_param_type_t param_type;
                if (get_param_info(target_interface_index, i, param_name, param_name_buf_size,
                                  &param_type, type_name, type_name_buf_size) == 0) {
                    state->param_types[i] = param_type;
                }
            }
        }
        
        free(param_name);
        free(type_name);
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
     
     if (state->return_type == PT_RETURN_TYPE_STRUCT_PTR) {
         internal_log_write("INFO", "Struct return type detected for %s.%s from description, return type set to pointer", plugin_name, interface_name);
     }
     
     if (saved_desc_buf != NULL) {
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
 static int call_target_plugin_interface_internal(const pointer_transfer_rule_t* rule, void* ptr, int recursion_depth, const char* call_chain[], size_t call_chain_size, int skip_param_cleanup) {
     if (recursion_depth > 32) {
         internal_log_write("WARNING", "High recursion depth detected in call_target_plugin_interface (depth=%d)", recursion_depth);
     }
     
     if (rule == NULL || rule->target_plugin == NULL || rule->target_interface == NULL) {
         internal_log_write("ERROR", "Invalid parameters for call_target_plugin_interface");
         return -1;
     }
     
     char current_call[512];
     snprintf(current_call, sizeof(current_call), "%s.%s", rule->target_plugin, rule->target_interface);
     
     if (call_chain_size > 0) {
         for (size_t i = 0; i < call_chain_size; i++) {
             if (call_chain[i] != NULL && strcmp(call_chain[i], current_call) == 0) {
                 internal_log_write("WARNING", "Call cycle detected: %s -> ... -> %s", call_chain[0] != NULL ? call_chain[0] : "unknown", current_call);
                 return -1;
             }
         }
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
        if (state->param_values == NULL || state->param_ready == NULL) {
            internal_log_write("ERROR", "Parameter arrays are NULL for interface %s.%s", 
                           rule->target_plugin, rule->target_interface);
            return -1;
        }
        if (rule->target_param_value != NULL && strlen(rule->target_param_value) > 0) {
            if (!set_parameter_value_from_const_string((struct target_interface_state_s*)state, rule->target_param_index, 
                                                        rule->target_param_value, rule->target_plugin, rule->target_interface)) {
                internal_log_write("WARNING", "Failed to parse constant value for parameter %d of %s.%s, falling back to pointer", 
                    rule->target_param_index, rule->target_plugin, rule->target_interface);
                /* 回退到指针设置，确保所有字段一致 / Fallback to pointer setting, ensure all fields are consistent / Fallback auf Zeigereinstellung, sicherstellen, dass alle Felder konsistent sind */
                state->param_values[rule->target_param_index] = ptr;
                state->param_ready[rule->target_param_index] = 1;
                if (state->param_sizes != NULL) {
                    state->param_sizes[rule->target_param_index] = ctx->stored_size > 0 ? ctx->stored_size : sizeof(void*);
                }
            }
        } else {
            set_parameter_value_from_pointer((struct target_interface_state_s*)state, rule->target_param_index, 
                                            ptr, ctx->stored_size, rule->target_plugin, rule->target_interface);
        }
    }
     
    if (ctx->rules != NULL) {
        for (size_t i = 0; i < ctx->rule_count; i++) {
            pointer_transfer_rule_t* const_rule = &ctx->rules[i];
            if (!const_rule->enabled || const_rule->target_param_value == NULL || strlen(const_rule->target_param_value) == 0) {
                continue;
            }
            if (const_rule->target_plugin != NULL && const_rule->target_interface != NULL &&
                strcmp(const_rule->target_plugin, rule->target_plugin) == 0 &&
                strcmp(const_rule->target_interface, rule->target_interface) == 0) {
                if (const_rule->target_param_index >= 0 && const_rule->target_param_index < state->param_count &&
                    state->param_ready != NULL && !state->param_ready[const_rule->target_param_index]) {
                    if (!set_parameter_value_from_const_string((struct target_interface_state_s*)state, const_rule->target_param_index, 
                                                                const_rule->target_param_value, const_rule->target_plugin, const_rule->target_interface)) {
                        internal_log_write("WARNING", "Failed to parse constant value for parameter %d of %s.%s", 
                            const_rule->target_param_index, const_rule->target_plugin, const_rule->target_interface);
                    }
                }
            }
        }
    }
     
    int required_param_count = state->param_count;
    if (state->is_variadic) {
        /* 可变参数接口：首先计算实际就绪的参数数量（连续） / Variadic interface: calculate actual ready parameter count (consecutive) / Variabler Parameter-Interface: Tatsächliche bereite Parameteranzahl berechnen (aufeinanderfolgend) */
        int actual_ready_count = 0;
        if (state->param_ready != NULL) {
            for (int i = 0; i < state->param_count; i++) {
                if (state->param_ready[i]) {
                    actual_ready_count = i + 1;
                } else {
                    break;
                }
            }
        }
        /* 检查是否满足最小参数要求 / Check if minimum parameter requirement is met / Prüfen, ob Mindestparameteranforderung erfüllt ist */
        if (actual_ready_count < state->min_param_count) {
            char unready_params[256] = {0};
            int first = 1;
            size_t pos = 0;
            for (int i = actual_ready_count; i < state->min_param_count && pos < sizeof(unready_params) - 1; i++) {
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
            internal_log_write("WARNING", "Required parameters not all ready for %s.%s (min_required=%d, actual_ready=%d, unready_params=[%s], is_variadic=1)", 
                         rule->target_plugin, rule->target_interface, state->min_param_count, actual_ready_count, unready_params);
            return -1;
        }
        /* 可变参数接口满足要求，继续执行 / Variadic interface requirement met, continue / Variabler Parameter-Interface-Anforderung erfüllt, fortfahren */
    } else {
        /* 固定参数接口：检查所有参数是否就绪 / Fixed parameter interface: check if all parameters are ready / Fester Parameter-Interface: Prüfen, ob alle Parameter bereit sind */
        if (state->param_ready == NULL) {
            internal_log_write("ERROR", "param_ready array is NULL for interface %s.%s", 
                           rule->target_plugin, rule->target_interface);
            return -1;
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
        if (ctx->rules != NULL && state->param_ready != NULL) {
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
                        if (extra_rule->target_param_index == INT_MAX || extra_rule->target_param_index < 0) {
                            internal_log_write("ERROR", "Invalid parameter index detected for %s.%s: target_param_index=%d, skipping this rule", 
                                         rule->target_plugin, rule->target_interface, extra_rule->target_param_index);
                            /* 跳过此规则，不更新actual_param_count / Skip this rule, don't update actual_param_count / Diese Regel überspringen, actual_param_count nicht aktualisieren */
                        } else if (extra_rule->target_param_index >= state->param_count) {
                            internal_log_write("WARNING", "Parameter index %d exceeds param_count %d for %s.%s, limiting to param_count", 
                                         extra_rule->target_param_index, state->param_count, rule->target_plugin, rule->target_interface);
                            actual_param_count = state->param_count;
                        } else {
                            actual_param_count = extra_rule->target_param_index + 1;
                        }
                    } else {
                        internal_log_write("WARNING", "Variadic parameter gap detected for %s.%s: parameter %d is set but intermediate parameters are not ready (actual_param_count=%d)", 
                                     rule->target_plugin, rule->target_interface, extra_rule->target_param_index, actual_param_count);
                    }
                }
            }
        }
        state->actual_param_count = actual_param_count;
        /* 再次验证是否满足最小参数要求 / Verify again if minimum parameter requirement is met / Erneut prüfen, ob Mindestparameteranforderung erfüllt ist */
        if (actual_param_count < state->min_param_count) {
            internal_log_write("WARNING", "Variadic interface %s.%s: actual_param_count=%d is less than min_param_count=%d, this may cause call failure", 
                         rule->target_plugin, rule->target_interface, actual_param_count, state->min_param_count);
        }
        internal_log_write("INFO", "Variadic interface %s.%s: using %d parameters (min_required=%d, max_available=%d)", 
                     rule->target_plugin, rule->target_interface, actual_param_count, state->min_param_count, state->param_count);
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
     
    /* 最终验证：可变参数接口必须满足最小参数要求 / Final validation: variadic interface must meet minimum parameter requirement / Endgültige Validierung: Variabler Parameter-Interface muss Mindestparameteranforderung erfüllen */
    if (state->is_variadic && actual_param_count < state->min_param_count) {
        internal_log_write("ERROR", "Cannot call %s.%s: actual_param_count=%d is less than min_param_count=%d", 
                     rule->target_plugin, rule->target_interface, actual_param_count, state->min_param_count);
        if (struct_buffer != NULL) {
            free(struct_buffer);
        }
        return -1;
    }
    
    internal_log_write("INFO", "Calling %s.%s (param_count=%d, return_type=%d, return_size=%zu)", 
                 rule->target_plugin, rule->target_interface, actual_param_count, return_type, return_size);
    
    if (!state->validation_done) {
         /* 使用pt_validate_plugin_function进行完整验证（包括时间戳检查和.nxpv文件生成）/ Use pt_validate_plugin_function for full validation (including timestamp check and .nxpv file generation) / pt_validate_plugin_function für vollständige Validierung verwenden (einschließlich Zeitstempelprüfung und .nxpv-Dateigenerierung) */
         int32_t validation_result = pt_validate_plugin_function(state->func_ptr, rule->target_plugin_path, 
                                                                 rule->target_interface, actual_param_count, return_type);
         if (validation_result != 0) {
             internal_log_write("ERROR", "Plugin %s.%s validation failed: function validation returned error", 
                               rule->target_plugin, rule->target_interface);
             if (struct_buffer != NULL) {
                 free(struct_buffer);
                 struct_buffer = NULL;
             }
             return -1;
         }
         state->validation_done = 1;
         internal_log_write("INFO", "Plugin %s.%s validation passed: compatible with currying API", 
                           rule->target_plugin, rule->target_interface);
     }
     
     if (state->param_types == NULL || state->param_values == NULL) {
         internal_log_write("ERROR", "Parameter arrays are NULL for interface %s.%s", 
                        rule->target_plugin, rule->target_interface);
         if (struct_buffer != NULL) {
             free(struct_buffer);
         }
         return -1;
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
    } else {
        internal_log_write("INFO", "Called %s.%s, result = %lld (integer/pointer)", 
                      rule->target_plugin, rule->target_interface, (long long)result_int);
    }
    
    /* 匹配 SourceParamIndex=-1 的传递规则 / Match transfer rules with SourceParamIndex=-1 / Übertragungsregeln mit SourceParamIndex=-1 abgleichen */
    /* 首先收集所有匹配的规则，然后按SetGroup分组处理 / First collect all matching rules, then process by SetGroup / Zuerst alle passenden Regeln sammeln, dann nach SetGroup gruppiert verarbeiten */
    if (ctx->rules != NULL) {
        /* 收集所有匹配的规则索引 / Collect all matching rule indices / Alle passenden Regelindizes sammeln */
        size_t matched_rules[256];
        size_t matched_count = 0;
        
        for (size_t i = 0; i < ctx->rule_count && matched_count < sizeof(matched_rules) / sizeof(matched_rules[0]); i++) {
            pointer_transfer_rule_t* active_rule = &ctx->rules[i];
            if (!active_rule->enabled || active_rule->source_plugin == NULL || active_rule->source_interface == NULL) {
                continue;
            }
            
            if (active_rule->source_param_index == -1 &&
                strcmp(active_rule->source_plugin, rule->target_plugin) == 0 &&
                strcmp(active_rule->source_interface, rule->target_interface) == 0) {
                matched_rules[matched_count++] = i;
            }
        }
        
        /* 按SetGroup分组处理规则 / Process rules grouped by SetGroup / Regeln nach SetGroup gruppiert verarbeiten */
        int processed[256] = {0}; /* 标记已处理的规则 / Mark processed rules / Verarbeitete Regeln markieren */
        
        for (size_t match_idx = 0; match_idx < matched_count; match_idx++) {
            if (processed[match_idx]) {
                continue;
            }
            
            size_t i = matched_rules[match_idx];
            pointer_transfer_rule_t* active_rule = &ctx->rules[i];
            
            /* 检查是否有SetGroup / Check if has SetGroup / Prüfen, ob SetGroup vorhanden */
            if (active_rule->set_group != NULL && strlen(active_rule->set_group) > 0 &&
                active_rule->target_plugin != NULL && active_rule->target_interface != NULL) {
                
                /* 收集同一SetGroup中的所有规则 / Collect all rules in same SetGroup / Alle Regeln in derselben SetGroup sammeln */
                size_t group_rules[64];
                size_t group_count = 0;
                
                for (size_t j = 0; j < matched_count && group_count < sizeof(group_rules) / sizeof(group_rules[0]); j++) {
                    if (processed[j]) {
                        continue;
                    }
                    size_t rule_idx = matched_rules[j];
                    pointer_transfer_rule_t* group_rule = &ctx->rules[rule_idx];
                    if (group_rule->set_group != NULL && 
                        strcmp(group_rule->set_group, active_rule->set_group) == 0 &&
                        group_rule->target_plugin != NULL && group_rule->target_interface != NULL &&
                        strcmp(group_rule->target_plugin, active_rule->target_plugin) == 0 &&
                        strcmp(group_rule->target_interface, active_rule->target_interface) == 0) {
                        group_rules[group_count++] = rule_idx;
                        processed[j] = 1;
                    }
                }
                
                /* 按target_param_index排序 / Sort by target_param_index / Nach target_param_index sortieren */
                for (size_t a = 0; a < group_count; a++) {
                    for (size_t b = a + 1; b < group_count; b++) {
                        pointer_transfer_rule_t* rule_a = &ctx->rules[group_rules[a]];
                        pointer_transfer_rule_t* rule_b = &ctx->rules[group_rules[b]];
                        if (rule_a->target_param_index > rule_b->target_param_index) {
                            size_t temp = group_rules[a];
                            group_rules[a] = group_rules[b];
                            group_rules[b] = temp;
                        }
                    }
                }
                
                /* 按顺序执行SetGroup中的规则，每次重新调用源接口 / Execute rules in SetGroup sequentially, re-call source interface each time / Regeln in SetGroup sequenziell ausführen, Quellschnittstelle jedes Mal neu aufrufen */
                for (size_t group_idx = 0; group_idx < group_count; group_idx++) {
                    size_t rule_idx = group_rules[group_idx];
                    pointer_transfer_rule_t* group_rule = &ctx->rules[rule_idx];
                    
                    /* 为SetGroup中的每个规则重新调用源接口以获取新的返回值 / Re-call source interface for each rule in SetGroup to get new return value / Quellschnittstelle für jede Regel in SetGroup neu aufrufen, um neuen Rückgabewert zu erhalten */
                    int64_t group_result_int = result_int;
                    double group_result_float = result_float;
                    pt_return_type_t group_return_type = return_type;
                    size_t group_return_size = return_size;
                    void* group_struct_buffer = NULL;
                    
                    if (group_rule->source_plugin != NULL && group_rule->source_interface != NULL) {
                        target_interface_state_t* source_state = find_interface_state(group_rule->source_plugin, group_rule->source_interface);
                        if (source_state != NULL && source_state->func_ptr != NULL) {
                            /* 重新调用源接口获取新的返回值 / Re-call source interface to get new return value / Quellschnittstelle neu aufrufen, um neuen Rückgabewert zu erhalten */
                            int source_actual_param_count = source_state->param_count;
                            if (source_state->is_variadic) {
                                source_actual_param_count = 0;
                                if (source_state->param_ready != NULL) {
                                    for (int i = 0; i < source_state->param_count; i++) {
                                        if (source_state->param_ready[i]) {
                                            source_actual_param_count = i + 1;
                                        } else {
                                            break;
                                        }
                                    }
                                }
                            }
                            
                            pt_return_type_t source_return_type = source_state->return_type;
                            size_t source_return_size = source_state->return_size;
                            
                            if (source_return_type == PT_RETURN_TYPE_STRUCT_PTR && source_return_size > 0) {
#ifdef _WIN32
                                if (source_return_size > 8) {
                                    source_return_type = PT_RETURN_TYPE_STRUCT_VAL;
                                }
#else
                                if (source_return_size > 16) {
                                    source_return_type = PT_RETURN_TYPE_STRUCT_VAL;
                                }
#endif
                            }
                            
                            if (source_return_type == PT_RETURN_TYPE_STRUCT_VAL && source_return_size > 0) {
                                group_struct_buffer = malloc(source_return_size);
                                if (group_struct_buffer != NULL) {
                                    memset(group_struct_buffer, 0, source_return_size);
                                }
                            }
                            
                            if (source_state->param_types != NULL && source_state->param_values != NULL) {
                                int64_t temp_result_int = 0;
                                double temp_result_float = 0.0;
                                int32_t source_call_result = pt_platform_safe_call(source_state->func_ptr, source_actual_param_count,
                                                                                  (void*)source_state->param_types, source_state->param_values,
                                                                                  (void*)source_state->param_sizes,
                                                                                  source_return_type, source_return_size, &temp_result_int, &temp_result_float, group_struct_buffer);
                                if (source_call_result == 0) {
                                    group_result_int = temp_result_int;
                                    group_result_float = temp_result_float;
                                    group_return_type = source_return_type;
                                    group_return_size = source_return_size;
                                    internal_log_write("INFO", "Re-called source interface %s.%s for SetGroup rule %zu, new result = %lld", 
                                                  group_rule->source_plugin, group_rule->source_interface, rule_idx, (long long)group_result_int);
                                } else {
                                    internal_log_write("WARNING", "Failed to re-call source interface %s.%s for SetGroup rule %zu (error=%d), using original return value", 
                                                  group_rule->source_plugin, group_rule->source_interface, rule_idx, source_call_result);
                                    if (group_struct_buffer != NULL) {
                                        free(group_struct_buffer);
                                        group_struct_buffer = NULL;
                                    }
                                }
                            }
                        }
                    }
                    
                    /* 设置组检查：如果规则属于设置组，检查是否有其他规则会设置后续参数 / Set group check: if rule belongs to set group, check if other rules will set subsequent parameters / Set-Gruppenprüfung: Wenn Regel zur Set-Gruppe gehört, prüfen, ob andere Regeln nachfolgende Parameter setzen */
                    int should_check_group = 0;
                    int is_min_param_index = 1;  /* 当前规则是否是参数索引最小的规则 / Is current rule the one with minimum parameter index / Ist aktuelle Regel die mit minimalem Parameterindex */
                    if (group_rule->set_group != NULL && 
                        strlen(group_rule->set_group) > 0 &&
                        group_rule->target_plugin != NULL && group_rule->target_interface != NULL &&
                        group_rule->target_param_index >= 0) {
                        internal_log_write("INFO", "Set group check: rule %zu has set_group=%s, target=%s.%s[%d]", 
                            rule_idx, group_rule->set_group, group_rule->target_plugin, group_rule->target_interface, group_rule->target_param_index);
                        /* 检查同一组内是否有其他规则会设置后续参数，以及是否有规则设置更小的参数索引 / Check if other rules in same group will set subsequent parameters, and if any rule sets smaller parameter index / Prüfen, ob andere Regeln in derselben Gruppe nachfolgende Parameter setzen, und ob eine Regel kleineren Parameterindex setzt */
                        for (size_t j = 0; j < ctx->rule_count; j++) {
                            if (j == rule_idx) continue;
                            pointer_transfer_rule_t* check_rule = &ctx->rules[j];
                            if (check_rule->enabled && check_rule->set_group != NULL &&
                                strcmp(check_rule->set_group, group_rule->set_group) == 0 &&
                                check_rule->source_param_index == -1 &&
                                check_rule->source_plugin != NULL && check_rule->source_interface != NULL &&
                                group_rule->source_plugin != NULL && group_rule->source_interface != NULL &&
                                strcmp(check_rule->source_plugin, group_rule->source_plugin) == 0 &&
                                strcmp(check_rule->source_interface, group_rule->source_interface) == 0 &&
                                check_rule->target_plugin != NULL && check_rule->target_interface != NULL &&
                                strcmp(check_rule->target_plugin, group_rule->target_plugin) == 0 &&
                                strcmp(check_rule->target_interface, group_rule->target_interface) == 0) {
                                if (check_rule->target_param_index > group_rule->target_param_index) {
                                    /* 找到同一组内会设置后续参数的规则，需要检查参数就绪状态 / Found rule in same group that will set subsequent parameter, need to check parameter readiness / Regel in derselben Gruppe gefunden, die nachfolgende Parameter setzt, Parameterbereitschaft prüfen */
                                    should_check_group = 1;
                                    internal_log_write("INFO", "Set group check: rule %zu belongs to set group %s, found subsequent rule %zu that will set parameter %d", 
                                        rule_idx, group_rule->set_group, j, check_rule->target_param_index);
                                }
                                if (check_rule->target_param_index < group_rule->target_param_index) {
                                    /* 找到同一组内会设置更小参数索引的规则，当前规则不是最小索引 / Found rule in same group that will set smaller parameter index, current rule is not minimum / Regel in derselben Gruppe gefunden, die kleineren Parameterindex setzt, aktuelle Regel ist nicht Minimum */
                                    is_min_param_index = 0;
                                }
                            }
                        }
                    } else if (group_rule->set_group == NULL) {
                        internal_log_write("INFO", "Set group check: rule %zu has no set_group", rule_idx);
                    }
                    
                    /* 提前检查参数就绪状态，避免不必要的调用尝试 / Pre-check parameter readiness to avoid unnecessary call attempts / Vorabprüfung der Parameterbereitschaft, um unnötige Aufrufversuche zu vermeiden */
                    if (group_rule->target_plugin != NULL && group_rule->target_interface != NULL && group_rule->target_param_index >= 0) {
                        target_interface_state_t* target_state = find_interface_state(group_rule->target_plugin, group_rule->target_interface);
                        if (target_state != NULL && target_state->param_ready != NULL) {
                            /* 检查目标参数索引之前的所有参数是否就绪 / Check if all parameters before target parameter index are ready / Prüfen, ob alle Parameter vor dem Zielparameterindex bereit sind */
                            int can_apply = 1;
                            int check_limit = group_rule->target_param_index;
                            if (target_state->is_variadic) {
                                /* 可变参数接口：检查到目标参数索引之前的所有参数 / Variadic interface: check all parameters before target parameter index / Variabler Parameter-Interface: Alle Parameter vor Zielparameterindex prüfen */
                                for (int j = 0; j < check_limit && j < target_state->param_count; j++) {
                                    if (!target_state->param_ready[j]) {
                                        can_apply = 0;
                                        break;
                                    }
                                }
                                if (can_apply && check_limit < target_state->min_param_count) {
                                    int ready_count_after_set = check_limit + 1;
                                    if (ready_count_after_set < target_state->min_param_count) {
                                        can_apply = 0;
                                    }
                                }
                            } else {
                                /* 固定参数接口：检查目标参数索引之前的所有参数是否就绪 / Fixed parameter interface: check if all parameters before target parameter index are ready / Fester Parameter-Interface: Prüfen, ob alle Parameter vor dem Zielparameterindex bereit sind */
                                for (int j = 0; j < check_limit && j < target_state->param_count; j++) {
                                    if (!target_state->param_ready[j]) {
                                        can_apply = 0;
                                        break;
                                    }
                                }
                                /* 固定参数接口：目标参数索引必须小于参数总数 / Fixed parameter interface: target parameter index must be less than total parameter count / Fester Parameter-Interface: Zielparameterindex muss kleiner als Gesamtparameteranzahl sein */
                                if (can_apply && group_rule->target_param_index >= target_state->param_count) {
                                    can_apply = 0;
                                }
                            }
                            
                            if (!can_apply) {
                                /* 参数未就绪，跳过此规则 / Parameters not ready, skip this rule / Parameter nicht bereit, diese Regel überspringen */
                                continue;
                            }
                        } else if (should_check_group && !is_min_param_index) {
                            /* 设置组检查：接口状态不存在，但同一组内有规则会设置后续参数，且当前规则不是参数索引最小的规则，跳过当前规则等待接口状态创建 / Set group check: interface state does not exist, but rules in same group will set subsequent parameters, and current rule is not the one with minimum parameter index, skip current rule and wait for interface state creation / Set-Gruppenprüfung: Schnittstellenstatus existiert nicht, aber Regeln in derselben Gruppe setzen nachfolgende Parameter, und aktuelle Regel ist nicht die mit minimalem Parameterindex, aktuelle Regel überspringen und auf Schnittstellenstatus-Erstellung warten */
                            internal_log_write("INFO", "Set group check: skipping rule %zu (target state not exists, subsequent rule will set parameter, current rule is not minimum param index)", rule_idx);
                            continue;
                        }
                    }
                    
                    internal_log_write("INFO", "Found active call rule %zu: %s.%s -> %s.%s", 
                                  rule_idx, group_rule->source_plugin, group_rule->source_interface,
                                  group_rule->target_plugin != NULL ? group_rule->target_plugin : "unknown",
                                  group_rule->target_interface != NULL ? group_rule->target_interface : "unknown");
                    
                    void* call_param = NULL;
                    if (group_rule->target_param_value != NULL && strlen(group_rule->target_param_value) > 0) {
                        call_param = (void*)group_rule->target_param_value;
                    } else {
                        /* 根据返回值类型选择传递参数 / Select transfer parameter by return type / Übertragungsparameter nach Rückgabetyp auswählen */
                        if (group_return_type == PT_RETURN_TYPE_FLOAT || group_return_type == PT_RETURN_TYPE_DOUBLE) {
                            call_param = &group_result_float;
                            internal_log_write("INFO", "Using float return value %lf for transfer", group_result_float);
                        } else if (group_return_type == PT_RETURN_TYPE_STRUCT_VAL && group_struct_buffer != NULL) {
                            /* PT_RETURN_TYPE_STRUCT_VAL: 传递 struct_buffer / PT_RETURN_TYPE_STRUCT_VAL: pass struct_buffer / PT_RETURN_TYPE_STRUCT_VAL: struct_buffer übergeben */
                            call_param = group_struct_buffer;
                            internal_log_write("INFO", "Using struct return value (size=%zu) for transfer", group_return_size);
                        } else if (group_return_type == PT_RETURN_TYPE_STRUCT_PTR) {
                            /* PT_RETURN_TYPE_STRUCT_PTR: 传递指针值（字符串指针等）/ PT_RETURN_TYPE_STRUCT_PTR: pass pointer value (string pointer etc.) / PT_RETURN_TYPE_STRUCT_PTR: Zeigerwert übergeben (Zeichenfolgenzeiger usw.) */
                            call_param = (void*)(intptr_t)group_result_int;
                            ctx->stored_size = sizeof(void*);
                            ctx->stored_type = NXLD_PARAM_TYPE_STRING;
                            internal_log_write("INFO", "Using pointer return value %p for transfer", (void*)(intptr_t)group_result_int);
                        } else {
                            /* 其他类型: 传递 result_int / Other types: pass result_int / Andere Typen: result_int übergeben */
                            call_param = &group_result_int;
                            ctx->stored_size = sizeof(int64_t);
                            ctx->stored_type = NXLD_PARAM_TYPE_INT64;
                            internal_log_write("INFO", "Using integer/pointer return value %lld for transfer", (long long)group_result_int);
                        }
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
                    
                    if (rule->target_plugin != NULL && rule->target_interface != NULL && new_call_chain_size < sizeof(new_call_chain) / sizeof(new_call_chain[0])) {
                        static char current_call_identifiers[64][512];
                        static size_t current_call_id_index = 0;
                        size_t current_id_index = current_call_id_index % (sizeof(current_call_identifiers) / sizeof(current_call_identifiers[0]));
                        snprintf(current_call_identifiers[current_id_index], sizeof(current_call_identifiers[0]), "%s.%s", rule->target_plugin, rule->target_interface);
                        new_call_chain[new_call_chain_size] = current_call_identifiers[current_id_index];
                        new_call_chain_size++;
                        current_call_id_index++;
                    }
                
                    /* SetGroup处理中，跳过参数清理，直到所有规则处理完毕 / In SetGroup processing, skip parameter cleanup until all rules are processed / In SetGroup-Verarbeitung Parameterbereinigung überspringen, bis alle Regeln verarbeitet sind */
                    int active_call_result = call_target_plugin_interface_internal(group_rule, call_param, recursion_depth + 1, new_call_chain, new_call_chain_size, 1);
                    if (active_call_result == 0) {
                        internal_log_write("INFO", "Successfully executed active call rule %zu", rule_idx);
                    } else {
                        internal_log_write("WARNING", "Failed to execute active call rule %zu (error=%d)", rule_idx, active_call_result);
                    }
                    
                    /* 释放SetGroup规则使用的struct_buffer / Free struct_buffer used by SetGroup rule / Von SetGroup-Regel verwendeten struct_buffer freigeben */
                    if (group_struct_buffer != NULL) {
                        free(group_struct_buffer);
                        group_struct_buffer = NULL;
                    }
                    
                    /* UNICAST模式：对于返回值传递，允许传递给同一个接口的不同参数索引，只有当找到完全相同的目标位置时才停止 / UNICAST mode: for return value transfer, allow passing to different parameter indices of the same interface, only stop when exact duplicate target is found / UNICAST-Modus: Für Rückgabewertübertragung, Übergabe an verschiedene Parameterindizes derselben Schnittstelle zulassen, nur stoppen, wenn exaktes doppeltes Ziel gefunden wird */
                    /* 注意：对于SetGroup中的规则，即使目标位置相同，也应该继续执行，因为每次都会重新调用源接口获取新的返回值 / Note: For rules in SetGroup, even if target location is same, should continue execution, because source interface is re-called each time to get new return value / Hinweis: Für Regeln in SetGroup, auch wenn Zielposition gleich ist, sollte Ausführung fortgesetzt werden, da Quellschnittstelle jedes Mal neu aufgerufen wird, um neuen Rückgabewert zu erhalten */
                    if (group_rule->transfer_mode == TRANSFER_MODE_UNICAST) {
                        /* 检查SetGroup中是否还有其他规则要处理 / Check if there are more rules in SetGroup to process / Prüfen, ob noch weitere Regeln in SetGroup zu verarbeiten sind */
                        int has_more_in_group = 0;
                        for (size_t j = group_idx + 1; j < group_count; j++) {
                            size_t next_rule_idx = group_rules[j];
                            pointer_transfer_rule_t* next_group_rule = &ctx->rules[next_rule_idx];
                            if (next_group_rule->enabled && 
                                next_group_rule->target_plugin != NULL && next_group_rule->target_interface != NULL &&
                                group_rule->target_plugin != NULL && group_rule->target_interface != NULL &&
                                strcmp(next_group_rule->target_plugin, group_rule->target_plugin) == 0 &&
                                strcmp(next_group_rule->target_interface, group_rule->target_interface) == 0 &&
                                next_group_rule->target_param_index == group_rule->target_param_index) {
                                /* SetGroup中还有相同目标位置的规则，继续执行 / More rules with same target location in SetGroup, continue execution / Weitere Regeln mit gleicher Zielposition in SetGroup, Ausführung fortsetzen */
                                has_more_in_group = 1;
                                break;
                            }
                        }
                        /* 如果SetGroup中还有规则要处理，不停止 / If there are more rules in SetGroup to process, don't stop / Wenn noch weitere Regeln in SetGroup zu verarbeiten sind, nicht stoppen */
                        if (has_more_in_group) {
                            continue;
                        }
                        
                        /* 检查是否有其他规则（不在SetGroup中）匹配完全相同的目标位置 / Check if other rules (not in SetGroup) match the exact same target location / Prüfen, ob andere Regeln (nicht in SetGroup) exakt dieselbe Zielposition abgleichen */
                        int has_exact_duplicate = 0;
                        for (size_t j = rule_idx + 1; j < ctx->rule_count; j++) {
                            pointer_transfer_rule_t* next_rule = &ctx->rules[j];
                            if (!next_rule->enabled || next_rule->source_plugin == NULL || next_rule->source_interface == NULL) {
                                continue;
                            }
                            /* 跳过SetGroup中的其他规则 / Skip other rules in SetGroup / Andere Regeln in SetGroup überspringen */
                            if (next_rule->set_group != NULL && strlen(next_rule->set_group) > 0 &&
                                group_rule->set_group != NULL && strcmp(next_rule->set_group, group_rule->set_group) == 0) {
                                continue;
                            }
                            if (next_rule->source_param_index == -1 &&
                                strcmp(next_rule->source_plugin, rule->target_plugin) == 0 &&
                                strcmp(next_rule->source_interface, rule->target_interface) == 0 &&
                                next_rule->target_plugin != NULL && next_rule->target_interface != NULL &&
                                group_rule->target_plugin != NULL && group_rule->target_interface != NULL &&
                                strcmp(next_rule->target_plugin, group_rule->target_plugin) == 0 &&
                                strcmp(next_rule->target_interface, group_rule->target_interface) == 0 &&
                                next_rule->target_param_index == group_rule->target_param_index) {
                                has_exact_duplicate = 1;
                                break;
                            }
                        }
                        /* 只有在找到完全相同的目标位置时才停止，允许传递给同一接口的不同参数 / Only stop when exact duplicate target is found, allow passing to different parameters of the same interface / Nur stoppen, wenn exaktes doppeltes Ziel gefunden wird, Übergabe an verschiedene Parameter derselben Schnittstelle zulassen */
                        if (has_exact_duplicate) {
                            break;
                        }
                    }
                }
                
                /* SetGroup处理完成后，清理目标接口的参数状态 / After SetGroup processing, cleanup parameter state of target interface / Nach SetGroup-Verarbeitung Parameterstatus der Zielschnittstelle bereinigen */
                if (group_count > 0 && active_rule->target_plugin != NULL && active_rule->target_interface != NULL) {
                    target_interface_state_t* target_state = find_interface_state(active_rule->target_plugin, active_rule->target_interface);
                    if (target_state != NULL && target_state->param_count > 0 && target_state->param_ready != NULL && target_state->param_values != NULL) {
                        for (int i = 0; i < target_state->param_count; i++) {
                            target_state->param_ready[i] = 0;
                            target_state->param_values[i] = NULL;
                        }
                        internal_log_write("INFO", "Cleaned up parameter state for SetGroup target interface %s.%s", 
                                      active_rule->target_plugin, active_rule->target_interface);
                    }
                }
            } else {
                /* 处理没有SetGroup的规则 / Process rules without SetGroup / Regeln ohne SetGroup verarbeiten */
                void* call_param = NULL;
                if (active_rule->target_param_value != NULL && strlen(active_rule->target_param_value) > 0) {
                    call_param = (void*)active_rule->target_param_value;
                } else {
                    /* 根据返回值类型选择传递参数 / Select transfer parameter by return type / Übertragungsparameter nach Rückgabetyp auswählen */
                    if (return_type == PT_RETURN_TYPE_FLOAT || return_type == PT_RETURN_TYPE_DOUBLE) {
                        call_param = &result_float;
                        internal_log_write("INFO", "Using float return value %lf for transfer", result_float);
                    } else if (return_type == PT_RETURN_TYPE_STRUCT_VAL && struct_buffer != NULL) {
                        call_param = struct_buffer;
                        internal_log_write("INFO", "Using struct return value (size=%zu) for transfer", return_size);
                    } else if (return_type == PT_RETURN_TYPE_STRUCT_PTR) {
                        call_param = (void*)(intptr_t)result_int;
                        ctx->stored_size = sizeof(void*);
                        ctx->stored_type = NXLD_PARAM_TYPE_STRING;
                        internal_log_write("INFO", "Using pointer return value %p for transfer", (void*)(intptr_t)result_int);
                    } else {
                        call_param = &result_int;
                        ctx->stored_size = sizeof(int64_t);
                        ctx->stored_type = NXLD_PARAM_TYPE_INT64;
                        internal_log_write("INFO", "Using integer/pointer return value %lld for transfer", (long long)result_int);
                    }
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
                
                if (rule->target_plugin != NULL && rule->target_interface != NULL && new_call_chain_size < sizeof(new_call_chain) / sizeof(new_call_chain[0])) {
                    static char current_call_identifiers[64][512];
                    static size_t current_call_id_index = 0;
                    size_t current_id_index = current_call_id_index % (sizeof(current_call_identifiers) / sizeof(current_call_identifiers[0]));
                    snprintf(current_call_identifiers[current_id_index], sizeof(current_call_identifiers[0]), "%s.%s", rule->target_plugin, rule->target_interface);
                    new_call_chain[new_call_chain_size] = current_call_identifiers[current_id_index];
                    new_call_chain_size++;
                    current_call_id_index++;
                }
                
                internal_log_write("INFO", "Found active call rule %zu (no SetGroup): %s.%s -> %s.%s", 
                              i, active_rule->source_plugin, active_rule->source_interface,
                              active_rule->target_plugin != NULL ? active_rule->target_plugin : "unknown",
                              active_rule->target_interface != NULL ? active_rule->target_interface : "unknown");
                
                int active_call_result = call_target_plugin_interface_internal(active_rule, call_param, recursion_depth + 1, new_call_chain, new_call_chain_size, 0);
                if (active_call_result == 0) {
                    internal_log_write("INFO", "Successfully executed active call rule %zu (no SetGroup)", i);
                } else {
                    internal_log_write("WARNING", "Failed to execute active call rule %zu (no SetGroup, error=%d)", i, active_call_result);
                }
                
                /* UNICAST模式：对于返回值传递，允许传递给同一个接口的不同参数索引，只有当找到完全相同的目标位置时才停止 / UNICAST mode: for return value transfer, allow passing to different parameter indices of the same interface, only stop when exact duplicate target is found / UNICAST-Modus: Für Rückgabewertübertragung, Übergabe an verschiedene Parameterindizes derselben Schnittstelle zulassen, nur stoppen, wenn exaktes doppeltes Ziel gefunden wird */
                if (active_rule->transfer_mode == TRANSFER_MODE_UNICAST) {
                    /* 检查是否有其他规则匹配完全相同的目标位置（插件+接口+参数索引） / Check if other rules match the exact same target location (plugin+interface+parameter index) / Prüfen, ob andere Regeln exakt dieselbe Zielposition (Plugin+Schnittstelle+Parameterindex) abgleichen */
                    int has_exact_duplicate = 0;
                    for (size_t j = i + 1; j < ctx->rule_count; j++) {
                        pointer_transfer_rule_t* next_rule = &ctx->rules[j];
                        if (!next_rule->enabled || next_rule->source_plugin == NULL || next_rule->source_interface == NULL) {
                            continue;
                        }
                        if (next_rule->source_param_index == -1 &&
                            strcmp(next_rule->source_plugin, rule->target_plugin) == 0 &&
                            strcmp(next_rule->source_interface, rule->target_interface) == 0 &&
                            next_rule->target_plugin != NULL && next_rule->target_interface != NULL &&
                            active_rule->target_plugin != NULL && active_rule->target_interface != NULL &&
                            strcmp(next_rule->target_plugin, active_rule->target_plugin) == 0 &&
                            strcmp(next_rule->target_interface, active_rule->target_interface) == 0 &&
                            next_rule->target_param_index == active_rule->target_param_index) {
                            has_exact_duplicate = 1;
                            break;
                        }
                    }
                    /* 只有在找到完全相同的目标位置时才停止，允许传递给同一接口的不同参数 / Only stop when exact duplicate target is found, allow passing to different parameters of the same interface / Nur stoppen, wenn exaktes doppeltes Ziel gefunden wird, Übergabe an verschiedene Parameter derselben Schnittstelle zulassen */
                    if (has_exact_duplicate) {
                        break;
                    }
                }
            }
        }
     }
     
     /* 释放 PT_RETURN_TYPE_STRUCT_VAL 缓冲区 / Free PT_RETURN_TYPE_STRUCT_VAL buffer / PT_RETURN_TYPE_STRUCT_VAL-Puffer freigeben */
     if (return_type == PT_RETURN_TYPE_STRUCT_VAL && struct_buffer != NULL) {
         free(struct_buffer);
         struct_buffer = NULL;
     }
     
     /* 检查是否有规则需要从该接口的参数中获取值（source_param_index >= 0）/ Check if there are rules that need to get parameter values from this interface (source_param_index >= 0) / Prüfen, ob es Regeln gibt, die Parameterwerte von dieser Schnittstelle abrufen müssen (source_param_index >= 0) */
     if (ctx->rules != NULL && rule != NULL && rule->target_plugin != NULL && rule->target_interface != NULL && state != NULL) {
         for (size_t i = 0; i < ctx->rule_count; i++) {
             pointer_transfer_rule_t* param_rule = &ctx->rules[i];
             if (!param_rule->enabled || param_rule->source_plugin == NULL || param_rule->source_interface == NULL) {
                 continue;
             }
             
             /* 检查规则是否匹配当前接口，且source_param_index >= 0（需要从参数中获取值）/ Check if rule matches current interface and source_param_index >= 0 (needs to get value from parameter) / Prüfen, ob Regel mit aktueller Schnittstelle übereinstimmt und source_param_index >= 0 (muss Wert aus Parameter abrufen) */
             if (strcmp(param_rule->source_plugin, rule->target_plugin) == 0 &&
                 strcmp(param_rule->source_interface, rule->target_interface) == 0 &&
                 param_rule->source_param_index >= 0 &&
                 param_rule->source_param_index < state->param_count &&
                 state->param_ready != NULL && state->param_values != NULL &&
                 state->param_ready[param_rule->source_param_index] &&
                 state->param_values[param_rule->source_param_index] != NULL) {
                 
                 void* param_value = state->param_values[param_rule->source_param_index];
                 internal_log_write("INFO", "Triggering rule %zu: %s.%s[%d] -> %s.%s[%d] (getting parameter value)", 
                              i, param_rule->source_plugin, param_rule->source_interface, param_rule->source_param_index,
                              param_rule->target_plugin != NULL ? param_rule->target_plugin : "unknown",
                              param_rule->target_interface != NULL ? param_rule->target_interface : "unknown",
                              param_rule->target_param_index);
                 
                 /* 调用目标接口 / Call target interface / Ziel-Schnittstelle aufrufen */
                 int call_result = call_target_plugin_interface(param_rule, param_value);
                 if (call_result == 0) {
                     internal_log_write("INFO", "Successfully triggered rule %zu for parameter %d", i, param_rule->source_param_index);
                 } else {
                     internal_log_write("WARNING", "Failed to trigger rule %zu for parameter %d (error=%d)", i, param_rule->source_param_index, call_result);
                 }
             }
         }
     }
     
     if (state != NULL) {
         state->in_use = 0;
         
         /* 如果skip_param_cleanup为1，则跳过参数清理（用于SetGroup处理）/ If skip_param_cleanup is 1, skip parameter cleanup (for SetGroup processing) / Wenn skip_param_cleanup 1 ist, Parameterbereinigung überspringen (für SetGroup-Verarbeitung) */
         if (!skip_param_cleanup && state->param_count > 0 && state->param_ready != NULL && state->param_values != NULL) {
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
    
    return call_target_plugin_interface_internal(rule, ptr, 0, initial_call_chain, initial_call_chain_size, 0);
}