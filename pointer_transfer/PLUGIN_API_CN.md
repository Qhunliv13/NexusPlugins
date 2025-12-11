# 插件API标准 / Plugin API Standard

## 函数签名

```c
ReturnType PluginFunc(void* pack_ptr)
```

- `pack_ptr`: 参数包指针，指向 `pt_param_pack_t` 结构体
- `ReturnType`: int32_t, int64_t, float, double, void*, 或结构体

**调用约定**：
- Windows: `__cdecl` 调用约定（通过 `NXLD_PLUGIN_CALL` 宏定义）
- Linux: 标准调用约定

## 参数包结构（ABI约定）

**注意**：以下结构定义与实际代码中的 `pointer_transfer_plugin_types.h` 头文件完全对应。插件可以选择包含该头文件以获得类型定义，也可以直接通过内存偏移量访问。

```c
// 参数值联合体
typedef union {
    int32_t int32_val;              // 32位整数值
    int64_t int64_val;              // 64位整数值
    float float_val;                // 单精度浮点值
    double double_val;              // 双精度浮点值
    char char_val;                  // 字符值
    void* ptr_val;                  // 指针值
} pt_param_value_u;  // 大小：8字节（x64），4字节（x86）

// 单个参数结构（柯里化参数）
typedef struct {
    nxld_param_type_t type;        // 偏移量 0，大小 4字节（枚举类型，实际为int）
    size_t size;                   // 偏移量 8，大小 8字节（x64）或 4字节（x86）
    pt_param_value_u value;        // 偏移量 16（x64）或 8（x86），大小 8字节（x64）或 4字节（x86）
} pt_curried_param_t;  // 总大小：24字节（x64），16字节（x86，考虑对齐）

// 参数包结构
typedef struct {
    int param_count;                // 偏移量 0，大小 4字节
    pt_curried_param_t* params;    // 偏移量 8（x64）或 4（x86），大小 8字节（x64）或 4字节（x86，指针）
} pt_param_pack_t;  // 总大小：16字节（x64），8字节（x86）
```

**内存布局说明（x64平台）**：

1. **pt_param_value_u** (8字节)：
   - 所有成员共享同一块内存
   - 根据参数类型访问对应的成员

2. **pt_curried_param_t** (24字节)：
   - `type` (偏移0-3): 4字节，枚举值
   - 填充 (偏移4-7): 4字节，用于对齐到8字节边界
   - `size` (偏移8-15): 8字节，参数大小
   - `value` (偏移16-23): 8字节，参数值联合体

3. **pt_param_pack_t** (16字节)：
   - `param_count` (偏移0-3): 4字节，参数数量
   - 填充 (偏移4-7): 4字节，用于对齐到8字节边界
   - `params` (偏移8-15): 8字节，指向 `pt_curried_param_t` 数组的指针

**类型说明**：
- `nxld_param_type_t` 是枚举类型，在内存中表现为 `int`（4字节）
- `size_t` 在x64平台为8字节，在x86平台为4字节
- 插件可以通过 `#include "pointer_transfer_plugin_types.h"` 获取完整的类型定义
- 或者通过直接内存访问（使用偏移量）来访问结构体字段，无需包含任何头文件
- **重要**：结构体字段顺序和内存布局是ABI约定的一部分，不得更改

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

## 使用示例

### 示例1：包含头文件方式（推荐）

```c
#include "pointer_transfer_plugin_types.h"
#include "nxld_plugin_interface.h"

NXLD_PLUGIN_EXPORT int32_t NXLD_PLUGIN_CALL AddInt(void* pack_ptr) {
    pt_param_pack_t* pack = (pt_param_pack_t*)pack_ptr;
    if (pack == NULL || pack->param_count < 2) {
        return 0;
    }
    int32_t a = pack->params[0].value.int32_val;
    int32_t b = pack->params[1].value.int32_val;
    return a + b;
}
```

### 示例2：直接内存访问方式（无需头文件）

```c
// 通过偏移量直接访问，无需包含任何头文件
int32_t AddInt(void* pack_ptr) {
    if (pack_ptr == NULL) {
        return 0;
    }
    
    // 读取 param_count (偏移0)
    int param_count = *(int*)pack_ptr;
    if (param_count < 2) {
        return 0;
    }
    
    // 读取 params 指针 (偏移8)
    void** params_ptr = (void**)((char*)pack_ptr + 8);
    void* params = *params_ptr;
    
    // 访问第一个参数的值 (偏移16)
    int32_t a = *(int32_t*)((char*)params + 16);
    
    // 访问第二个参数的值 (偏移40 = 24+16)
    int32_t b = *(int32_t*)((char*)params + 40);
    
    return a + b;
}
```

## 返回值处理

根据返回值类型，函数应返回相应的值：

- **整数/指针返回值** (`int32_t`, `int64_t`, `void*`): 通过 `RAX` 寄存器返回
- **浮点返回值** (`float`, `double`): 通过 `XMM0` 寄存器返回
- **小结构体返回值** (Windows: ≤8字节, Linux: ≤16字节): 通过 `RAX` 寄存器返回指针
- **大结构体返回值** (Windows: >8字节, Linux: >16字节): 通过隐藏指针参数返回

**注意**：插件函数不需要处理大结构体的隐藏指针参数，系统会自动处理。

