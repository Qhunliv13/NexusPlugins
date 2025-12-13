/**
 * @file pointer_transfer_interface_state_info.c
 * @brief 接口信息获取 / Interface Information Retrieval / Schnittstelleninformationen-Abruf
 */

#include "pointer_transfer_interface.h"
#include "pointer_transfer_utils.h"
#include "nxld_plugin_interface.h"
#include "pointer_transfer_platform.h"
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

/**
 * @brief 获取插件接口函数指针 / Get plugin interface function pointers / Plugin-Schnittstellen-Funktionszeiger abrufen
 * @param handle 插件句柄 / Plugin handle / Plugin-Handle
 * @param get_interface_count_out 输出接口数量函数 / Output interface count function / Ausgabe-Interface-Anzahl-Funktion
 * @param get_interface_info_out 输出接口信息函数 / Output interface info function / Ausgabe-Interface-Info-Funktion
 * @param get_param_count_out 输出参数数量函数 / Output parameter count function / Ausgabe-Parameter-Anzahl-Funktion
 * @param get_param_info_out 输出参数信息函数 / Output parameter info function / Ausgabe-Parameter-Info-Funktion
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int get_plugin_interface_functions(void* handle,
                                    void** get_interface_count_out,
                                    void** get_interface_info_out,
                                    void** get_param_count_out,
                                    void** get_param_info_out) {
    if (handle == NULL || get_interface_count_out == NULL || get_interface_info_out == NULL ||
        get_param_count_out == NULL || get_param_info_out == NULL) {
        return -1;
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
        return -1;
    }
    
    *get_interface_count_out = (void*)get_interface_count;
    *get_interface_info_out = (void*)get_interface_info;
    *get_param_count_out = (void*)get_param_count;
    *get_param_info_out = (void*)get_param_info;
    
    return 0;
}

/**
 * @brief 查找接口索引 / Find interface index / Schnittstellenindex suchen
 * @param get_interface_count 接口数量函数 / Interface count function / Interface-Anzahl-Funktion
 * @param get_interface_info 接口信息函数 / Interface info function / Interface-Info-Funktion
 * @param interface_name 接口名称 / Interface name / Schnittstellenname
 * @param interface_index_out 输出接口索引 / Output interface index / Ausgabe-Schnittstellenindex
 * @param inferred_return_type_out 输出推断的返回类型 / Output inferred return type / Ausgabe-Abgeleiteter Rückgabetyp
 * @param saved_desc_buf_out 输出保存的描述缓冲区 / Output saved description buffer / Ausgabe-Gespeicherter Beschreibungspuffer
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int find_interface_index(void* get_interface_count, void* get_interface_info,
                         const char* interface_name,
                         size_t* interface_index_out, pt_return_type_t* inferred_return_type_out,
                         char** saved_desc_buf_out) {
    if (get_interface_count == NULL || get_interface_info == NULL || interface_name == NULL ||
        interface_index_out == NULL || inferred_return_type_out == NULL || saved_desc_buf_out == NULL) {
        return -1;
    }
    
    typedef int32_t (NXLD_PLUGIN_CALL *get_interface_count_func)(size_t*);
    typedef int32_t (NXLD_PLUGIN_CALL *get_interface_info_func)(size_t, char*, size_t, char*, size_t, char*, size_t);
    
    get_interface_count_func count_func = (get_interface_count_func)get_interface_count;
    get_interface_info_func info_func = (get_interface_info_func)get_interface_info;
    
    size_t interface_count = 0;
    if (count_func(&interface_count) != 0) {
        return -1;
    }
    
    size_t iface_name_buf_size = 512;
    char* iface_name = (char*)malloc(iface_name_buf_size);
    if (iface_name == NULL) {
        return -1;
    }
    
    size_t desc_buf_size = 512;
    char* desc_buf = (char*)malloc(desc_buf_size);
    if (desc_buf == NULL) {
        free(iface_name);
        return -1;
    }
    
    size_t target_interface_index = SIZE_MAX;
    pt_return_type_t inferred_return_type = PT_RETURN_TYPE_INTEGER;
    char* saved_desc_buf = NULL;
    
    for (size_t i = 0; i < interface_count; i++) {
        if (info_func(i, iface_name, iface_name_buf_size, desc_buf, desc_buf_size, NULL, 0) == 0) {
            if (strcmp(iface_name, interface_name) == 0) {
                target_interface_index = i;
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
        return -1;
    }
    
    *interface_index_out = target_interface_index;
    *inferred_return_type_out = inferred_return_type;
    *saved_desc_buf_out = saved_desc_buf;
    
    return 0;
}

/**
 * @brief 获取参数计数信息 / Get parameter count information / Parameteranzahl-Informationen abrufen
 * @param get_param_count 参数数量函数 / Parameter count function / Parameter-Anzahl-Funktion
 * @param interface_index 接口索引 / Interface index / Schnittstellenindex
 * @param param_count_type_out 输出参数计数类型 / Output parameter count type / Ausgabe-Parameteranzahl-Typ
 * @param min_count_out 输出最小数量 / Output minimum count / Ausgabe-Mindestanzahl
 * @param max_count_out 输出最大数量 / Output maximum count / Ausgabe-Maximalanzahl
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
int get_parameter_count_info(void* get_param_count, size_t interface_index,
                              nxld_param_count_type_t* param_count_type_out,
                              int32_t* min_count_out, int32_t* max_count_out) {
    if (get_param_count == NULL || param_count_type_out == NULL ||
        min_count_out == NULL || max_count_out == NULL) {
        return -1;
    }
    
    typedef int32_t (NXLD_PLUGIN_CALL *get_interface_param_count_func)(size_t, nxld_param_count_type_t*, int32_t*, int32_t*);
    get_interface_param_count_func count_func = (get_interface_param_count_func)get_param_count;
    
    return count_func(interface_index, param_count_type_out, min_count_out, max_count_out);
}

