/**
 * @file pointer_transfer_plugin.c
 * @brief NXLD指针传递插件主实现 / NXLD Pointer Transfer Plugin Main Implementation / NXLD-Zeigerübertragungs-Plugin-Hauptimplementierung
 */

#include "nxld_plugin_interface.h"
#include "pointer_transfer_plugin.h"
#include "pointer_transfer_types.h"
#include "pointer_transfer_context.h"
#include "pointer_transfer_config.h"
#include "pointer_transfer_plugin_loader.h"
#include "pointer_transfer_interface.h"
#include "pointer_transfer_utils.h"
#include "pointer_transfer_platform.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

/* 插件常量定义 / Plugin constant definitions / Plugin-Konstantendefinitionen */
#define PLUGIN_NAME "PointerTransferPlugin"
#define PLUGIN_VERSION "1.2.0"
#define INTERFACE_COUNT 2

/* 接口名称和描述 / Interface names and descriptions / Schnittstellennamen und -beschreibungen */
static const char* interface_names[] = {
    "TransferPointer",
    "CallPlugin"
};

static const char* interface_descriptions[] = {
    "传递指针 / Transfer pointer / Zeiger übertragen",
    "调用目标插件接口 / Call target plugin interface / Ziel-Plugin-Schnittstelle aufrufen"
};

static const char* interface_versions[] = {
    "1.2.0",
    "1.2.0"
};

/* TransferPointer接口参数信息 / TransferPointer interface parameter information / TransferPointer-Schnittstellenparameterinformationen */
#define TRANSFERPOINTER_PARAM_COUNT 4
static const char* transferpointer_param_names[] = {
    "ptr",
    "expected_type",
    "type_name",
    "data_size"
};

static const nxld_param_type_t transferpointer_param_types[] = {
    NXLD_PARAM_TYPE_POINTER,
    NXLD_PARAM_TYPE_INT32,
    NXLD_PARAM_TYPE_STRING,
    NXLD_PARAM_TYPE_INT64
};

static const char* transferpointer_param_type_names[] = {
    "void*",
    "nxld_param_type_t",
    "const char*",
    "size_t"
};

/**
 * @brief 获取插件名称 / Get plugin name / Plugin-Namen abrufen
 */
NXLD_PLUGIN_EXPORT int32_t NXLD_PLUGIN_CALL nxld_plugin_get_name(char* name, size_t name_size) {
    if (name == NULL || name_size == 0) {
        return -1;
    }
    
    size_t len = strlen(PLUGIN_NAME);
    if (len >= name_size) {
        len = name_size - 1;
    }
    
    memcpy(name, PLUGIN_NAME, len);
    name[len] = '\0';
    
    return 0;
}

/**
 * @brief 获取插件版本 / Get plugin version / Plugin-Version abrufen
 */
NXLD_PLUGIN_EXPORT int32_t NXLD_PLUGIN_CALL nxld_plugin_get_version(char* version, size_t version_size) {
    if (version == NULL || version_size == 0) {
        return -1;
    }
    
    size_t len = strlen(PLUGIN_VERSION);
    if (len >= version_size) {
        len = version_size - 1;
    }
    
    memcpy(version, PLUGIN_VERSION, len);
    version[len] = '\0';
    
    return 0;
}

/**
 * @brief 获取接口数量 / Get interface count / Schnittstellenanzahl abrufen
 */
NXLD_PLUGIN_EXPORT int32_t NXLD_PLUGIN_CALL nxld_plugin_get_interface_count(size_t* count) {
    if (count == NULL) {
        return -1;
    }
    
    *count = INTERFACE_COUNT;
    return 0;
}

/**
 * @brief 获取接口信息 / Get interface information / Schnittstelleninformationen abrufen
 */
