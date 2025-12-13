/**
 * @file pointer_transfer_plugin_entry.c
 * @brief 插件入口点实现 / Plugin Entry Point Implementation / Plugin-Einstiegspunkt-Implementierung
 */

#include "pointer_transfer_context.h"

#ifdef _WIN32
#include <windows.h>

/**
 * @brief DLL入口点函数 / DLL entry point function / DLL-Einstiegspunktfunktion
 */
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    (void)lpvReserved;
    (void)hinstDLL;
    
    switch (fdwReason) {
        case DLL_PROCESS_ATTACH: {
            extern void plugin_init(void);
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
    extern void plugin_init(void);
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

