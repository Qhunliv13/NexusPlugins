/**
 * @file pointer_transfer_types.h
 * @brief 指针传递插件类型定义 / Pointer Transfer Plugin Type Definitions / Zeigerübertragungs-Plugin-Typdefinitionen
 */

#ifndef POINTER_TRANSFER_TYPES_H
#define POINTER_TRANSFER_TYPES_H

#include "nxld_plugin_interface.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 返回值类型枚举 / Return value type enumeration / Rückgabewerttyp-Aufzählung
 */
typedef enum {
    PT_RETURN_TYPE_INTEGER = 0,    /**< 整数/指针返回值（RAX） / Integer/pointer return value (RAX) / Integer/Zeiger-Rückgabewert (RAX) */
    PT_RETURN_TYPE_FLOAT,          /**< 浮点返回值（XMM0） / Floating-point return value (XMM0) / Gleitkomma-Rückgabewert (XMM0) */
    PT_RETURN_TYPE_DOUBLE,         /**< 双精度返回值（XMM0） / Double return value (XMM0) / Double-Rückgabewert (XMM0) */
    PT_RETURN_TYPE_STRUCT_PTR,     /**< 小结构体返回值（通过指针，RAX包含指针，Windows: <=8字节，Linux: <=16字节） / Small struct return value (via pointer, RAX contains pointer, Windows: <=8 bytes, Linux: <=16 bytes) / Kleine Struktur-Rückgabewert (über Zeiger, RAX enthält Zeiger, Windows: <=8 Bytes, Linux: <=16 Bytes) */
    PT_RETURN_TYPE_STRUCT_VAL      /**< 大结构体返回值（值返回，通过隐藏指针参数，Windows: >8字节，Linux: >16字节） / Large struct return value (value return via hidden pointer parameter, Windows: >8 bytes, Linux: >16 bytes) / Große Struktur-Rückgabewert (Wertrückgabe über versteckten Zeigerparameter, Windows: >8 Bytes, Linux: >16 Bytes) */
} pt_return_type_t;

/**
 * @brief 传递模式枚举 / Transfer mode enumeration / Übertragungsmodus-Aufzählung
 */
typedef enum {
    TRANSFER_MODE_UNICAST = 0,    /**< 单播模式 / Unicast mode / Unicast-Modus */
    TRANSFER_MODE_BROADCAST,       /**< 广播模式 / Broadcast mode / Broadcast-Modus */
    TRANSFER_MODE_MULTICAST        /**< 组播模式 / Multicast mode / Multicast-Modus */
} transfer_mode_t;

/**
 * @brief 指针传递规则结构体 / Pointer transfer rule structure / Zeigerübertragungsregel-Struktur
 */
typedef struct {
    char* source_plugin;          /**< 源插件名称 / Source plugin name / Quell-Plugin-Name */
    char* source_interface;       /**< 源接口名称 / Source interface name / Quell-Schnittstellenname */
    int source_param_index;       /**< 源参数索引 / Source parameter index / Quell-Parameterindex */
    char* target_plugin;          /**< 目标插件名称 / Target plugin name / Ziel-Plugin-Name */
    char* target_plugin_path;     /**< 目标插件路径 / Target plugin path / Ziel-Plugin-Pfad */
    char* target_interface;       /**< 目标接口名称 / Target interface name / Ziel-Schnittstellenname */
    int target_param_index;       /**< 目标参数索引 / Target parameter index / Ziel-Parameterindex */
    char* target_param_value;    /**< 目标参数常量值 / Target parameter constant value / Ziel-Parameter-Konstantenwert */
    char* description;           /**< 规则描述 / Rule description / Regelbeschreibung */
    char* multicast_group;        /**< 组播组名称 / Multicast group name / Multicast-Gruppenname */
    transfer_mode_t transfer_mode; /**< 传递模式 / Transfer mode / Übertragungsmodus */
    int enabled;                  /**< 启用标志 / Enabled flag / Aktivierungsflag */
    char* condition;              /**< 传递条件 / Transfer condition / Übertragungsbedingung */
    int cache_self;               /**< 缓存自身规则标志 / Cache self rule flag / Selbst-Regel-Cache-Flag */
    char* set_group;              /**< 设置组名称 / Set group name / Set-Gruppenname */
} pointer_transfer_rule_t;

/**
 * @brief 已加载的插件信息结构体 / Loaded plugin information structure / Geladenes Plugin-Informationsstruktur
 */