NXLD_PLUGIN_EXPORT int32_t NXLD_PLUGIN_CALL nxld_plugin_get_interface_info(size_t index, 
                                                        char* name, size_t name_size,
                                                        char* description, size_t desc_size,
                                                        char* version, size_t version_size) {
    if (index >= INTERFACE_COUNT) {
        return -1;
    }
    
    if (name != NULL && name_size > 0) {
        size_t len = strlen(interface_names[index]);
        if (len >= name_size) {
            len = name_size - 1;
        }
        memcpy(name, interface_names[index], len);
        name[len] = '\0';
    }
    
    if (description != NULL && desc_size > 0) {
        size_t len = strlen(interface_descriptions[index]);
        if (len >= desc_size) {
            len = desc_size - 1;
        }
        memcpy(description, interface_descriptions[index], len);
        description[len] = '\0';
    }
    
    if (version != NULL && version_size > 0) {
        size_t len = strlen(interface_versions[index]);
        if (len >= version_size) {
            len = version_size - 1;
        }
        memcpy(version, interface_versions[index], len);
        version[len] = '\0';
    }
    
    return 0;
}

/**
 * @brief 获取接口参数数量 / Get interface parameter count / Schnittstellenparameteranzahl abrufen
 */
NXLD_PLUGIN_EXPORT int32_t NXLD_PLUGIN_CALL nxld_plugin_get_interface_param_count(size_t index,
                                                               nxld_param_count_type_t* count_type,
                                                               int32_t* min_count, int32_t* max_count) {
    if (index >= INTERFACE_COUNT || count_type == NULL || min_count == NULL || max_count == NULL) {
        return -1;
    }
    
    if (index == 0) {
        *count_type = NXLD_PARAM_COUNT_FIXED;
        *min_count = (int32_t)TRANSFERPOINTER_PARAM_COUNT;
        *max_count = (int32_t)TRANSFERPOINTER_PARAM_COUNT;
    } else if (index == 1) {
        *count_type = NXLD_PARAM_COUNT_FIXED;
        *min_count = 4;
        *max_count = 4;
    } else {
        return -1;
    }
    
    return 0;
}

/**
 * @brief 获取接口参数信息 / Get interface parameter information / Schnittstellenparameterinformationen abrufen
 */
NXLD_PLUGIN_EXPORT int32_t NXLD_PLUGIN_CALL nxld_plugin_get_interface_param_info(size_t index, int32_t param_index,
                                                              char* param_name, size_t name_size,
                                                              nxld_param_type_t* param_type,
                                                              char* type_name, size_t type_name_size) {
    if (index >= INTERFACE_COUNT || param_name == NULL || name_size == 0 || param_type == NULL) {
        return -1;
    }
    
    if (index == 0) {
        if (param_index < 0 || param_index >= (int32_t)TRANSFERPOINTER_PARAM_COUNT) {
            return -1;
        }
        
        size_t len = strlen(transferpointer_param_names[param_index]);
        if (len >= name_size) {
            len = name_size - 1;
        }
        memcpy(param_name, transferpointer_param_names[param_index], len);
        param_name[len] = '\0';
        
        *param_type = transferpointer_param_types[param_index];
        
        if (type_name != NULL && type_name_size > 0) {
            len = strlen(transferpointer_param_type_names[param_index]);
            if (len >= type_name_size) {
                len = type_name_size - 1;
            }
            memcpy(type_name, transferpointer_param_type_names[param_index], len);
            type_name[len] = '\0';
        }
    } else if (index == 1) {
        static const char* callplugin_param_names[] = {
            "source_plugin_name",
            "source_interface_name",
            "param_index",
            "param_value"
        };
        static const nxld_param_type_t callplugin_param_types[] = {
            NXLD_PARAM_TYPE_STRING,
            NXLD_PARAM_TYPE_STRING,
            NXLD_PARAM_TYPE_INT32,
            NXLD_PARAM_TYPE_POINTER
        };
        static const char* callplugin_param_type_names[] = {
            "const char*",
            "const char*",
            "int",
            "void*"
        };
        
        if (param_index < 0 || param_index >= 4) {
            return -1;
        }
        
        size_t len = strlen(callplugin_param_names[param_index]);
        if (len >= name_size) {
            len = name_size - 1;
        }
        memcpy(param_name, callplugin_param_names[param_index], len);
        param_name[len] = '\0';
        
        *param_type = callplugin_param_types[param_index];
        
        if (type_name != NULL && type_name_size > 0) {
            len = strlen(callplugin_param_type_names[param_index]);
            if (len >= type_name_size) {
                len = type_name_size - 1;
            }
            memcpy(type_name, callplugin_param_type_names[param_index], len);
            type_name[len] = '\0';
        }
    } else {
        return -1;
    }
    
    return 0;
}

