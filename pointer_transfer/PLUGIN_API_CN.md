# 插件API标准 / Plugin API Standard

## 函数签名

```c
ReturnType PluginFunc(void* pack_ptr)
```

- `pack_ptr`: 参数包指针，指向 `pt_param_pack_t` 结构体
- `ReturnType`: int32_t, int64_t, float, double, void*, 或结构体

## 参数包结构（ABI约定）

**注意**：以下结构定义与实际代码中的类型名称对应。插件可以选择包含 `pointer_transfer_plugin_types.h` 头文件以获得类型定义，也可以直接通过内存偏移量访问。

```c
// 参数值联合体
typedef union {
    int32_t int32_val;
    int64_t int64_val;
    float float_val;
    double double_val;
    char char_val;
    void* ptr_val;
} pt_param_value_u;  // 大小：8字节（x64）

// 单个参数结构（柯里化参数）
typedef struct {
    nxld_param_type_t type;  // 偏移量 0，大小 4字节（枚举类型，实际为int）
    size_t size;             // 偏移量 8，大小 8字节（x64，考虑对齐）
    pt_param_value_u value;  // 偏移量 16，大小 8字节
} pt_curried_param_t;  // 总大小：24字节（x64）

// 参数包结构
typedef struct {
    int param_count;              // 偏移量 0，大小 4字节
    // 填充：4字节（对齐到8字节）
    pt_curried_param_t* params;   // 偏移量 8，大小 8字节（指针）
} pt_param_pack_t;  // 总大小：16字节（x64）
```

**类型说明**：
- `nxld_param_type_t` 是枚举类型，在内存中表现为 `int`（4字节）
- 插件可以通过 `#include "pointer_transfer_plugin_types.h"` 获取完整的类型定义
- 或者通过直接内存访问（使用偏移量）来访问结构体字段，无需包含任何头文件

## 参数类型枚举

```c
typedef enum {
    NXLD_PARAM_TYPE_VOID = 0,
    NXLD_PARAM_TYPE_INT32,
    NXLD_PARAM_TYPE_INT64,
    NXLD_PARAM_TYPE_FLOAT,
    NXLD_PARAM_TYPE_DOUBLE,
    NXLD_PARAM_TYPE_CHAR,
    NXLD_PARAM_TYPE_POINTER,
    NXLD_PARAM_TYPE_STRING,
    NXLD_PARAM_TYPE_VARIADIC,
    NXLD_PARAM_TYPE_ANY,
    NXLD_PARAM_TYPE_UNKNOWN
} nxld_param_type_t;
```

**数值对应关系**：
- `NXLD_PARAM_TYPE_VOID` = 0
- `NXLD_PARAM_TYPE_INT32` = 1
- `NXLD_PARAM_TYPE_INT64` = 2
- `NXLD_PARAM_TYPE_FLOAT` = 3
- `NXLD_PARAM_TYPE_DOUBLE` = 4
- `NXLD_PARAM_TYPE_CHAR` = 5
- `NXLD_PARAM_TYPE_POINTER` = 6
- `NXLD_PARAM_TYPE_STRING` = 7
- `NXLD_PARAM_TYPE_VARIADIC` = 8
- `NXLD_PARAM_TYPE_ANY` = 9
- `NXLD_PARAM_TYPE_UNKNOWN` = 10