typedef struct {
    void* handle;                 /**< 动态库句柄 / Dynamic library handle / Dynamisches Bibliothekshandle */
    char* plugin_name;            /**< 插件名称 / Plugin name / Plugin-Name */
    char* plugin_path;            /**< 插件路径 / Plugin path / Plugin-Pfad */
} loaded_plugin_info_t;

/**
 * @brief 目标接口参数状态结构体 / Target interface parameter state structure / Ziel-Schnittstellenparameter-Statusstruktur
 */
typedef struct {
    char* plugin_name;            /**< 插件名称 / Plugin name / Plugin-Name */
    char* interface_name;          /**< 接口名称 / Interface name / Schnittstellenname */
    void* handle;                  /**< 插件句柄 / Plugin handle / Plugin-Handle */
    void* func_ptr;                /**< 函数指针 / Function pointer / Funktionszeiger */
    int param_count;              /**< 参数数量 / Parameter count / Parameteranzahl */
    int* param_ready;             /**< 参数就绪标志数组 / Parameter ready flags array / Parameterbereitschafts-Flag-Array */
    void** param_values;          /**< 参数值数组 / Parameter values array / Parameterwerte-Array */
    nxld_param_type_t* param_types; /**< 参数类型数组 / Parameter types array / Parametertyp-Array */
    size_t* param_sizes;          /**< 参数大小数组 / Parameter sizes array / Parametergrößen-Array */
    int64_t* param_int_values;    /**< 参数INT32/INT64常量值数组 / Parameter INT32/INT64 constant values array / Parameter INT32/INT64-Konstantenwerte-Array */
    double* param_float_values;    /**< 参数FLOAT/DOUBLE常量值数组 / Parameter FLOAT/DOUBLE constant values array / Parameter FLOAT/DOUBLE-Konstantenwerte-Array */
    int is_variadic;               /**< 可变参数接口标志 / Variadic interface flag / Variabler Parameter-Interface-Flag */
    int min_param_count;           /**< 最小参数数量（用于可变参数接口） / Minimum parameter count (for variadic interfaces) / Mindestparameteranzahl (für variabler Parameter-Interfaces) */
    int actual_param_count;        /**< 实际参数数量 / Actual parameter count / Tatsächliche Parameteranzahl */
    pt_return_type_t return_type;  /**< 返回值类型 / Return value type / Rückgabewerttyp */
    size_t return_size;            /**< 返回值大小 / Return value size / Rückgabewertgröße */
    int in_use;                   /**< 使用中标志 / In use flag / In-Verwendung-Flag */
    int validation_done;          /**< 验证完成标志 / Validation done flag / Validierungs-Flag */
} target_interface_state_t;

/**
 * @brief 哈希表桶节点结构体 / Hash table bucket node structure / Hash-Tabelle-Bucket-Knoten-Struktur
 */
typedef struct rule_hash_node_s {
    uint64_t hash_key;            /**< 哈希键（整数） / Hash key (integer) / Hash-Schlüssel (Ganzzahl) */
    size_t rule_index;            /**< 规则索引 / Rule index / Regelindex */
    struct rule_hash_node_s* next; /**< 下一个节点（链式冲突解决） / Next node (chaining collision resolution) / Nächster Knoten (Verkettungskollisionsauflösung) */
} rule_hash_node_t;

/**
 * @brief 规则哈希表结构体 / Rule hash table structure / Regel-Hash-Tabelle-Struktur
 */
typedef struct {
    rule_hash_node_t** buckets;   /**< 桶数组 / Bucket array / Bucket-Array */
    size_t bucket_count;          /**< 桶数量 / Bucket count / Bucket-Anzahl */
    size_t entry_count;           /**< 条目数量 / Entry count / Eintragsanzahl */
} rule_hash_table_t;

/**
 * @brief 插件路径缓存项结构体 / Plugin path cache entry structure / Plugin-Pfad-Cache-Eintrag-Struktur
 */
typedef struct {
    char* plugin_name;            /**< 插件名称 / Plugin name / Plugin-Name */
    char* plugin_path;            /**< 插件路径 / Plugin path / Plugin-Pfad */
} plugin_path_cache_entry_t;

/**
 * @brief 已加载的.nxpt文件信息结构体 / Loaded .nxpt file information structure / Geladene .nxpt-Datei-Informationsstruktur
 */
