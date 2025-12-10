/**
 * @file pointer_transfer_context.c
 * @brief 指针传递插件上下文操作 / Pointer Transfer Plugin Context Operations / Zeigerübertragungs-Plugin-Kontextoperationen
 */

#include "pointer_transfer_context.h"
#include "pointer_transfer_utils.h"
#include "pointer_transfer_platform.h"
#include "pointer_transfer_types.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* 全局上下文变量 / Global context variable / Globale Kontextvariable */
static pointer_transfer_context_t g_context = {
    NULL, NXLD_PARAM_TYPE_UNKNOWN, NULL, 0,
    NULL, 0, 0, NULL, 0,
    NULL, 0, 0, NULL, 0, 0,
    NULL, NULL, 0, 0,
    NULL, 0, 0,
    NULL, NULL, NULL
};

/**
 * @brief 获取全局上下文指针 / Get global context pointer / Globalen Kontextzeiger abrufen
 * @return 全局上下文指针 / Global context pointer / Globaler Kontextzeiger
 */
pointer_transfer_context_t* get_global_context(void) {
    return &g_context;
}

/**
 * @brief 扩展规则数组容量 / Expand rules array capacity / Regel-Array-Kapazität erweitern
 * @return 成功返回0，失败返回非0 / Returns 0 on success, non-zero on failure / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
 */
int expand_rules_capacity(void) {
    pointer_transfer_context_t* ctx = get_global_context();
    size_t new_capacity = ctx->rule_capacity == 0 ? INITIAL_RULE_CAPACITY : ctx->rule_capacity * CAPACITY_GROWTH_FACTOR;
    pointer_transfer_rule_t* new_rules = (pointer_transfer_rule_t*)realloc(ctx->rules, new_capacity * sizeof(pointer_transfer_rule_t));
    if (new_rules == NULL) {
        return -1;
    }
    memset(new_rules + ctx->rule_count, 0, (new_capacity - ctx->rule_count) * sizeof(pointer_transfer_rule_t));
    ctx->rules = new_rules;
    ctx->rule_capacity = new_capacity;
    return 0;
}

/**
 * @brief 扩展已加载插件数组容量 / Expand loaded plugins array capacity / Geladenes Plugin-Array-Kapazität erweitern
 * @return 成功返回0，失败返回非0 / Returns 0 on success, non-zero on failure / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
 */
int expand_loaded_plugins_capacity(void) {
    pointer_transfer_context_t* ctx = get_global_context();
    size_t new_capacity = ctx->loaded_plugin_capacity == 0 ? INITIAL_PLUGIN_CAPACITY : ctx->loaded_plugin_capacity * CAPACITY_GROWTH_FACTOR;
    loaded_plugin_info_t* new_plugins = (loaded_plugin_info_t*)realloc(ctx->loaded_plugins, new_capacity * sizeof(loaded_plugin_info_t));
    if (new_plugins == NULL) {
        return -1;
    }
    memset(new_plugins + ctx->loaded_plugin_count, 0, (new_capacity - ctx->loaded_plugin_count) * sizeof(loaded_plugin_info_t));
    ctx->loaded_plugins = new_plugins;
    ctx->loaded_plugin_capacity = new_capacity;
    return 0;
}

/**
 * @brief 扩展接口状态数组容量 / Expand interface states array capacity / Schnittstellen-Status-Array-Kapazität erweitern
 * @return 成功返回0，失败返回非0 / Returns 0 on success, non-zero on failure / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
 */
int expand_interface_states_capacity(void) {
    pointer_transfer_context_t* ctx = get_global_context();
    size_t new_capacity = ctx->interface_state_capacity == 0 ? INITIAL_INTERFACE_STATE_CAPACITY : ctx->interface_state_capacity * CAPACITY_GROWTH_FACTOR;
    target_interface_state_t* new_states = (target_interface_state_t*)realloc(ctx->interface_states, new_capacity * sizeof(target_interface_state_t));
    if (new_states == NULL) {
        return -1;
    }
    memset(new_states + ctx->interface_state_count, 0, (new_capacity - ctx->interface_state_count) * sizeof(target_interface_state_t));
    ctx->interface_states = new_states;
    ctx->interface_state_capacity = new_capacity;
    return 0;
}

/**
 * @brief 释放传递规则内存 / Free transfer rules memory / Übertragungsregel-Speicher freigeben
 */
