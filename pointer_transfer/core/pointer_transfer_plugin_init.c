/**
 * @file pointer_transfer_plugin_init.c
 * @brief 插件初始化实现 / Plugin Initialization Implementation / Plugin-Initialisierungsimplementierung
 */

#include "pointer_transfer_context.h"
#include "pointer_transfer_config.h"
#include "pointer_transfer_plugin_loader.h"
#include "pointer_transfer_plugin.h"
#include "pointer_transfer_utils.h"
#include "pointer_transfer_platform.h"
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

/**
 * @brief 插件初始化函数 / Plugin initialization function / Plugin-Initialisierungsfunktion
 */
static void plugin_init(void) {
    init_context();
    
    size_t dll_path_size = 4096;
    char* dll_path = (char*)malloc(dll_path_size);
    if (dll_path == NULL) {
        return;
    }
    
    if (get_current_dll_path(dll_path, dll_path_size) != 0) {
        free(dll_path);
        return;
    }
    
    pointer_transfer_context_t* ctx = get_global_context();
    ctx->plugin_dll_path = allocate_string(dll_path);
    if (ctx->plugin_dll_path == NULL) {
        internal_log_write("ERROR", "Failed to allocate memory for plugin DLL path");
        free(dll_path);
        return;
    }
    
    size_t nxpt_path_size = strlen(dll_path) + 10;
    char* nxpt_path = (char*)malloc(nxpt_path_size);
    if (nxpt_path != NULL) {
        if (build_nxpt_path(dll_path, nxpt_path, nxpt_path_size) == 0) {
            /* 加载插件配置文件本身的规则 / Load plugin config file's own rules / Eigene Regeln der Plugin-Konfigurationsdatei laden */
            load_transfer_rules(nxpt_path);
            
            if (parse_entry_plugin_config(nxpt_path) == 0) {
                if (ctx->entry_nxpt_path != NULL) {
                    internal_log_write("INFO", "Loading entry plugin .nxpt file: %s", ctx->entry_nxpt_path);
                    if (load_transfer_rules(ctx->entry_nxpt_path) == 0) {
                        mark_nxpt_loaded(ctx->entry_plugin_name, ctx->entry_nxpt_path);
                        
                        size_t entry_rule_count = ctx->rule_count;
                        for (size_t i = 0; i < entry_rule_count; i++) {
                            pointer_transfer_rule_t* rule = &ctx->rules[i];
                            if (rule->enabled && rule->target_plugin != NULL && rule->target_plugin_path != NULL) {
                                if (!is_nxpt_loaded(rule->target_plugin)) {
                                    chain_load_plugin_nxpt(rule->target_plugin, rule->target_plugin_path);
                                }
                            }
                        }
                    }
                }
                
                /* 自动运行入口插件接口 / Auto-run entry plugin interface / Einstiegs-Plugin-Schnittstelle automatisch ausführen */
                if (ctx->entry_plugin_name != NULL && ctx->entry_plugin_path != NULL && ctx->entry_auto_run_interface != NULL) {
                    void* entry_handle = load_target_plugin(ctx->entry_plugin_name, ctx->entry_plugin_path);
                    if (entry_handle != NULL) {
                        void* auto_run_func = pt_platform_get_symbol(entry_handle, ctx->entry_auto_run_interface);
                        if (auto_run_func != NULL) {
                            typedef int32_t (NXLD_PLUGIN_CALL *AutoRunFunc)(void*);
                            AutoRunFunc auto_run = (AutoRunFunc)auto_run_func;
                            int32_t return_value = auto_run(NULL);
                            
                            CallPlugin(ctx->entry_plugin_name, ctx->entry_auto_run_interface, -1, &return_value);
                        }
                    }
                }
            }
        }
        free(nxpt_path);
    }
    
    free(dll_path);
}

#ifdef _WIN32
/**
 * @brief DLL入口点函数 / DLL entry point function / DLL-Einstiegspunktfunktion
 */
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    (void)lpvReserved;
    (void)hinstDLL;
    
    switch (fdwReason) {
        case DLL_PROCESS_ATTACH: {
            plugin_init();
            break;
        }
        case DLL_PROCESS_DETACH:
            cleanup_context();
            break;
    }
    
    return TRUE;
}
#else
/**
 * @brief 构造函数 / Constructor / Konstruktor
 */
__attribute__((constructor))
static void plugin_constructor(void) {
    plugin_init();
}

/**
 * @brief 析构函数 / Destructor / Destruktor
 */
__attribute__((destructor))
static void plugin_destructor(void) {
    cleanup_context();
}
#endif

