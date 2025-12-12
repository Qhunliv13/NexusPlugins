/**
 * @file pointer_transfer_currying.c
 * @brief 柯里化参数传递机制实现 / Currying Parameter Transfer Mechanism Implementation / Currying-Parameterübertragungsmechanismus-Implementierung
 */

#include "pointer_transfer_currying.h"
#include "pointer_transfer_utils.h"
#include "pointer_transfer_context.h"
#include "pointer_transfer_platform.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

/**
 * @brief 创建参数包 / Create parameter pack / Parameterpaket erstellen
 * @param param_count 参数数量 / Parameter count / Parameteranzahl
 * @param param_types 参数类型数组 / Parameter types array / Parametertyp-Array
 * @param param_values 参数值数组 / Parameter values array / Parameterwerte-Array
 * @param param_sizes 参数大小数组 / Parameter sizes array / Parametergrößen-Array
 * @return 成功返回参数包指针，失败返回NULL / Returns parameter pack pointer on success, NULL on failure / Gibt Parameterpaket-Zeiger bei Erfolg zurück, NULL bei Fehler
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
                        } else {
                            /* 内存分配失败，回退到直接使用指针 / Memory allocation failed, fallback to direct pointer / Speicherzuweisung fehlgeschlagen, Fallback auf direkten Zeiger */
                            internal_log_write("WARNING", "Failed to allocate memory for struct parameter %d (size=%zu), using pointer directly", i, param_sizes[i]);
                            pack->params[i].value.ptr_val = value_ptr;
                            pack->params[i].size = 0; /* 标记为未分配 / Mark as not allocated / Als nicht zugewiesen markieren */
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
 * @param pack 参数包指针 / Parameter pack pointer / Parameterpaket-Zeiger
 */
void pt_free_param_pack(pt_param_pack_t* pack) {
    if (pack == NULL) {
        return;
    }
    
    if (pack->params != NULL) {
        for (int i = 0; i < pack->param_count; i++) {
            /* 只释放通过malloc分配的内存（size > sizeof(void*)且size > 0） / Only free memory allocated via malloc (size > sizeof(void*) and size > 0) / Nur Speicher freigeben, der über malloc zugewiesen wurde (size > sizeof(void*) und size > 0) */
            if (pack->params[i].type != NXLD_PARAM_TYPE_POINTER &&
                pack->params[i].type != NXLD_PARAM_TYPE_STRING &&
                pack->params[i].type != NXLD_PARAM_TYPE_VARIADIC &&
                pack->params[i].type != NXLD_PARAM_TYPE_ANY &&
                pack->params[i].type != NXLD_PARAM_TYPE_UNKNOWN &&
                pack->params[i].value.ptr_val != NULL &&
                pack->params[i].size > sizeof(void*) &&
                pack->params[i].size > 0) { /* 确保size > 0，避免释放未分配的内存 / Ensure size > 0 to avoid freeing unallocated memory / Sicherstellen, dass size > 0, um Freigabe von nicht zugewiesenem Speicher zu vermeiden */
                free(pack->params[i].value.ptr_val);
            }
        }
        free(pack->params);
    }
    
    free(pack);
}

/**
 * @brief 序列化参数包为单个指针 / Serialize parameter pack to single pointer / Parameterpaket in einzelnen Zeiger serialisieren
 * @param pack 参数包指针 / Parameter pack pointer / Parameterpaket-Zeiger
 * @return 成功返回序列化后的指针，失败返回NULL / Returns serialized pointer on success, NULL on failure / Gibt serialisierten Zeiger bei Erfolg zurück, NULL bei Fehler
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
 * @param data 序列化数据指针（由pt_serialize_param_pack返回的连续内存块）/ Serialized data pointer (contiguous memory block returned by pt_serialize_param_pack) / Serialisierter Datenzeiger (zusammenhängender Speicherblock, zurückgegeben von pt_serialize_param_pack)
 * @return 成功返回参数包指针，失败返回NULL / Returns parameter pack pointer on success, NULL on failure / Gibt Parameterpaket-Zeiger bei Erfolg zurück, NULL bei Fehler
 * @note 序列化后的数据格式与pt_param_pack_t完全兼容，支持直接类型转换。此函数主要用于设置params指针（当其为NULL时）并验证数据有效性。插件可将void*直接转换为pt_param_pack_t*使用 / Serialized data format is fully compatible with pt_param_pack_t and supports direct type casting. This function is primarily used to set params pointer (when NULL) and validate data validity. Plugins may directly cast void* to pt_param_pack_t* / Serialisiertes Datenformat ist vollständig mit pt_param_pack_t kompatibel und unterstützt direkte Typumwandlung. Diese Funktion wird hauptsächlich verwendet, um params-Zeiger zu setzen (wenn NULL) und Datenvalidität zu überprüfen. Plugins können void* direkt in pt_param_pack_t* umwandeln
 */