/**
 * @brief 传递指针 / Transfer pointer / Zeiger übertragen
 * @param ptr 指针 / Pointer / Zeiger
 * @param expected_type 数据类型 / Data type / Datentyp
 * @param type_name 类型名称 / Type name / Typname
 * @param data_size 数据大小 / Data size / Datengröße
 * @return 成功返回0，类型不匹配返回1，其他错误返回-1 / Returns 0 on success, 1 on type mismatch, -1 on other errors / Gibt 0 bei Erfolg zurück, 1 bei Typfehlanpassung, -1 bei anderen Fehlern
 */
POINTER_TRANSFER_PLUGIN_EXPORT int POINTER_TRANSFER_PLUGIN_CALL TransferPointer(void* ptr, nxld_param_type_t expected_type, const char* type_name, size_t data_size) {
    if (ptr == NULL) {
        internal_log_write("WARNING", "TransferPointer: received NULL pointer");
        return -1;
    }
    
    pointer_transfer_context_t* ctx = get_global_context();
    int type_mismatch = 0;
    
    if (ctx->stored_ptr != NULL && ctx->stored_ptr == ptr) {
        nxld_param_type_t stored_type = ctx->stored_type;
        
        if (!check_type_compatibility(stored_type, expected_type)) {
            const char* stored_type_str = get_type_name_string(stored_type);
            const char* expected_type_str = get_type_name_string(expected_type);
            const char* stored_type_name = ctx->stored_type_name != NULL ? ctx->stored_type_name : "unknown";
            const char* expected_type_name = type_name != NULL ? type_name : "unknown";
            
            internal_log_write("WARNING", "TransferPointer: type mismatch detected for pointer %p - stored: %s (%s), expected: %s (%s)", 
                           ptr, stored_type_str, stored_type_name, expected_type_str, expected_type_name);
            type_mismatch = 1;
        }
        
        if (data_size > 0 && ctx->stored_size > 0 && data_size != ctx->stored_size) {
            internal_log_write("WARNING", "TransferPointer: size mismatch detected for pointer %p - stored: %zu, expected: %zu", 
                           ptr, ctx->stored_size, data_size);
            type_mismatch = 1;
        }
    }
    
    ctx->stored_ptr = ptr;
    ctx->stored_type = expected_type;
    ctx->stored_size = data_size;
    
    if (ctx->stored_type_name != NULL) {
        free(ctx->stored_type_name);
        ctx->stored_type_name = NULL;
    }
    
    if (type_name != NULL && strlen(type_name) > 0) {
        size_t type_name_len = strlen(type_name);
        ctx->stored_type_name = (char*)malloc(type_name_len + 1);
        if (ctx->stored_type_name != NULL) {
            memcpy(ctx->stored_type_name, type_name, type_name_len);
            ctx->stored_type_name[type_name_len] = '\0';
        }
    }
    
    if (type_mismatch) {
        internal_log_write("INFO", "TransferPointer: pointer transferred with type mismatch warning - type: %s (%s), size: %zu", 
                      get_type_name_string(expected_type), type_name != NULL ? type_name : "unknown", data_size);
        return 1;
    }
    
    internal_log_write("INFO", "TransferPointer: pointer transferred successfully - type: %s (%s), size: %zu", 
                  get_type_name_string(expected_type), type_name != NULL ? type_name : "unknown", data_size);
    
    if (ctx->rule_count > 0 && ctx->rules != NULL) {
        size_t matched_count = 0;
        size_t success_count = 0;
        
        /* 使用索引快速查找匹配规则 / Use index to quickly find matching rules / Index verwenden, um passende Regeln schnell zu finden */
        size_t start_index = 0;
        size_t end_index = 0;
        int use_index = find_rule_index_range("PointerTransferPlugin", "TransferPointer", 0, &start_index, &end_index);
        
        if (use_index) {
            /* 使用索引范围查找 / Use index range lookup / Indexbereichssuche verwenden */
            for (size_t i = start_index; i <= end_index && i < ctx->rule_count; i++) {
                pointer_transfer_rule_t* rule = &ctx->rules[i];
                if (!rule->enabled) {
                    continue;
                }
                
                if (rule->source_interface != NULL && strcmp(rule->source_interface, "TransferPointer") == 0 &&
                    rule->source_param_index == 0) {
                    int should_apply = 0;
                    
                    if (rule->transfer_mode == TRANSFER_MODE_BROADCAST) {
                        should_apply = 1;
                    } else if (rule->transfer_mode == TRANSFER_MODE_MULTICAST) {
                        if (rule->multicast_group != NULL && strlen(rule->multicast_group) > 0) {
                            should_apply = 1;
                        }
                    } else {
                        should_apply = 1;
                    }
                    
                    if (should_apply) {
                        matched_count++;
                        internal_log_write("INFO", "Applying transfer rule %zu (mode=%d) - %s to %s.%s[%d]", 
                                    i, (int)rule->transfer_mode,
                                    rule->description != NULL ? rule->description : "unnamed",
                                    rule->target_plugin != NULL ? rule->target_plugin : "unknown",
                                    rule->target_interface != NULL ? rule->target_interface : "unknown",
                                    rule->target_param_index);
                        
                        int call_result = call_target_plugin_interface(rule, ptr);
                        if (call_result == 0) {
                            success_count++;
                            internal_log_write("INFO", "Successfully called target plugin interface");
                        } else {
                            internal_log_write("WARNING", "Failed to call target plugin interface (error=%d)", call_result);
                        }
                        
                        if (rule->transfer_mode == TRANSFER_MODE_UNICAST) {
                            break;
                        }
                    }
                }
            }
        } else {
            /* 回退到线性搜索 / Fallback to linear search / Fallback auf lineare Suche */
            for (size_t i = 0; i < ctx->rule_count; i++) {
                pointer_transfer_rule_t* rule = &ctx->rules[i];
                if (!rule->enabled) {
                    continue;
                }
                
                if (rule->source_interface != NULL && strcmp(rule->source_interface, "TransferPointer") == 0) {
                    if (rule->source_param_index == 0) {
                        int should_apply = 0;
                        
                        if (rule->transfer_mode == TRANSFER_MODE_BROADCAST) {
                            should_apply = 1;
                        } else if (rule->transfer_mode == TRANSFER_MODE_MULTICAST) {
                            if (rule->multicast_group != NULL && strlen(rule->multicast_group) > 0) {
                                should_apply = 1;
                            }
                        } else {
                            should_apply = 1;
                        }
                        
                        if (should_apply) {
                            matched_count++;
                            internal_log_write("INFO", "Applying transfer rule %zu (mode=%d) - %s to %s.%s[%d]", 
                                        i, (int)rule->transfer_mode,
                                        rule->description != NULL ? rule->description : "unnamed",
                                        rule->target_plugin != NULL ? rule->target_plugin : "unknown",
                                        rule->target_interface != NULL ? rule->target_interface : "unknown",
                                        rule->target_param_index);
                            
                            int call_result = call_target_plugin_interface(rule, ptr);
                            if (call_result == 0) {
                                success_count++;
                                internal_log_write("INFO", "Successfully called target plugin interface");
                            } else {
                                internal_log_write("WARNING", "Failed to call target plugin interface (error=%d)", call_result);
                            }
                            
                            if (rule->transfer_mode == TRANSFER_MODE_UNICAST) {
                                break;
                            }
                        }
                    }
                }
            }
        }
        
        if (matched_count > 0) {
            internal_log_write("INFO", "Processed %zu rules, %zu successful", matched_count, success_count);
        }
    }
    
    return 0;
}

