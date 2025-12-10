# Plugin API Standard

## Function Signature

```c
ReturnType PluginFunc(void* pack_ptr)
```

- `pack_ptr`: Parameter pack pointer, pointing to `pt_param_pack_t` structure
- `ReturnType`: int32_t, int64_t, float, double, void*, or struct

## Parameter Pack Structure (ABI Convention)

**Note**: The following structure definitions correspond to the actual type names in the code. Plugins can optionally include the `pointer_transfer_plugin_types.h` header file to obtain type definitions, or access structure fields directly via memory offsets without including any headers.

```c
// Parameter value union
typedef union {
    int32_t int32_val;
    int64_t int64_val;
    float float_val;
    double double_val;
    char char_val;
    void* ptr_val;
} pt_param_value_u;  // Size: 8 bytes (x64)

// Single parameter structure (curried parameter)
typedef struct {
    nxld_param_type_t type;  // Offset 0, size 4 bytes (enum type, actually int)
    size_t size;             // Offset 8, size 8 bytes (x64, considering alignment)
    pt_param_value_u value;  // Offset 16, size 8 bytes
} pt_curried_param_t;  // Total size: 24 bytes (x64)

// Parameter pack structure
typedef struct {
    int param_count;              // Offset 0, size 4 bytes
    // Padding: 4 bytes (aligned to 8 bytes)
    pt_curried_param_t* params;   // Offset 8, size 8 bytes (pointer)
} pt_param_pack_t;  // Total size: 16 bytes (x64)
```

**Type Notes**:
- `nxld_param_type_t` is an enum type, represented as `int` (4 bytes) in memory
- Plugins can include `#include "pointer_transfer_plugin_types.h"` to get complete type definitions
- Or access structure fields via direct memory access (using offsets) without including any header files

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