pt_param_pack_t* pt_deserialize_param_pack(void* data) {
    if (data == NULL) {
        return NULL;
    }
    
    pt_param_pack_t* pack = (pt_param_pack_t*)data;
    
    if (pack->param_count < 0 || pack->param_count > 256) {
        return NULL;
    }
    
    /* 当params为NULL时，表示数据为序列化格式，需设置正确的指针位置 / If params is NULL, this indicates serialized data format, need to set correct pointer position / Wenn params NULL ist, bedeutet dies serialisiertes Datenformat, korrekte Zeigerposition muss gesetzt werden */
    if (pack->params == NULL) {
        pack->params = (pt_curried_param_t*)((uint8_t*)data + sizeof(pt_param_pack_t));
    }
    
    return pack;
}

/**
 * @brief 释放序列化的参数包 / Free serialized parameter pack / Serialisiertes Parameterpaket freigeben
 * @note 序列化的参数包为连续内存块，需直接释放，不应调用pt_free_param_pack / Serialized parameter pack is a contiguous memory block and must be freed directly, not via pt_free_param_pack / Serialisiertes Parameterpaket ist ein zusammenhängender Speicherblock und muss direkt freigegeben werden, nicht über pt_free_param_pack
 */
void pt_free_serialized_param_pack(void* data) {
    if (data == NULL) {
        return;
    }
    free(data);
}

/**
 * @brief 验证参数包结构符合ABI约定 / Validate parameter pack structure conforms to ABI convention / Parameterpaket-Strukturvalidierung gemäß ABI-Konvention
 * @param pack 参数包指针 / Parameter pack pointer / Parameterpaket-Zeiger
 * @return 有效返回0，无效返回-1 / Returns 0 if valid, -1 if invalid / Gibt 0 zurück wenn gültig, -1 wenn ungültig
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
 * @brief 构建.nxpv文件路径 / Build .nxpv file path / .nxpv-Dateipfad erstellen
 * @param plugin_path 插件文件路径 / Plugin file path / Plugin-Dateipfad
 * @param nxpv_path 输出.nxpv路径缓冲区 / Output .nxpv path buffer / Ausgabe-.nxpv-Pfad-Puffer
 * @param buffer_size 缓冲区大小 / Buffer size / Puffergröße
 * @return 成功返回0，失败返回非0 / Returns 0 on success, non-zero on failure / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
 */
static int build_nxpv_path(const char* plugin_path, char* nxpv_path, size_t buffer_size) {
    if (plugin_path == NULL || nxpv_path == NULL || buffer_size == 0) {
        return -1;
    }
    
    const char* ext_pos = strrchr(plugin_path, '.');
    
    size_t base_len;
    if (ext_pos != NULL) {
        base_len = ext_pos - plugin_path;
    } else {
        base_len = strlen(plugin_path);
    }
    
    if (base_len + 6 >= buffer_size) {
        return -1;
    }
    
    memcpy(nxpv_path, plugin_path, base_len);
    memcpy(nxpv_path + base_len, ".nxpv", 6);
    
    return 0;
}

