/**
 * @file pointer_transfer_context_global.c
 * @brief 全局上下文管理 / Global Context Management / Globale Kontextverwaltung
 */

#include "pointer_transfer_context.h"
#include "pointer_transfer_types.h"
#include <stdint.h>

/* 全局上下文变量 / Global context variable / Globale Kontextvariable */
static pointer_transfer_context_t g_context = {
    NULL, NXLD_PARAM_TYPE_UNKNOWN, NULL, 0,
    NULL, 0, 0, {NULL, 0, 0}, NULL, 0, 0,
    NULL, 0, 0, NULL, 0, 0,
    NULL, NULL, 0, 0,
    NULL, 0, 0,
    NULL, NULL, NULL, NULL,
    0,  /* disable_info_log = 0 (默认启用INFO级别日志) / disable_info_log = 0 (INFO level logging enabled by default) / disable_info_log = 0 (INFO-Level-Protokollierung standardmäßig aktiviert) */
    0,  /* enable_validation = 0 (默认禁用验证) / enable_validation = 0 (validation disabled by default) / enable_validation = 0 (Validierung standardmäßig deaktiviert) */
    NULL, 0, 0  /* ignore_plugins = NULL, ignore_plugin_count = 0, ignore_plugin_capacity = 0 (默认无忽略插件) / ignore_plugins = NULL, ignore_plugin_count = 0, ignore_plugin_capacity = 0 (no ignored plugins by default) / ignore_plugins = NULL, ignore_plugin_count = 0, ignore_plugin_capacity = 0 (standardmäßig keine ignorierten Plugins) */
};

/**
 * @brief 获取全局上下文指针 / Get global context pointer / Globalen Kontextzeiger abrufen
 * @return 全局上下文指针 / Global context pointer / Globaler Kontextzeiger
 */
pointer_transfer_context_t* get_global_context(void) {
    return &g_context;
}

