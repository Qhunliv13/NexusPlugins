/**
 * @file pointer_transfer_currying.c
 * @brief 柯里化参数传递机制实现 / Currying Parameter Transfer Mechanism Implementation / Currying-Parameterübertragungsmechanismus-Implementierung
 */

#include "pointer_transfer_currying.h"
#include "pointer_transfer_utils.h"
#include <stdlib.h>
#include <string.h>

/**
 * @brief 创建参数包 / Create parameter pack / Parameterpaket erstellen
 */
pt_param_pack_t* pt_create_param_pack(int param_count, nxld_param_type_t* param_types, void** param_values, size_t* param_sizes) {
    if (param_count < 0 || param_types == NULL || param_values == NULL) {
        return NULL;
    }
    
    pt_param_pack_t* pack = (pt_param_pack_t*)malloc(sizeof(pt_param_pack_t));
    if (pack == NULL) {
        return NULL;
    }
    
    pack->param_count = param_count;
    pack->params = NULL;
    
    if (param_count > 0) {
        pack->params = (pt_curried_param_t*)calloc(param_count, sizeof(pt_curried_param_t));
        if (pack->params == NULL) {
            free(pack);
            return NULL;
        }
        
        for (int i = 0; i < param_count; i++) {
            pack->params[i].type = param_types[i];
            pack->params[i].size = param_sizes != NULL ? param_sizes[i] : 0;
            
            void* value_ptr = param_values[i];
            if (value_ptr == NULL) {
                memset(&pack->params[i].value, 0, sizeof(pack->params[i].value));
                continue;
            }
            
            switch (param_types[i]) {
                case NXLD_PARAM_TYPE_INT32:
                    pack->params[i].value.int32_val = *(int32_t*)value_ptr;
                    break;
                case NXLD_PARAM_TYPE_INT64:
                    pack->params[i].value.int64_val = *(int64_t*)value_ptr;
                    break;
                case NXLD_PARAM_TYPE_FLOAT:
                    pack->params[i].value.float_val = *(float*)value_ptr;
                    break;
                case NXLD_PARAM_TYPE_DOUBLE:
                    pack->params[i].value.double_val = *(double*)value_ptr;
                    break;
                case NXLD_PARAM_TYPE_CHAR:
                    pack->params[i].value.char_val = *(char*)value_ptr;
                    break;
                case NXLD_PARAM_TYPE_POINTER:
                case NXLD_PARAM_TYPE_STRING:
                case NXLD_PARAM_TYPE_VARIADIC:
                case NXLD_PARAM_TYPE_ANY:
                case NXLD_PARAM_TYPE_UNKNOWN:
                    pack->params[i].value.ptr_val = value_ptr;
                    break;
                default:
                    if (param_sizes != NULL && param_sizes[i] > 0) {
                        void* struct_data = malloc(param_sizes[i]);
                        if (struct_data != NULL) {
                            memcpy(struct_data, value_ptr, param_sizes[i]);
                            pack->params[i].value.ptr_val = struct_data;
                        }
                    } else {
                        pack->params[i].value.ptr_val = value_ptr;
                    }
                    break;
            }
        }
    }
    
    return pack;
}

/**
 * @brief 释放参数包 / Free parameter pack / Parameterpaket freigeben
 */
void pt_free_param_pack(pt_param_pack_t* pack) {
    if (pack == NULL) {
        return;
    }
    
    if (pack->params != NULL) {
        for (int i = 0; i < pack->param_count; i++) {
            if (pack->params[i].type != NXLD_PARAM_TYPE_POINTER &&
                pack->params[i].type != NXLD_PARAM_TYPE_STRING &&
                pack->params[i].type != NXLD_PARAM_TYPE_VARIADIC &&
                pack->params[i].type != NXLD_PARAM_TYPE_ANY &&
                pack->params[i].type != NXLD_PARAM_TYPE_UNKNOWN &&
                pack->params[i].value.ptr_val != NULL &&
                pack->params[i].size > sizeof(void*)) {
                free(pack->params[i].value.ptr_val);
            }
        }
        free(pack->params);
    }
    
    free(pack);
}

/**
 * @brief 序列化参数包为单个指针 / Serialize parameter pack to single pointer / Parameterpaket in einzelnen Zeiger serialisieren
 */