/**
 * @brief 从.nxpv文件读取验证信息 / Read validation info from .nxpv file / Validierungsinformationen aus .nxpv-Datei lesen
 * @param nxpv_path .nxpv文件路径 / .nxpv file path / .nxpv-Dateipfad
 * @param timestamp 输出时间戳指针 / Output timestamp pointer / Ausgabe-Zeitstempel-Zeiger
 * @param is_valid 输出验证结果指针 / Output validation result pointer / Ausgabe-Validierungsergebnis-Zeiger
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
static int32_t read_validation_from_nxpv(const char* nxpv_path, int64_t* timestamp, int* is_valid) {
    if (nxpv_path == NULL || timestamp == NULL || is_valid == NULL) {
        return -1;
    }
    
    FILE* fp = fopen(nxpv_path, "r");
    if (fp == NULL) {
        return -1;
    }
    
    char line_buffer[1024];
    int32_t in_validation_section = 0;
    int32_t found_timestamp = 0;
    int32_t found_valid = 0;
    
    while (fgets(line_buffer, sizeof(line_buffer), fp) != NULL && (!found_timestamp || !found_valid)) {
        size_t len = strlen(line_buffer);
        while (len > 0 && (line_buffer[len - 1] == '\r' || line_buffer[len - 1] == '\n' || line_buffer[len - 1] == ' ' || line_buffer[len - 1] == '\t')) {
            line_buffer[--len] = '\0';
        }
        
        if (len == 0 || line_buffer[0] == '#') {
            continue;
        }
        
        if (len >= 2 && line_buffer[0] == '[' && line_buffer[len - 1] == ']') {
            if (strcmp(line_buffer, "[Validation]") == 0) {
                in_validation_section = 1;
            } else {
                in_validation_section = 0;
            }
            continue;
        }
        
        if (in_validation_section) {
            char key[512];
            char value[2048];
            if (parse_key_value_simple(line_buffer, key, sizeof(key), value, sizeof(value))) {
                trim_string(key);
                trim_string(value);
                
                if (strcmp(key, "Timestamp") == 0) {
                    char* endptr = NULL;
                    int64_t ts = strtoll(value, &endptr, 10);
                    if (endptr != NULL && *endptr == '\0') {
                        *timestamp = ts;
                        found_timestamp = 1;
                    }
                } else if (strcmp(key, "Valid") == 0) {
                    if (strcmp(value, "1") == 0 || strcmp(value, "true") == 0 || 
                        strcmp(value, "True") == 0 || strcmp(value, "TRUE") == 0 ||
                        strcmp(value, "yes") == 0 || strcmp(value, "Yes") == 0 || 
                        strcmp(value, "YES") == 0) {
                        *is_valid = 1;
                    } else {
                        *is_valid = 0;
                    }
                    found_valid = 1;
                }
            }
        }
    }
    
    fclose(fp);
    
    if (found_timestamp && found_valid) {
        return 0;
    }
    
    return -1;
}

/**
 * @brief 生成.nxpv验证文件 / Generate .nxpv validation file / .nxpv-Validierungsdatei generieren
 * @param plugin_path 插件文件路径 / Plugin file path / Plugin-Dateipfad
 * @param timestamp 时间戳 / Timestamp / Zeitstempel
 * @param is_valid 验证结果（1=合规，0=不合规）/ Validation result (1=valid, 0=invalid) / Validierungsergebnis (1=gültig, 0=ungültig)
 * @return 成功返回0，失败返回-1 / Returns 0 on success, -1 on failure / Gibt 0 bei Erfolg zurück, -1 bei Fehler
 */
static int32_t generate_nxpv_file(const char* plugin_path, int64_t timestamp, int is_valid) {
    if (plugin_path == NULL) {
        return -1;
    }
    
    char nxpv_path[1024];
    if (build_nxpv_path(plugin_path, nxpv_path, sizeof(nxpv_path)) != 0) {
        return -1;
    }
    
    FILE* fp = fopen(nxpv_path, "w");
    if (fp == NULL) {
        return -1;
    }
    
    /**
     * 写入验证文件头 / Write validation file header / Validierungsdateikopf schreiben
     * 包含文件格式标识和版本信息 / Contains file format identifier and version information / Enthält Dateiformatkennung und Versionsinformationen
     */
    fprintf(fp, "# NXLD Plugin Validation File / NXLD插件验证文件\n");
    fprintf(fp, "# Generated automatically / 自动生成\n");
    fprintf(fp, "# Format: NXPV v1.0 / 格式: NXPV v1.0\n");
    fprintf(fp, "\n");
    
    /**
     * 写入验证信息节 / Write validation information section / Validierungsinformationsabschnitt schreiben
     * 包含时间戳和验证结果 / Contains timestamp and validation result / Enthält Zeitstempel und Validierungsergebnis
     */
    fprintf(fp, "[Validation]\n");
    fprintf(fp, "Timestamp=%lld\n", (long long)timestamp);
    fprintf(fp, "Valid=%d\n", is_valid);
    fprintf(fp, "\n");
    
    fclose(fp);
    return 0;
}

