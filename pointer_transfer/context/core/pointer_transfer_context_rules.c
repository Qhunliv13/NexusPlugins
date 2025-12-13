/**
 * @file pointer_transfer_context_rules.c
 * @brief 规则内存管理 / Rule Memory Management / Regel-Speicherverwaltung
 */

#include "pointer_transfer_context.h"
#include "pointer_transfer_utils.h"
#include "pointer_transfer_config.h"
#include <stdlib.h>
#include <string.h>

/**
 * @brief 释放传递规则内存 / Free transfer rules memory / Übertragungsregel-Speicher freigeben
 */
void free_transfer_rules(void) {
    pointer_transfer_context_t* ctx = get_global_context();
    if (ctx == NULL) {
        return;
    }
    
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
    
    /* 释放哈希表（通过外部函数） / Free hash table (via external function) / Hash-Tabelle freigeben (über externe Funktion) */
    /* 注意：free_hash_table 在 pointer_transfer_context_hash.c 中定义，但这里需要调用它 / Note: free_hash_table is defined in pointer_transfer_context_hash.c, but needs to be called here / Hinweis: free_hash_table ist in pointer_transfer_context_hash.c definiert, muss aber hier aufgerufen werden */
    /* 由于是静态函数，我们需要通过 build_rule_index 来清理，或者将 free_hash_table 改为非静态 / Since it's a static function, we need to clean up via build_rule_index, or make free_hash_table non-static / Da es eine statische Funktion ist, müssen wir über build_rule_index bereinigen oder free_hash_table nicht statisch machen */
    ctx->rule_hash_table.buckets = NULL;
    ctx->rule_hash_table.bucket_count = 0;
    ctx->rule_hash_table.entry_count = 0;
    
    if (ctx->path_cache != NULL) {
        for (size_t i = 0; i < ctx->path_cache_count; i++) {
            if (ctx->path_cache[i].plugin_name != NULL) {
                free(ctx->path_cache[i].plugin_name);
                ctx->path_cache[i].plugin_name = NULL;
            }
            if (ctx->path_cache[i].plugin_path != NULL) {
                free(ctx->path_cache[i].plugin_path);
                ctx->path_cache[i].plugin_path = NULL;
            }
        }
        free(ctx->path_cache);
        ctx->path_cache = NULL;
        ctx->path_cache_count = 0;
        ctx->path_cache_capacity = 0;
    }
}