void* pt_serialize_param_pack(pt_param_pack_t* pack) {
    if (pack == NULL) {
        return NULL;
    }
    
    if (pack->param_count > 0 && pack->params == NULL) {
        return NULL;
    }
    
    size_t total_size = sizeof(pt_param_pack_t);
    size_t struct_data_size = 0;
    if (pack->params != NULL) {
        for (int i = 0; i < pack->param_count; i++) {
            if (pack->params[i].size > sizeof(void*) && 
                pack->params[i].value.ptr_val != NULL &&
                pack->params[i].type != NXLD_PARAM_TYPE_POINTER &&
                pack->params[i].type != NXLD_PARAM_TYPE_STRING) {
                struct_data_size += pack->params[i].size;
            }
        }
    }
    
    total_size += pack->param_count * sizeof(pt_curried_param_t);
    total_size += struct_data_size;
    
    uint8_t* data = (uint8_t*)malloc(total_size);
    if (data == NULL) {
        return NULL;
    }
    
    pt_param_pack_t* serialized_pack = (pt_param_pack_t*)data;
    serialized_pack->param_count = pack->param_count;
    serialized_pack->params = (pt_curried_param_t*)(data + sizeof(pt_param_pack_t));
    
    uint8_t* current_ptr = data + sizeof(pt_param_pack_t) + pack->param_count * sizeof(pt_curried_param_t);
    if (pack->params != NULL) {
        for (int i = 0; i < pack->param_count; i++) {
            serialized_pack->params[i] = pack->params[i];
            
            if (pack->params[i].size > sizeof(void*) && 
                pack->params[i].value.ptr_val != NULL &&
                pack->params[i].type != NXLD_PARAM_TYPE_POINTER &&
                pack->params[i].type != NXLD_PARAM_TYPE_STRING) {
                memcpy(current_ptr, pack->params[i].value.ptr_val, pack->params[i].size);
                serialized_pack->params[i].value.ptr_val = current_ptr;
                current_ptr += pack->params[i].size;
            }
        }
    }
    
    return data;
}

/**
 * @brief 反序列化指针为参数包 / Deserialize pointer to parameter pack / Zeiger in Parameterpaket deserialisieren
 */
pt_param_pack_t* pt_deserialize_param_pack(void* data) {
    if (data == NULL) {
        return NULL;
    }
    
    pt_param_pack_t* pack = (pt_param_pack_t*)data;
    
    if (pack->param_count < 0 || pack->param_count > 256) {
        return NULL;
    }
    
    if (pack->params == NULL) {
        pack->params = (pt_curried_param_t*)((uint8_t*)data + sizeof(pt_param_pack_t));
    }
    
    return pack;
}

/**
 * @brief 释放序列化的参数包 / Free serialized parameter pack / Serialisiertes Parameterpaket freigeben
 * @note 序列化的参数包是一个连续的内存块，应该直接释放，而不是调用pt_free_param_pack / Serialized parameter pack is a contiguous memory block and should be freed directly, not via pt_free_param_pack / Serialisiertes Parameterpaket ist ein zusammenhängender Speicherblock und sollte direkt freigegeben werden, nicht über pt_free_param_pack
 */
void pt_free_serialized_param_pack(void* data) {
    if (data == NULL) {
        return;
    }
    free(data);
}

/**
 * @brief 验证参数包结构符合ABI约定 / Validate parameter pack structure conforms to ABI convention / Parameterpaket-Strukturvalidierung gemäß ABI-Konvention
 */
int32_t pt_validate_param_pack(pt_param_pack_t* pack) {
    if (pack == NULL) {
        return -1;
    }
    
    if (pack->param_count < 0 || pack->param_count > 256) {
        internal_log_write("ERROR", "Invalid param pack: param_count=%d (must be 0-256)", pack->param_count);
        return -1;
    }
    
    if (pack->param_count > 0) {
        if (pack->params == NULL) {
            internal_log_write("ERROR", "Invalid param pack: params array is NULL but param_count=%d", pack->param_count);
            return -1;
        }
        
        for (int i = 0; i < pack->param_count; i++) {
            pt_curried_param_t* param = &pack->params[i];
            
            if (param->type < NXLD_PARAM_TYPE_VOID || param->type > NXLD_PARAM_TYPE_UNKNOWN) {
                internal_log_write("ERROR", "Invalid param pack: param[%d] has invalid type=%d", i, param->type);
                return -1;
            }
        }
    }
    
    return 0;
}

/**
 * @brief 验证插件函数兼容性 / Validate plugin function compatibility / Plugin-Funktionskompatibilität validieren
 * @param func_ptr 插件函数指针 / Plugin function pointer / Plugin-Funktionszeiger
 * @param expected_param_count 期望的参数数量 / Expected parameter count / Erwartete Parameteranzahl
 * @param return_type 返回值类型 / Return type / Rückgabetyp
 * @return 验证通过返回0，失败返回-1 / Returns 0 if validation passes, -1 on failure / Gibt 0 zurück wenn Validierung erfolgreich, -1 bei Fehler
 */