/**
 * @brief 验证插件函数兼容性（完整检查）/ Validate plugin function compatibility (full check) / Plugin-Funktionskompatibilität validieren (vollständige Prüfung)
 * @param func_ptr 插件函数指针 / Plugin function pointer / Plugin-Funktionszeiger
 * @param plugin_path 插件文件路径 / Plugin file path / Plugin-Dateipfad
 * @param interface_name 接口名称 / Interface name / Schnittstellenname
 * @param expected_param_count 期望的参数数量 / Expected parameter count / Erwartete Parameteranzahl
 * @param return_type 返回值类型 / Return type / Rückgabetyp
 * @return 验证通过返回0，失败返回-1 / Returns 0 if validation passes, -1 on failure / Gibt 0 zurück wenn Validierung erfolgreich, -1 bei Fehler
 * @note 此函数执行完整检查，对所有参数数量均执行实际调用测试。当验证结果合规且时间戳未变更时，跳过重复验证 / This function performs full check and executes actual call test for all parameter counts. When validation result is valid and timestamp is unchanged, skip repeated validation / Diese Funktion führt vollständige Prüfung durch und führt tatsächlichen Aufruftest für alle Parameteranzahlen aus. Wenn Validierungsergebnis gültig und Zeitstempel unverändert ist, wiederholte Validierung überspringen
 */