typedef struct {
    char* plugin_name;            /**< 插件名称 / Plugin name / Plugin-Name */
    char* nxpt_path;              /**< .nxpt文件路径 / .nxpt file path / .nxpt-Dateipfad */
    int loaded;                   /**< 已加载标志 / Loaded flag / Geladenes Flag */
} loaded_nxpt_info_t;

/**
 * @brief 指针传输上下文结构体 / Pointer transfer context structure / Zeigerübertragungskontext-Struktur
 */
typedef struct {
    void* stored_ptr;             /**< 存储的指针 / Stored pointer / Gespeicherter Zeiger */
    nxld_param_type_t stored_type; /**< 存储的类型 / Stored type / Gespeicherter Typ */
    char* stored_type_name;      /**< 存储的类型名称 / Stored type name / Gespeicherter Typname */
    size_t stored_size;           /**< 存储的数据大小 / Stored data size / Gespeicherte Datengröße */
    pointer_transfer_rule_t* rules; /**< 传递规则数组 / Transfer rules array / Übertragungsregel-Array */
    size_t rule_count;            /**< 规则数量 / Rule count / Regelanzahl */
    size_t rule_capacity;        /**< 规则数组容量 / Rule array capacity / Regel-Array-Kapazität */
    rule_hash_table_t rule_hash_table; /**< 规则哈希表 / Rule hash table / Regel-Hash-Tabelle */
    size_t* cached_rule_indices;  /**< 缓存的规则索引数组 / Cached rule indices array / Gecachte Regelindex-Array */
    size_t cached_rule_count;     /**< 缓存的规则数量 / Cached rule count / Anzahl gecachter Regeln */
    size_t cached_rule_capacity; /**< 缓存的规则容量 / Cached rule capacity / Kapazität gecachter Regeln */
    loaded_plugin_info_t* loaded_plugins; /**< 已加载的插件数组 / Loaded plugins array / Geladenes Plugin-Array */
    size_t loaded_plugin_count;   /**< 已加载插件数量 / Loaded plugin count / Anzahl geladener Plugins */
    size_t loaded_plugin_capacity; /**< 已加载插件数组容量 / Loaded plugin array capacity / Geladenes Plugin-Array-Kapazität */
    plugin_path_cache_entry_t* path_cache; /**< 插件路径缓存 / Plugin path cache / Plugin-Pfad-Cache */
    size_t path_cache_count;      /**< 路径缓存数量 / Path cache count / Pfad-Cache-Anzahl */
    size_t path_cache_capacity;   /**< 路径缓存容量 / Path cache capacity / Pfad-Cache-Kapazität */
    char* plugin_dll_path;       /**< 当前插件DLL路径 / Current plugin DLL path / Aktueller Plugin-DLL-Pfad */
    target_interface_state_t* interface_states; /**< 目标接口状态数组 / Target interface states array / Ziel-Schnittstellen-Status-Array */
    size_t interface_state_count; /**< 接口状态数量 / Interface state count / Schnittstellen-Statusanzahl */
    size_t interface_state_capacity; /**< 接口状态数组容量 / Interface state array capacity / Schnittstellen-Status-Array-Kapazität */
    loaded_nxpt_info_t* loaded_nxpt_files; /**< 已加载的.nxpt文件数组 / Loaded .nxpt files array / Geladene .nxpt-Dateien-Array */
    size_t loaded_nxpt_count;     /**< 已加载.nxpt文件数量 / Loaded .nxpt files count / Anzahl geladener .nxpt-Dateien */
    size_t loaded_nxpt_capacity;  /**< 已加载.nxpt文件数组容量 / Loaded .nxpt files array capacity / Geladene .nxpt-Dateien-Array-Kapazität */
    char* entry_plugin_name;      /**< 入口插件名称 / Entry plugin name / Einstiegs-Plugin-Name */
    char* entry_plugin_path;      /**< 入口插件路径 / Entry plugin path / Einstiegs-Plugin-Pfad */
    char* entry_nxpt_path;        /**< 入口插件.nxpt路径 / Entry plugin .nxpt path / Einstiegs-Plugin-.nxpt-Pfad */
    char* entry_auto_run_interface; /**< 入口插件自动运行接口名称 / Entry plugin auto-run interface name / Einstiegs-Plugin-Autostart-Schnittstellenname */
} pointer_transfer_context_t;

#ifdef __cplusplus
}
#endif

#endif /* POINTER_TRANSFER_TYPES_H */