void free_transfer_rules(void) {
    pointer_transfer_context_t* ctx = get_global_context();
    if (ctx->rules == NULL) {
        return;
    }
    
    for (size_t i = 0; i < ctx->rule_count; i++) {
        free_single_rule(&ctx->rules[i]);
    }
    
    free(ctx->rules);
    ctx->rules = NULL;
    ctx->rule_count = 0;
    ctx->rule_capacity = 0;
    
    if (ctx->rule_index != NULL) {
        for (size_t i = 0; i < ctx->rule_index_count; i++) {
            if (ctx->rule_index[i].key != NULL) {
                free(ctx->rule_index[i].key);
            }
        }
        free(ctx->rule_index);
        ctx->rule_index = NULL;
        ctx->rule_index_count = 0;
    }
    
    if (ctx->path_cache != NULL) {
        for (size_t i = 0; i < ctx->path_cache_count; i++) {
            if (ctx->path_cache[i].plugin_name != NULL) {
                free(ctx->path_cache[i].plugin_name);
            }
            if (ctx->path_cache[i].plugin_path != NULL) {
                free(ctx->path_cache[i].plugin_path);
            }
        }
        free(ctx->path_cache);
        ctx->path_cache = NULL;
        ctx->path_cache_count = 0;
        ctx->path_cache_capacity = 0;
    }
}

/**
 * @brief 初始化上下文 / Initialize context / Kontext initialisieren
 */
void init_context(void) {
    pointer_transfer_context_t* ctx = get_global_context();
    memset(ctx, 0, sizeof(pointer_transfer_context_t));
    ctx->stored_type = NXLD_PARAM_TYPE_UNKNOWN;
}

/**
 * @brief 清理上下文 / Cleanup context / Kontext bereinigen
 */
void cleanup_context(void) {
    pointer_transfer_context_t* ctx = get_global_context();
    
    if (ctx->stored_type_name != NULL) {
        free(ctx->stored_type_name);
        ctx->stored_type_name = NULL;
    }
    if (ctx->plugin_dll_path != NULL) {
        free(ctx->plugin_dll_path);
        ctx->plugin_dll_path = NULL;
    }
    if (ctx->interface_states != NULL) {
        for (size_t i = 0; i < ctx->interface_state_count; i++) {
            target_interface_state_t* state = &ctx->interface_states[i];
            if (state->plugin_name != NULL) {
                free(state->plugin_name);
            }
            if (state->interface_name != NULL) {
                free(state->interface_name);
            }
            if (state->param_ready != NULL) {
                free(state->param_ready);
            }
            if (state->param_values != NULL) {
                free(state->param_values);
            }
            if (state->param_types != NULL) {
                free(state->param_types);
            }
            if (state->param_sizes != NULL) {
                free(state->param_sizes);
            }
            if (state->param_int_values != NULL) {
                free(state->param_int_values);
            }
            if (state->param_float_values != NULL) {
                free(state->param_float_values);
            }
        }
        free(ctx->interface_states);
        ctx->interface_states = NULL;
    }
    if (ctx->loaded_plugins != NULL) {
        for (size_t i = 0; i < ctx->loaded_plugin_count; i++) {
            if (ctx->loaded_plugins[i].plugin_name != NULL) {
                free(ctx->loaded_plugins[i].plugin_name);
            }
            if (ctx->loaded_plugins[i].plugin_path != NULL) {
                free(ctx->loaded_plugins[i].plugin_path);
            }
            if (ctx->loaded_plugins[i].handle != NULL) {
                pt_platform_close_library(ctx->loaded_plugins[i].handle);
            }
        }
        free(ctx->loaded_plugins);
        ctx->loaded_plugins = NULL;
    }
    free_transfer_rules();
    if (ctx->entry_plugin_name != NULL) {
        free(ctx->entry_plugin_name);
        ctx->entry_plugin_name = NULL;
    }
    if (ctx->entry_plugin_path != NULL) {
        free(ctx->entry_plugin_path);
        ctx->entry_plugin_path = NULL;
    }
    if (ctx->entry_nxpt_path != NULL) {
        free(ctx->entry_nxpt_path);
        ctx->entry_nxpt_path = NULL;
    }
    if (ctx->loaded_nxpt_files != NULL) {
        for (size_t i = 0; i < ctx->loaded_nxpt_count; i++) {
            if (ctx->loaded_nxpt_files[i].plugin_name != NULL) {
                free(ctx->loaded_nxpt_files[i].plugin_name);
            }
            if (ctx->loaded_nxpt_files[i].nxpt_path != NULL) {
                free(ctx->loaded_nxpt_files[i].nxpt_path);
            }
        }
        free(ctx->loaded_nxpt_files);
        ctx->loaded_nxpt_files = NULL;
    }
    memset(ctx, 0, sizeof(pointer_transfer_context_t));
}