int32_t pt_validate_plugin_function(void* func_ptr, const char* plugin_path, const char* interface_name, int expected_param_count, pt_return_type_t return_type) {
    if (func_ptr == NULL) {
        internal_log_write("ERROR", "Plugin function validation failed: func_ptr is NULL");
        return -1;
    }
    
    if (expected_param_count < 0 || expected_param_count > 256) {
        internal_log_write("ERROR", "Plugin function validation failed: invalid expected_param_count=%d (must be 0-256)", expected_param_count);
        return -1;
    }
    
    /* 检查是否启用验证 / Check if validation is enabled / Prüfen, ob Validierung aktiviert ist */
    pointer_transfer_context_t* ctx = get_global_context();
    if (ctx == NULL || ctx->enable_validation == 0) {
        /* 验证未启用，直接返回成功 / Validation not enabled, return success directly / Validierung nicht aktiviert, direkt Erfolg zurückgeben */
        return 0;
    }
    
    /* 检查插件是否在忽略列表中 / Check if plugin is in ignore list / Prüfen, ob Plugin in Ignorierliste ist */
    if (plugin_path != NULL && is_plugin_ignored(plugin_path)) {
        internal_log_write("INFO", "Plugin function validation skipped: plugin is in ignore list (plugin=%s, interface=%s, param_count=%d)",
                         plugin_path, interface_name != NULL ? interface_name : "unknown", expected_param_count);
        return 0;
    }
    
    /* 递归查找同目录下的所有DLL文件并为它们生成.nxpv文件（仅在启用验证时）/ Recursively find all DLL files in the same directory and generate .nxpv files for them (only when validation is enabled) / Alle DLL-Dateien im selben Verzeichnis rekursiv finden und .nxpv-Dateien für sie generieren (nur wenn Validierung aktiviert ist) */
    if (plugin_path != NULL) {
        char dll_files[256][1024];
        int file_count = 0;
        if (pt_platform_find_all_dll_files(plugin_path, dll_files, 256, &file_count) == 0 && file_count > 0) {
            internal_log_write("INFO", "Found %d DLL files in directory, generating .nxpv files", file_count);
            for (int i = 0; i < file_count; i++) {
                /* 跳过当前插件文件（后续处理）/ Skip current plugin file (processed subsequently) / Aktuelle Plugin-Datei überspringen (wird anschließend verarbeitet) */
                if (strcmp(dll_files[i], plugin_path) == 0) {
                    continue;
                }
                
                /* 检查DLL文件是否在忽略列表中 / Check if DLL file is in ignore list / Prüfen, ob DLL-Datei in Ignorierliste ist */
                if (is_plugin_ignored(dll_files[i])) {
                    internal_log_write("INFO", "Skipping ignored plugin DLL: %s", dll_files[i]);
                    continue;
                }
                
                /* 检查.nxpv文件是否已存在且时间戳匹配 / Check if .nxpv file already exists and timestamp matches / Prüfen, ob .nxpv-Datei bereits existiert und Zeitstempel übereinstimmt */
                char nxpv_path[1024];
                if (build_nxpv_path(dll_files[i], nxpv_path, sizeof(nxpv_path)) == 0) {
                    int64_t cached_timestamp = 0;
                    int cached_valid = 0;
                    
                    int64_t dll_timestamp = 0;
                    if (pt_platform_get_file_timestamp(dll_files[i], &dll_timestamp) == 0) {
                        /* 如果.nxpv文件不存在或时间戳不匹配，生成新的验证文件 / If .nxpv file doesn't exist or timestamp doesn't match, generate new validation file / Wenn .nxpv-Datei nicht existiert oder Zeitstempel nicht übereinstimmt, neue Validierungsdatei generieren */
                        if (read_validation_from_nxpv(nxpv_path, &cached_timestamp, &cached_valid) != 0 || 
                            cached_timestamp != dll_timestamp) {
                            /* 为DLL文件生成.nxpv文件，标记为未验证（Valid=0），等待实际验证 / Generate .nxpv file for DLL file, mark as unvalidated (Valid=0), pending actual validation / .nxpv-Datei für DLL-Datei generieren, als nicht validiert markieren (Valid=0), wartet auf tatsächliche Validierung */
                            if (generate_nxpv_file(dll_files[i], dll_timestamp, 0) == 0) {
                                internal_log_write("INFO", "Generated .nxpv validation file for DLL: %s (timestamp=%lld, valid=0, not validated)", 
                                                 dll_files[i], (long long)dll_timestamp);
                            }
                        }
                    }
                }
            }
        }
    }
    
    /* 检查.nxpv文件是否存在，如果存在且时间戳匹配且验证结果为合规，则跳过验证 / Check if .nxpv file exists, if exists and timestamp matches and validation result is valid, skip validation / Prüfen, ob .nxpv-Datei existiert, wenn vorhanden und Zeitstempel übereinstimmt und Validierungsergebnis gültig ist, Validierung überspringen */
    if (plugin_path != NULL) {
        char nxpv_path[1024];
        if (build_nxpv_path(plugin_path, nxpv_path, sizeof(nxpv_path)) == 0) {
            int64_t cached_timestamp = 0;
            int cached_valid = 0;
            
            /* 读取缓存的验证信息 / Read cached validation info / Gecachte Validierungsinformationen lesen */
            if (read_validation_from_nxpv(nxpv_path, &cached_timestamp, &cached_valid) == 0) {
                /* 获取当前DLL文件的修改时间戳 / Get current DLL file modification timestamp / Aktuellen DLL-Dateiänderungszeitstempel abrufen */
                int64_t current_timestamp = 0;
                if (pt_platform_get_file_timestamp(plugin_path, &current_timestamp) == 0) {
                    /* 如果时间戳匹配且验证结果为合规，则跳过验证 / If timestamp matches and validation result is valid, skip validation / Wenn Zeitstempel übereinstimmt und Validierungsergebnis gültig ist, Validierung überspringen */
                    if (current_timestamp == cached_timestamp && cached_valid == 1) {
                        internal_log_write("INFO", "Plugin function validation skipped: cached validation is valid and timestamp unchanged (plugin=%s, interface=%s, param_count=%d)", 
                                         plugin_path != NULL ? plugin_path : "unknown", interface_name != NULL ? interface_name : "unknown", expected_param_count);
                        return 0;
                    }
                }
            }
        }
    }
    
    pt_param_pack_t* test_pack = NULL;
    int32_t validation_result = 0;
    
    /* 完整检查：为所有参数数量创建测试参数包并执行实际调用测试 / Full check: create test param pack for all parameter counts and execute actual call test / Vollständige Prüfung: Testparameterpaket für alle Parameteranzahlen erstellen und tatsächlichen Aufruftest ausführen */
    if (expected_param_count == 0) {
        test_pack = pt_create_param_pack(0, NULL, NULL, NULL);
        if (test_pack == NULL) {
            internal_log_write("ERROR", "Plugin function validation failed: failed to create test param pack (0 params)");
            return -1;
        }
    } else {
        /* 动态分配测试参数数组，支持任意数量的参数 / Dynamically allocate test parameter arrays to support arbitrary parameter counts / Testparameter-Arrays dynamisch zuweisen, um beliebige Parameteranzahlen zu unterstützen */
        nxld_param_type_t* types = (nxld_param_type_t*)malloc(expected_param_count * sizeof(nxld_param_type_t));
        int32_t* values = (int32_t*)malloc(expected_param_count * sizeof(int32_t));
        void** value_ptrs = (void**)malloc(expected_param_count * sizeof(void*));
        size_t* sizes = (size_t*)malloc(expected_param_count * sizeof(size_t));
        
        if (types == NULL || values == NULL || value_ptrs == NULL || sizes == NULL) {
            internal_log_write("ERROR", "Plugin function validation failed: failed to allocate memory for test parameters (%d params)", expected_param_count);
            if (types != NULL) free(types);
            if (values != NULL) free(values);
            if (value_ptrs != NULL) free(value_ptrs);
            if (sizes != NULL) free(sizes);
            return -1;
        }
        
        /* 初始化测试参数：使用INT32类型和递增的测试值 / Initialize test parameters: use INT32 type and incrementing test values / Testparameter initialisieren: INT32-Typ und inkrementelle Testwerte verwenden */
        for (int i = 0; i < expected_param_count; i++) {
            types[i] = NXLD_PARAM_TYPE_INT32;
            values[i] = i + 1;
            value_ptrs[i] = &values[i];
            sizes[i] = sizeof(int32_t);
        }
        
        test_pack = pt_create_param_pack(expected_param_count, types, value_ptrs, sizes);
        
        /* 释放临时分配的内存 / Free temporarily allocated memory / Temporär zugewiesenen Speicher freigeben */
        free(types);
        free(values);
        free(value_ptrs);
        free(sizes);
        
        if (test_pack == NULL) {
            internal_log_write("ERROR", "Plugin function validation failed: failed to create test param pack (%d params)", expected_param_count);
            return -1;
        }
    }
    
    if (pt_validate_param_pack(test_pack) != 0) {
        internal_log_write("ERROR", "Plugin function validation failed: test param pack validation failed");
        pt_free_param_pack(test_pack);
        return -1;
    }
    
    int64_t test_result_int = 0;
    double test_result_float = 0.0;
    void* test_result_struct = NULL;
    
    /* 执行实际调用测试以验证函数兼容性 / Execute actual call test to verify function compatibility / Tatsächlichen Aufruftest ausführen, um Funktionskompatibilität zu überprüfen */
    int32_t call_result = pt_call_with_currying(func_ptr, test_pack, return_type, 0, 
                                                 &test_result_int, &test_result_float, test_result_struct);
    
    if (call_result != 0) {
        internal_log_write("ERROR", "Plugin function validation failed: test call returned error code %d (plugin=%s, interface=%s, param_count=%d)", 
                         call_result, plugin_path != NULL ? plugin_path : "unknown", interface_name != NULL ? interface_name : "unknown", expected_param_count);
        validation_result = -1;
    } else {
        internal_log_write("INFO", "Plugin function validation passed: test call succeeded (plugin=%s, interface=%s, param_count=%d)", 
                         plugin_path != NULL ? plugin_path : "unknown", interface_name != NULL ? interface_name : "unknown", expected_param_count);
        validation_result = 0;
    }
    
    pt_free_param_pack(test_pack);
    
    /* 生成.nxpv验证文件，记录时间戳和验证结果 / Generate .nxpv validation file, record timestamp and validation result / .nxpv-Validierungsdatei generieren, Zeitstempel und Validierungsergebnis aufzeichnen */
    if (plugin_path != NULL) {
        int64_t current_timestamp = 0;
        if (pt_platform_get_file_timestamp(plugin_path, &current_timestamp) == 0) {
            int is_valid = (validation_result == 0) ? 1 : 0;
            if (generate_nxpv_file(plugin_path, current_timestamp, is_valid) == 0) {
                internal_log_write("INFO", "Generated .nxpv validation file for plugin: %s (timestamp=%lld, valid=%d)", 
                                 plugin_path, (long long)current_timestamp, is_valid);
            } else {
                internal_log_write("WARNING", "Failed to generate .nxpv validation file for plugin: %s", plugin_path);
            }
        } else {
            internal_log_write("WARNING", "Failed to get file timestamp for plugin: %s", plugin_path);
        }
    }
    
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
 * @note 内部使用序列化机制确保数据自包含和安全性 / Uses serialization internally to ensure data self-containment and safety / Verwendet Serialisierung intern, um Daten-Selbständigkeit und Sicherheit zu gewährleisten
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
    
    /* 序列化参数包为连续内存块，确保数据自包含和安全性 / Serialize parameter pack to contiguous memory block to ensure data self-containment and safety / Parameterpaket in zusammenhängenden Speicherblock serialisieren, um Daten-Selbständigkeit und Sicherheit zu gewährleisten */
    void* serialized_data = pt_serialize_param_pack(pack);
    if (serialized_data == NULL) {
        internal_log_write("ERROR", "Call with currying failed: failed to serialize parameter pack");
        return -1;
    }
    
    /* 序列化后的数据格式与pt_param_pack_t兼容，可直接作为参数包使用 / Serialized data format is compatible with pt_param_pack_t and can be used directly as parameter pack / Serialisiertes Datenformat ist mit pt_param_pack_t kompatibel und kann direkt als Parameterpaket verwendet werden */
    pt_param_pack_t* serialized_pack = pt_deserialize_param_pack(serialized_data);
    if (serialized_pack == NULL) {
        internal_log_write("ERROR", "Call with currying failed: failed to deserialize parameter pack");
        pt_free_serialized_param_pack(serialized_data);
        return -1;
    }
    
    typedef int32_t (*curried_func_int_t)(void*);
    typedef float (*curried_func_float_t)(void*);
    typedef double (*curried_func_double_t)(void*);
    typedef int64_t (*curried_func_int64_t)(void*);
    typedef void* (*curried_func_ptr_t)(void*);
    
    int32_t call_result = 0;
    
    switch (return_type) {
        case PT_RETURN_TYPE_FLOAT: {
            curried_func_float_t f = (curried_func_float_t)func_ptr;
            *result_float = (double)f((void*)serialized_pack);
            *result_int = 0;
            break;
        }
        case PT_RETURN_TYPE_DOUBLE: {
            curried_func_double_t f = (curried_func_double_t)func_ptr;
            *result_float = f((void*)serialized_pack);
            *result_int = 0;
            break;
        }
        case PT_RETURN_TYPE_STRUCT_PTR: {
            curried_func_ptr_t f = (curried_func_ptr_t)func_ptr;
            void* struct_ptr = f((void*)serialized_pack);
            if (result_struct != NULL && return_size > 0 && struct_ptr != NULL) {
                memcpy(result_struct, struct_ptr, return_size);
            }
            *result_int = (int64_t)struct_ptr;
            *result_float = 0.0;
            break;
        }
        case PT_RETURN_TYPE_STRUCT_VAL: {
            typedef void (*curried_func_struct_val_t)(void*, void*);
            curried_func_struct_val_t f = (curried_func_struct_val_t)func_ptr;
            if (result_struct != NULL) {
                f((void*)serialized_pack, result_struct);
            }
            *result_int = 0;
            *result_float = 0.0;
            break;
        }
        default:
        {
            curried_func_int_t f = (curried_func_int_t)func_ptr;
            *result_int = (int64_t)f((void*)serialized_pack);
            *result_float = 0.0;
            break;
        }
    }
    
    /* 释放序列化的参数包（连续内存块） / Free serialized parameter pack (contiguous memory block) / Serialisiertes Parameterpaket freigeben (zusammenhängender Speicherblock) */
    pt_free_serialized_param_pack(serialized_data);
    
    return call_result;
}