/**
 * @brief 调用目标插件接口 / Call target plugin interface / Ziel-Plugin-Schnittstelle aufrufen
 * @param source_plugin_name 源插件名称 / Source plugin name / Quell-Plugin-Name
 * @param source_interface_name 源接口名称 / Source interface name / Quell-Schnittstellenname
 * @param param_index 参数索引 / Parameter index / Parameterindex
 * @param param_value 参数值 / Parameter value / Parameterwert
 * @return 成功返回0，失败返回非0 / Returns 0 on success, non-zero on failure / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
 */
POINTER_TRANSFER_PLUGIN_EXPORT int POINTER_TRANSFER_PLUGIN_CALL CallPlugin(const char* source_plugin_name, const char* source_interface_name, int param_index, void* param_value) {
    if (source_plugin_name == NULL || source_interface_name == NULL) {
        internal_log_write("WARNING", "CallPlugin: invalid parameters");
        return -1;
    }
    
    internal_log_write("INFO", "CallPlugin: called with source_plugin=%s, source_interface=%s, param_index=%d", 
                  source_plugin_name, source_interface_name, param_index);
    
    pointer_transfer_context_t* ctx = get_global_context();
    size_t matched_count = 0;
    size_t success_count = 0;
    
    if (ctx->rule_count > 0 && ctx->rules != NULL) {
        /* 使用索引快速定位匹配规则 / Use index to quickly locate matching rules / Index verwenden, um passende Regeln schnell zu lokalisieren */
        size_t start_index = 0;
        size_t end_index = 0;
        int use_index = find_rule_index_range(source_plugin_name, source_interface_name, param_index, &start_index, &end_index);
        
        if (use_index) {
            /* 使用索引范围查找 / Use index range lookup / Indexbereichssuche verwenden */
            /* BROADCAST和MULTICAST规则 / BROADCAST and MULTICAST rules / BROADCAST- und MULTICAST-Regeln */
            for (size_t i = start_index; i <= end_index && i < ctx->rule_count; i++) {
                pointer_transfer_rule_t* rule = &ctx->rules[i];
                if (!rule->enabled) {
                    continue;
                }
                
                if (rule->source_plugin != NULL && rule->source_interface != NULL) {
                    if (strcmp(rule->source_plugin, source_plugin_name) == 0 &&
                        strcmp(rule->source_interface, source_interface_name) == 0 &&
                        rule->source_param_index == param_index) {
                        
                        internal_log_write("INFO", "Rule %zu matched: source=%s.%s[%d], mode=%d, target=%s.%s[%d]", 
                            i, source_plugin_name, source_interface_name, param_index,
                            (int)rule->transfer_mode,
                            rule->target_plugin != NULL ? rule->target_plugin : "unknown",
                            rule->target_interface != NULL ? rule->target_interface : "unknown",
                            rule->target_param_index);
                        
                        if (rule->transfer_mode == TRANSFER_MODE_BROADCAST || rule->transfer_mode == TRANSFER_MODE_MULTICAST) {
                            if (rule->transfer_mode == TRANSFER_MODE_MULTICAST) {
                                if (rule->multicast_group == NULL || strlen(rule->multicast_group) == 0) {
                                    continue;
                                }
                            }
                            
                            if (!check_condition(rule->condition, param_value)) {
                                internal_log_write("INFO", "Transfer rule %zu condition not met, skipping - condition: %s", 
                                            i, rule->condition != NULL ? rule->condition : "none");
                                continue;
                            }
                            
                            matched_count++;
                            internal_log_write("INFO", "Applying transfer rule %zu (mode=%d) - %s.%s[%d] to %s.%s[%d]", 
                                        i, (int)rule->transfer_mode,
                                        source_plugin_name, source_interface_name, param_index,
                                        rule->target_plugin != NULL ? rule->target_plugin : "unknown",
                                        rule->target_interface != NULL ? rule->target_interface : "unknown",
                                        rule->target_param_index);
                            
                            int call_result = call_target_plugin_interface(rule, param_value);
                            if (call_result == 0) {
                                success_count++;
                                internal_log_write("INFO", "Successfully called target plugin interface");
                            } else {
                                internal_log_write("WARNING", "Failed to call target plugin interface (error=%d)", call_result);
                            }
                        }
                    }
                }
            }
            
            /* UNICAST规则 / UNICAST rules / UNICAST-Regeln */
            /* UNICAST模式：允许传递给同一个接口的不同参数索引，只有当找到完全相同的目标位置时才停止 / UNICAST mode: allow passing to different parameter indices of the same interface, only stop when exact duplicate target is found / UNICAST-Modus: Übergabe an verschiedene Parameterindizes derselben Schnittstelle zulassen, nur stoppen, wenn exaktes doppeltes Ziel gefunden wird */
            for (size_t i = start_index; i <= end_index && i < ctx->rule_count; i++) {
                pointer_transfer_rule_t* rule = &ctx->rules[i];
                if (!rule->enabled) {
                    continue;
                }
                
                if (rule->source_plugin != NULL && rule->source_interface != NULL) {
                    if (strcmp(rule->source_plugin, source_plugin_name) == 0 &&
                        strcmp(rule->source_interface, source_interface_name) == 0 &&
                        rule->source_param_index == param_index) {
                        
                        if (rule->transfer_mode == TRANSFER_MODE_UNICAST) {
                            if (!check_condition(rule->condition, param_value)) {
                                internal_log_write("INFO", "Transfer rule %zu condition not met, skipping - condition: %s", 
                                            i, rule->condition != NULL ? rule->condition : "none");
                                continue;
                            }
                            
                            matched_count++;
                            internal_log_write("INFO", "Applying transfer rule %zu (mode=%d) - %s.%s[%d] to %s.%s[%d]", 
                                        i, (int)rule->transfer_mode,
                                        source_plugin_name, source_interface_name, param_index,
                                        rule->target_plugin != NULL ? rule->target_plugin : "unknown",
                                        rule->target_interface != NULL ? rule->target_interface : "unknown",
                                        rule->target_param_index);
                            
                            int call_result = call_target_plugin_interface(rule, param_value);
                            if (call_result == 0) {
                                success_count++;
                                internal_log_write("INFO", "Successfully called target plugin interface");
                            } else {
                                internal_log_write("WARNING", "Failed to call target plugin interface (error=%d)", call_result);
                            }
                            
                            /* 检查是否有其他规则匹配完全相同的目标位置（插件+接口+参数索引） / Check if other rules match the exact same target location (plugin+interface+parameter index) / Prüfen, ob andere Regeln exakt dieselbe Zielposition (Plugin+Schnittstelle+Parameterindex) abgleichen */
                            int has_exact_duplicate = 0;
                            for (size_t j = i + 1; j <= end_index && j < ctx->rule_count; j++) {
                                pointer_transfer_rule_t* next_rule = &ctx->rules[j];
                                if (!next_rule->enabled || next_rule->source_plugin == NULL || next_rule->source_interface == NULL) {
                                    continue;
                                }
                                if (strcmp(next_rule->source_plugin, source_plugin_name) == 0 &&
                                    strcmp(next_rule->source_interface, source_interface_name) == 0 &&
                                    next_rule->source_param_index == param_index &&
                                    next_rule->target_plugin != NULL && next_rule->target_interface != NULL &&
                                    rule->target_plugin != NULL && rule->target_interface != NULL &&
                                    strcmp(next_rule->target_plugin, rule->target_plugin) == 0 &&
                                    strcmp(next_rule->target_interface, rule->target_interface) == 0 &&
                                    next_rule->target_param_index == rule->target_param_index) {
                                    has_exact_duplicate = 1;
                                    break;
                                }
                            }
                            /* 只有在找到完全相同的目标位置时才停止，允许传递给同一接口的不同参数 / Only stop when exact duplicate target is found, allow passing to different parameters of the same interface / Nur stoppen, wenn exaktes doppeltes Ziel gefunden wird, Übergabe an verschiedene Parameter derselben Schnittstelle zulassen */
                            if (has_exact_duplicate) {
                                break;
                            }
                        }
                    }
                }
            }
        } else {
            /* 回退到线性搜索 / Fallback to linear search / Fallback auf lineare Suche */
            /* BROADCAST和MULTICAST规则 / BROADCAST and MULTICAST rules / BROADCAST- und MULTICAST-Regeln */
            for (size_t i = 0; i < ctx->rule_count; i++) {
                pointer_transfer_rule_t* rule = &ctx->rules[i];
                if (!rule->enabled) {
                    continue;
                }
                
                if (rule->source_plugin != NULL && rule->source_interface != NULL) {
                    if (strcmp(rule->source_plugin, source_plugin_name) == 0 &&
                        strcmp(rule->source_interface, source_interface_name) == 0 &&
                        rule->source_param_index == param_index) {
                        
                        internal_log_write("INFO", "Rule %zu matched: source=%s.%s[%d], mode=%d, target=%s.%s[%d]", 
                            i, source_plugin_name, source_interface_name, param_index,
                            (int)rule->transfer_mode,
                            rule->target_plugin != NULL ? rule->target_plugin : "unknown",
                            rule->target_interface != NULL ? rule->target_interface : "unknown",
                            rule->target_param_index);
                        
                        if (rule->transfer_mode == TRANSFER_MODE_BROADCAST || rule->transfer_mode == TRANSFER_MODE_MULTICAST) {
                            if (rule->transfer_mode == TRANSFER_MODE_MULTICAST) {
                                if (rule->multicast_group == NULL || strlen(rule->multicast_group) == 0) {
                                    continue;
                                }
                            }
                            
                            if (!check_condition(rule->condition, param_value)) {
                                internal_log_write("INFO", "Transfer rule %zu condition not met, skipping - condition: %s", 
                                            i, rule->condition != NULL ? rule->condition : "none");
                                continue;
                            }
                            
                            matched_count++;
                            internal_log_write("INFO", "Applying transfer rule %zu (mode=%d) - %s.%s[%d] to %s.%s[%d]", 
                                        i, (int)rule->transfer_mode,
                                        source_plugin_name, source_interface_name, param_index,
                                        rule->target_plugin != NULL ? rule->target_plugin : "unknown",
                                        rule->target_interface != NULL ? rule->target_interface : "unknown",
                                        rule->target_param_index);
                            
                            int call_result = call_target_plugin_interface(rule, param_value);
                            if (call_result == 0) {
                                success_count++;
                                internal_log_write("INFO", "Successfully called target plugin interface");
                            } else {
                                internal_log_write("WARNING", "Failed to call target plugin interface (error=%d)", call_result);
                            }
                        }
                    }
                }
            }
            
            /* UNICAST规则 / UNICAST rules / UNICAST-Regeln */
            /* UNICAST模式：允许传递给同一个接口的不同参数索引，只有当找到完全相同的目标位置时才停止 / UNICAST mode: allow passing to different parameter indices of the same interface, only stop when exact duplicate target is found / UNICAST-Modus: Übergabe an verschiedene Parameterindizes derselben Schnittstelle zulassen, nur stoppen, wenn exaktes doppeltes Ziel gefunden wird */
            for (size_t i = 0; i < ctx->rule_count; i++) {
                pointer_transfer_rule_t* rule = &ctx->rules[i];
                if (!rule->enabled) {
                    continue;
                }
                
                if (rule->source_plugin != NULL && rule->source_interface != NULL) {
                    if (strcmp(rule->source_plugin, source_plugin_name) == 0 &&
                        strcmp(rule->source_interface, source_interface_name) == 0 &&
                        rule->source_param_index == param_index) {
                        
                        if (rule->transfer_mode == TRANSFER_MODE_UNICAST) {
                            if (!check_condition(rule->condition, param_value)) {
                                internal_log_write("INFO", "Transfer rule %zu condition not met, skipping - condition: %s", 
                                            i, rule->condition != NULL ? rule->condition : "none");
                                continue;
                            }
                            
                            matched_count++;
                            internal_log_write("INFO", "Applying transfer rule %zu (mode=%d) - %s.%s[%d] to %s.%s[%d]", 
                                        i, (int)rule->transfer_mode,
                                        source_plugin_name, source_interface_name, param_index,
                                        rule->target_plugin != NULL ? rule->target_plugin : "unknown",
                                        rule->target_interface != NULL ? rule->target_interface : "unknown",
                                        rule->target_param_index);
                            
                            int call_result = call_target_plugin_interface(rule, param_value);
                            if (call_result == 0) {
                                success_count++;
                                internal_log_write("INFO", "Successfully called target plugin interface");
                            } else {
                                internal_log_write("WARNING", "Failed to call target plugin interface (error=%d)", call_result);
                            }
                            
                            /* 检查是否有其他规则匹配完全相同的目标位置（插件+接口+参数索引） / Check if other rules match the exact same target location (plugin+interface+parameter index) / Prüfen, ob andere Regeln exakt dieselbe Zielposition (Plugin+Schnittstelle+Parameterindex) abgleichen */
                            int has_exact_duplicate = 0;
                            for (size_t j = i + 1; j < ctx->rule_count; j++) {
                                pointer_transfer_rule_t* next_rule = &ctx->rules[j];
                                if (!next_rule->enabled || next_rule->source_plugin == NULL || next_rule->source_interface == NULL) {
                                    continue;
                                }
                                if (strcmp(next_rule->source_plugin, source_plugin_name) == 0 &&
                                    strcmp(next_rule->source_interface, source_interface_name) == 0 &&
                                    next_rule->source_param_index == param_index &&
                                    next_rule->target_plugin != NULL && next_rule->target_interface != NULL &&
                                    rule->target_plugin != NULL && rule->target_interface != NULL &&
                                    strcmp(next_rule->target_plugin, rule->target_plugin) == 0 &&
                                    strcmp(next_rule->target_interface, rule->target_interface) == 0 &&
                                    next_rule->target_param_index == rule->target_param_index) {
                                    has_exact_duplicate = 1;
                                    break;
                                }
                            }
                            /* 只有在找到完全相同的目标位置时才停止，允许传递给同一接口的不同参数 / Only stop when exact duplicate target is found, allow passing to different parameters of the same interface / Nur stoppen, wenn exaktes doppeltes Ziel gefunden wird, Übergabe an verschiedene Parameter derselben Schnittstelle zulassen */
                            if (has_exact_duplicate) {
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    
    if (matched_count == 0) {
        internal_log_write("WARNING", "CallPlugin: no matching rule found for %s.%s[%d]. Transfer rules must be configured in .nxpt file", source_plugin_name, source_interface_name, param_index);
        return -1;
    }
    
    internal_log_write("INFO", "CallPlugin: processed %zu rules, %zu successful", matched_count, success_count);
    return (success_count > 0) ? 0 : -1;
}

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
            /* 加载调度器配置文件本身的规则 / Load scheduler config file's own rules / Eigene Regeln der Scheduler-Konfigurationsdatei laden */
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
