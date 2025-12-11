# Plugin API Standard

## Function Signature

```c
ReturnType PluginFunc(void* pack_ptr)
```

- `pack_ptr`: Parameter pack pointer, pointing to `pt_param_pack_t` structure
- `ReturnType`: int32_t, int64_t, float, double, void*, or struct

**Calling Convention**:
- Windows: `__cdecl` calling convention (via `NXLD_PLUGIN_CALL` macro)
- Linux: Standard calling convention

## Parameter Pack Structure (ABI Convention)

**Note**: The following structure definitions correspond exactly to the `pointer_transfer_plugin_types.h` header file. Plugins can optionally include this header file to obtain type definitions, or access structure fields directly via memory offsets without including any headers.

```c
// Parameter value union
typedef union {
    int32_t int32_val;              // 32-bit integer value
    int64_t int64_val;              // 64-bit integer value
    float float_val;                // Single-precision floating-point value
    double double_val;              // Double-precision floating-point value
    char char_val;                  // Character value
    void* ptr_val;                  // Pointer value
} pt_param_value_u;  // Size: 8 bytes (x64), 4 bytes (x86)

// Single parameter structure (curried parameter)
typedef struct {
    nxld_param_type_t type;        // Offset 0, size 4 bytes (enum type, actually int)
    size_t size;                   // Offset 8, size 8 bytes (x64) or 4 bytes (x86)
    pt_param_value_u value;        // Offset 16 (x64) or 8 (x86), size 8 bytes (x64) or 4 bytes (x86)
} pt_curried_param_t;  // Total size: 24 bytes (x64), 16 bytes (x86, considering alignment)

// Parameter pack structure
typedef struct {
    int param_count;                // Offset 0, size 4 bytes
    pt_curried_param_t* params;    // Offset 8 (x64) or 4 (x86), size 8 bytes (x64) or 4 bytes (x86, pointer)
} pt_param_pack_t;  // Total size: 16 bytes (x64), 8 bytes (x86)
```

**Memory Layout Notes (x64 platform)**:

1. **pt_param_value_u** (8 bytes):
   - All members share the same memory space
   - Access the corresponding member based on parameter type

2. **pt_curried_param_t** (24 bytes):
   - `type` (offset 0-3): 4 bytes, enum value
   - Padding (offset 4-7): 4 bytes, for alignment to 8-byte boundary
   - `size` (offset 8-15): 8 bytes, parameter size
   - `value` (offset 16-23): 8 bytes, parameter value union

3. **pt_param_pack_t** (16 bytes):
   - `param_count` (offset 0-3): 4 bytes, parameter count
   - Padding (offset 4-7): 4 bytes, for alignment to 8-byte boundary
   - `params` (offset 8-15): 8 bytes, pointer to `pt_curried_param_t` array

**Type Notes**:
- `nxld_param_type_t` is an enum type, represented as `int` (4 bytes) in memory
- `size_t` is 8 bytes on x64 platform, 4 bytes on x86 platform
- Plugins can include `#include "pointer_transfer_plugin_types.h"` to get complete type definitions
- Or access structure fields via direct memory access (using offsets) without including any header files
- **Important**: Structure field order and memory layout are part of the ABI convention and must not be changed

## Parameter Type Enumeration

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

**Numeric Correspondences**:
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

## Usage Examples

### Example 1: Using Header File (Recommended)

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

### Example 2: Direct Memory Access (No Header Required)

```c
// Access via offsets directly, no header files needed
int32_t AddInt(void* pack_ptr) {
    if (pack_ptr == NULL) {
        return 0;
    }
    
    // Read param_count (offset 0)
    int param_count = *(int*)pack_ptr;
    if (param_count < 2) {
        return 0;
    }
    
    // Read params pointer (offset 8)
    void** params_ptr = (void**)((char*)pack_ptr + 8);
    void* params = *params_ptr;
    
    // Access first parameter value (offset 16)
    int32_t a = *(int32_t*)((char*)params + 16);
    
    // Access second parameter value (offset 40 = 24+16)
    int32_t b = *(int32_t*)((char*)params + 40);
    
    return a + b;
}
```

## Return Value Handling

Based on return type, functions should return corresponding values:

- **Integer/Pointer return** (`int32_t`, `int64_t`, `void*`): Returned via `RAX` register
- **Floating-point return** (`float`, `double`): Returned via `XMM0` register
- **Small struct return** (Windows: ≤8 bytes, Linux: ≤16 bytes): Returned via `RAX` register as pointer
- **Large struct return** (Windows: >8 bytes, Linux: >16 bytes): Returned via hidden pointer parameter

**Note**: Plugin functions do not need to handle hidden pointer parameters for large structs, the system handles this automatically.

