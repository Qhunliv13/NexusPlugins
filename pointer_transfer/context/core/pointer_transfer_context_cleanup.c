/**
 * @file pointer_transfer_context_cleanup.c
 * @brief 上下文清理 / Context Cleanup / Kontextbereinigung
 */

#include "pointer_transfer_context.h"
#include "pointer_transfer_utils.h"
#include "pointer_transfer_platform.h"
#include "pointer_transfer_config.h"
#include <stdlib.h>
#include <string.h>

/**
 * @brief 清理上下文 / Cleanup context / Kontext bereinigen
 */
void cleanup_context(void) {
    pointer_transfer_context_t* ctx = get_global_context();
    if (ctx == NULL) {
        return;
    }
    
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
                state->plugin_name = NULL;
            }
            if (state->interface_name != NULL) {
                free(state->interface_name);
                state->interface_name = NULL;
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
        free(ctx->interface_states);
        ctx->interface_states = NULL;
        ctx->interface_state_count = 0;
        ctx->interface_state_capacity = 0;
    }
    if (ctx->loaded_plugins != NULL) {
        for (size_t i = 0; i < ctx->loaded_plugin_count; i++) {
            if (ctx->loaded_plugins[i].plugin_name != NULL) {
                free(ctx->loaded_plugins[i].plugin_name);
                ctx->loaded_plugins[i].plugin_name = NULL;
            }
            if (ctx->loaded_plugins[i].plugin_path != NULL) {
                free(ctx->loaded_plugins[i].plugin_path);
                ctx->loaded_plugins[i].plugin_path = NULL;
            }
            if (ctx->loaded_plugins[i].handle != NULL) {
                pt_platform_close_library(ctx->loaded_plugins[i].handle);
                ctx->loaded_plugins[i].handle = NULL;
            }
        }
        free(ctx->loaded_plugins);
        ctx->loaded_plugins = NULL;
        ctx->loaded_plugin_count = 0;
        ctx->loaded_plugin_capacity = 0;
    }
    free_transfer_rules();
    
    /* 清理规则哈希表（手动清理，因为 free_hash_table 是静态的）/ Cleanup rule hash table (manual cleanup, since free_hash_table is static) / Regel-Hash-Tabelle bereinigen (manuelle Bereinigung, da free_hash_table statisch ist) */
    if (ctx->rule_hash_table.buckets != NULL) {
        for (size_t i = 0; i < ctx->rule_hash_table.bucket_count; i++) {
            rule_hash_node_t* node = ctx->rule_hash_table.buckets[i];
            while (node != NULL) {
                rule_hash_node_t* next = node->next;
                free(node);
                node = next;
            }
        }
        free(ctx->rule_hash_table.buckets);
        ctx->rule_hash_table.buckets = NULL;
    }
    ctx->rule_hash_table.bucket_count = 0;
    ctx->rule_hash_table.entry_count = 0;
    
    if (ctx->cached_rule_indices != NULL) {
        free(ctx->cached_rule_indices);
        ctx->cached_rule_indices = NULL;
        ctx->cached_rule_count = 0;
        ctx->cached_rule_capacity = 0;
    }
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
    if (ctx->entry_auto_run_interface != NULL) {
        free(ctx->entry_auto_run_interface);
        ctx->entry_auto_run_interface = NULL;
    }
    if (ctx->loaded_nxpt_files != NULL) {
        for (size_t i = 0; i < ctx->loaded_nxpt_count; i++) {
            if (ctx->loaded_nxpt_files[i].plugin_name != NULL) {
                free(ctx->loaded_nxpt_files[i].plugin_name);
                ctx->loaded_nxpt_files[i].plugin_name = NULL;
            }
            if (ctx->loaded_nxpt_files[i].nxpt_path != NULL) {
                free(ctx->loaded_nxpt_files[i].nxpt_path);
                ctx->loaded_nxpt_files[i].nxpt_path = NULL;
            }
        }
        free(ctx->loaded_nxpt_files);
        ctx->loaded_nxpt_files = NULL;
        ctx->loaded_nxpt_count = 0;
        ctx->loaded_nxpt_capacity = 0;
    }
    free_nxpt_hash_table_internal();
    
    int saved_disable_info_log = ctx->disable_info_log;
    int saved_enable_validation = ctx->enable_validation;
    
    /* 释放忽略插件列表 / Free ignored plugins list / Liste der ignorierten Plugins freigeben */
    if (ctx->ignore_plugins != NULL) {
        for (size_t i = 0; i < ctx->ignore_plugin_count; i++) {
            if (ctx->ignore_plugins[i] != NULL) {
                free(ctx->ignore_plugins[i]);
                ctx->ignore_plugins[i] = NULL;
            }
        }
        free(ctx->ignore_plugins);
        ctx->ignore_plugins = NULL;
        ctx->ignore_plugin_count = 0;
        ctx->ignore_plugin_capacity = 0;
    }
    
    /* 重置所有字段 / Reset all fields / Alle Felder zurücksetzen */
    memset(ctx, 0, sizeof(pointer_transfer_context_t));
    ctx->stored_type = NXLD_PARAM_TYPE_UNKNOWN;
    ctx->disable_info_log = saved_disable_info_log;  /* 恢复日志配置状态 / Restore log configuration state / Protokollkonfigurationsstatus wiederherstellen */
    ctx->enable_validation = saved_enable_validation;  /* 恢复验证配置状态 / Restore validation configuration state / Validierungskonfigurationsstatus wiederherstellen */
    
    internal_log_write("INFO", "Context cleaned up successfully");
}