int32_t pt_validate_plugin_function(void* func_ptr, int expected_param_count, pt_return_type_t return_type) {
    if (func_ptr == NULL) {
        internal_log_write("ERROR", "Plugin function validation failed: func_ptr is NULL");
        return -1;
    }
    
    pt_param_pack_t* test_pack = NULL;
    int32_t validation_result = 0;
    
    if (expected_param_count == 0) {
        test_pack = pt_create_param_pack(0, NULL, NULL, NULL);
        if (test_pack == NULL) {
            internal_log_write("ERROR", "Plugin function validation failed: failed to create test param pack (0 params)");
            return -1;
        }
    } else if (expected_param_count > 0 && expected_param_count <= 2) {
        nxld_param_type_t types[2] = {NXLD_PARAM_TYPE_INT32, NXLD_PARAM_TYPE_INT32};
        int32_t values[2] = {1, 2};
        void* value_ptrs[2] = {&values[0], &values[1]};
        size_t sizes[2] = {sizeof(int32_t), sizeof(int32_t)};
        
        test_pack = pt_create_param_pack(expected_param_count, types, value_ptrs, sizes);
        if (test_pack == NULL) {
            internal_log_write("ERROR", "Plugin function validation failed: failed to create test param pack (%d params)", expected_param_count);
            return -1;
        }
    } else {
        internal_log_write("INFO", "Plugin function validation: skipping detailed test for function with %d params (testing basic compatibility only)", expected_param_count);
        return 0;
    }
    
    if (pt_validate_param_pack(test_pack) != 0) {
        internal_log_write("ERROR", "Plugin function validation failed: test param pack validation failed");
        pt_free_param_pack(test_pack);
        return -1;
    }
    
    int64_t test_result_int = 0;
    double test_result_float = 0.0;
    void* test_result_struct = NULL;
    
    int32_t call_result = pt_call_with_currying(func_ptr, test_pack, return_type, 0, 
                                                 &test_result_int, &test_result_float, test_result_struct);
    
    if (call_result != 0) {
        internal_log_write("ERROR", "Plugin function validation failed: test call returned error code %d", call_result);
        validation_result = -1;
    } else {
        internal_log_write("INFO", "Plugin function validation passed: test call succeeded");
        validation_result = 0;
    }
    
    pt_free_param_pack(test_pack);
    return validation_result;
}

/**
 * @brief 柯里化调用函数 / Call function using currying / Funktion mit Currying aufrufen
 * @param func_ptr 函数指针 / Function pointer / Funktionszeiger
 * @param pack 参数包指针 / Parameter pack pointer / Parameterpaket-Zeiger
 * @param return_type 返回值类型 / Return type / Rückgabetyp
 * @param return_size 返回值大小 / Return size / Rückgabegröße
 * @param result_int 输出整数返回值指针 / Output integer return value pointer / Ausgabe-Integer-Rückgabewert-Zeiger
 * @param result_float 输出浮点返回值指针 / Output floating-point return value pointer / Ausgabe-Gleitkomma-Rückgabewert-Zeiger
 * @param result_struct 输出结构体返回值缓冲区 / Output struct return value buffer / Ausgabe-Struktur-Rückgabewert-Puffer
 * @return 成功返回0，失败返回非0值 / Returns 0 on success, non-zero on failure / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
 */
int32_t pt_call_with_currying(void* func_ptr, pt_param_pack_t* pack, 
                               pt_return_type_t return_type, size_t return_size,
                               int64_t* result_int, double* result_float, void* result_struct) {
    if (func_ptr == NULL || pack == NULL || result_int == NULL || result_float == NULL) {
        return -1;
    }
    
    if (pt_validate_param_pack(pack) != 0) {
        internal_log_write("ERROR", "Call with currying failed: param pack validation failed");
        return -1;
    }
    
    typedef int32_t (*curried_func_int_t)(void*);
    typedef float (*curried_func_float_t)(void*);
    typedef double (*curried_func_double_t)(void*);
    typedef int64_t (*curried_func_int64_t)(void*);
    typedef void* (*curried_func_ptr_t)(void*);
    
    switch (return_type) {
        case PT_RETURN_TYPE_FLOAT: {
            curried_func_float_t f = (curried_func_float_t)func_ptr;
            *result_float = (double)f((void*)pack);
            *result_int = 0;
            return 0;
        }
        case PT_RETURN_TYPE_DOUBLE: {
            curried_func_double_t f = (curried_func_double_t)func_ptr;
            *result_float = f((void*)pack);
            *result_int = 0;
            return 0;
        }
        case PT_RETURN_TYPE_STRUCT_PTR: {
            curried_func_ptr_t f = (curried_func_ptr_t)func_ptr;
            void* struct_ptr = f((void*)pack);
            if (result_struct != NULL && return_size > 0 && struct_ptr != NULL) {
                memcpy(result_struct, struct_ptr, return_size);
            }
            *result_int = (int64_t)struct_ptr;
            *result_float = 0.0;
            return 0;
        }
        case PT_RETURN_TYPE_STRUCT_VAL: {
            typedef void (*curried_func_struct_val_t)(void*, void*);
            curried_func_struct_val_t f = (curried_func_struct_val_t)func_ptr;
            if (result_struct != NULL) {
                f((void*)pack, result_struct);
            }
            *result_int = 0;
            *result_float = 0.0;
            return 0;
        }
        default:
        {
            curried_func_int_t f = (curried_func_int_t)func_ptr;
            *result_int = (int64_t)f((void*)pack);
            *result_float = 0.0;
            return 0;
        }
    }
}
