/**
 * @file pointer_transfer_context_init.c
 * @brief 上下文初始化 / Context Initialization / Kontextinitialisierung
 */

#include "pointer_transfer_context.h"
#include "pointer_transfer_types.h"
#include <stdint.h>

/**
 * @brief 初始化上下文 / Initialize context / Kontext initialisieren
 */
void init_context(void) {
    pointer_transfer_context_t* ctx = get_global_context();
    if (ctx == NULL) {
        return;
    }
    
    /* 清理旧数据 / Cleanup old data / Alte Daten bereinigen */
    if (ctx->rules != NULL || ctx->loaded_plugins != NULL || ctx->interface_states != NULL ||
        ctx->stored_type_name != NULL || ctx->plugin_dll_path != NULL) {
        cleanup_context();
    }
    
    /* 确保所有字段已初始化 / Ensure all fields are initialized / Sicherstellen, dass alle Felder initialisiert sind */
    ctx->stored_type = NXLD_PARAM_TYPE_UNKNOWN;
    
    /* 初始化哈希表结构 / Initialize hash table structure / Hash-Tabellen-Struktur initialisieren */
    ctx->rule_hash_table.buckets = NULL;
    ctx->rule_hash_table.bucket_count = 0;
    ctx->rule_hash_table.entry_count = 0;
    
    /* 初始化.nxpt哈希表结构 / Initialize .nxpt hash table structure / .nxpt-Hash-Tabellen-Struktur initialisieren */
    ctx->nxpt_hash_table.buckets = NULL;
    ctx->nxpt_hash_table.bucket_count = 0;
    ctx->nxpt_hash_table.entry_count = 0;
}

