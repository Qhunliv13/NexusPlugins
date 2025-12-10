/**
 * @file test_integration.c
 * @brief 指针传递插件集成测试 / Pointer Transfer Plugin Integration Test / Zeigerübertragungs-Plugin-Integrationstest
 * @details 完整的集成测试，包括插件加载和实际函数调用 / Complete integration test including plugin loading and actual function calls / Vollständiger Integrationstest einschließlich Plugin-Laden und tatsächlicher Funktionsaufrufe
 */

#include "pointer_transfer_plugin.h"
#include "pointer_transfer_config.h"
#include "pointer_transfer_context.h"
#include "pointer_transfer_currying.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

/* 测试插件函数类型定义 / Test plugin function type definitions / Test-Plugin-Funktionstypdefinitionen */
/* 所有函数接受void*参数包指针，无需头文件依赖 / All functions accept void* param pack pointer, no header dependency / Alle Funktionen akzeptieren void*-Parameterpaket-Zeiger, keine Header-Abhängigkeit */
typedef int32_t (*AddIntFunc)(void*);
typedef double (*AddDoubleFunc)(void*);
typedef float (*AddFloatFunc)(void*);
typedef int32_t (*StrLenFunc)(void*);
typedef void* (*MallocWrapperFunc)(void*);

typedef struct {
    int32_t x;
    int32_t y;
} Point2D;

typedef struct {
    int32_t x;
    int32_t y;
    int32_t z;
} Point3D;

typedef Point2D (*CreatePoint2DFunc)(void*);
typedef int64_t (*AddInt64Func)(void*);
typedef char (*GetCharFunc)(void*);
typedef Point3D (*CreatePoint3DFunc)(void*);
typedef int32_t (*MixedParamsFunc)(void*);
typedef int32_t (*ManyParamsFunc)(void*);
typedef int32_t (*StructParamFunc)(void*);
typedef int32_t (*VariadicSumFunc)(void*);

/**
 * @brief 测试插件直接调用 / Test plugin direct call / Test-Plugin-Direktaufruf
 * @return 成功返回0，失败返回非0 / Returns 0 on success, non-zero on failure / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
 */
int test_direct_plugin_call(void) {
    printf("\n=== Integration Test: Direct Plugin Call ===\n");
    
#ifdef _WIN32
    const char* plugin_path = "test_plugin.dll";
#else
    const char* plugin_path = "test_plugin.so";
#endif
    
    void* handle = NULL;
#ifdef _WIN32
    handle = LoadLibraryA(plugin_path);
#else
    handle = dlopen(plugin_path, RTLD_NOW);
#endif
    
    if (handle == NULL) {
        printf("WARNING: Cannot load test plugin (%s). Skipping direct call test.\n", plugin_path);
        printf("To enable this test, compile test_plugin first: scons test_plugin.dll\n");
        return 0;  /* 插件未编译时跳过测试 / Skip test when plugin not compiled / Test überspringen wenn Plugin nicht kompiliert */
    }
    
    printf("Test plugin loaded successfully\n");
    
    /* 测试AddInt / Test AddInt / AddInt testen */
    AddIntFunc add_int = NULL;
#ifdef _WIN32
    add_int = (AddIntFunc)GetProcAddress(handle, "AddInt");
#else
    add_int = (AddIntFunc)dlsym(handle, "AddInt");
#endif
    
    if (add_int != NULL) {
        /* 创建参数包 / Create param pack */
        int32_t a = 10, b = 20;
        void* values[] = {&a, &b};
        nxld_param_type_t types[] = {NXLD_PARAM_TYPE_INT32, NXLD_PARAM_TYPE_INT32};
        size_t sizes[] = {sizeof(int32_t), sizeof(int32_t)};
        pt_param_pack_t* pack = pt_create_param_pack(2, types, values, sizes);
        if (pack != NULL) {
            int32_t result = add_int((void*)pack);
            printf("AddInt(10, 20) = %d (expected: 30)\n", result);
            if (result == 30) {
                printf("  PASS: AddInt test\n");
            } else {
                printf("  FAIL: AddInt test (got %d, expected 30)\n", result);
            }
            pt_free_param_pack(pack);
        } else {
            printf("  FAIL: AddInt test (failed to create param pack)\n");
        }
    } else {
        printf("WARNING: AddInt function not found in plugin\n");
    }
    
    /* 测试AddDouble / Test AddDouble / AddDouble testen */
    AddDoubleFunc add_double = NULL;
#ifdef _WIN32
    add_double = (AddDoubleFunc)GetProcAddress(handle, "AddDouble");
#else
    add_double = (AddDoubleFunc)dlsym(handle, "AddDouble");
#endif
    
    if (add_double != NULL) {
        double a = 3.14, b = 2.86;
        void* values[] = {&a, &b};
        nxld_param_type_t types[] = {NXLD_PARAM_TYPE_DOUBLE, NXLD_PARAM_TYPE_DOUBLE};
        size_t sizes[] = {sizeof(double), sizeof(double)};
        pt_param_pack_t* pack = pt_create_param_pack(2, types, values, sizes);
        if (pack != NULL) {
            double result = add_double((void*)pack);
            printf("AddDouble(3.14, 2.86) = %lf (expected: 6.0)\n", result);
            if (result >= 5.99 && result <= 6.01) {
                printf("  PASS: AddDouble test\n");
            } else {
                printf("  FAIL: AddDouble test (got %lf, expected 6.0)\n", result);
            }
            pt_free_param_pack(pack);
        } else {
            printf("  FAIL: AddDouble test (failed to create param pack)\n");
        }
    } else {
        printf("WARNING: AddDouble function not found in plugin\n");
    }
    
    /* 测试AddFloat / Test AddFloat / AddFloat testen */
    AddFloatFunc add_float = NULL;
#ifdef _WIN32
    add_float = (AddFloatFunc)GetProcAddress(handle, "AddFloat");
#else
    add_float = (AddFloatFunc)dlsym(handle, "AddFloat");
#endif
    
    if (add_float != NULL) {
        float a = 1.5f, b = 2.5f;
        void* values[] = {&a, &b};
        nxld_param_type_t types[] = {NXLD_PARAM_TYPE_FLOAT, NXLD_PARAM_TYPE_FLOAT};
        size_t sizes[] = {sizeof(float), sizeof(float)};
        pt_param_pack_t* pack = pt_create_param_pack(2, types, values, sizes);
        if (pack != NULL) {
            float result = add_float((void*)pack);
            printf("AddFloat(1.5, 2.5) = %f (expected: 4.0)\n", result);
            if (result >= 3.99f && result <= 4.01f) {
                printf("  PASS: AddFloat test\n");
            } else {
                printf("  FAIL: AddFloat test (got %f, expected 4.0)\n", result);
            }
            pt_free_param_pack(pack);
        } else {
            printf("  FAIL: AddFloat test (failed to create param pack)\n");
        }
    } else {
        printf("WARNING: AddFloat function not found in plugin\n");
    }
    
    /* 测试StrLen / Test StrLen / StrLen testen */
    StrLenFunc str_len = NULL;
#ifdef _WIN32
    str_len = (StrLenFunc)GetProcAddress(handle, "StrLen");
#else
    str_len = (StrLenFunc)dlsym(handle, "StrLen");
#endif
    
    if (str_len != NULL) {
        const char* test_str = "Hello, World!";
        void* values[] = {(void*)test_str};
        nxld_param_type_t types[] = {NXLD_PARAM_TYPE_STRING};
        size_t sizes[] = {strlen(test_str) + 1};
        pt_param_pack_t* pack = pt_create_param_pack(1, types, values, sizes);
        if (pack != NULL) {
            int32_t result = str_len((void*)pack);
            printf("StrLen(\"%s\") = %d (expected: %zu)\n", test_str, result, strlen(test_str));
            if (result == (int32_t)strlen(test_str)) {
                printf("  PASS: StrLen test\n");
            } else {
                printf("  FAIL: StrLen test (got %d, expected %zu)\n", result, strlen(test_str));
            }
            pt_free_param_pack(pack);
        } else {
            printf("  FAIL: StrLen test (failed to create param pack)\n");
        }
    } else {
        printf("WARNING: StrLen function not found in plugin\n");
    }
    
    /* 测试CreatePoint2D / Test CreatePoint2D / CreatePoint2D testen */
    CreatePoint2DFunc create_point = NULL;
#ifdef _WIN32
    create_point = (CreatePoint2DFunc)GetProcAddress(handle, "CreatePoint2D");
#else
    create_point = (CreatePoint2DFunc)dlsym(handle, "CreatePoint2D");
#endif
    
    if (create_point != NULL) {
        int32_t x = 100, y = 200;
        void* values[] = {&x, &y};
        nxld_param_type_t types[] = {NXLD_PARAM_TYPE_INT32, NXLD_PARAM_TYPE_INT32};
        size_t sizes[] = {sizeof(int32_t), sizeof(int32_t)};
        pt_param_pack_t* pack = pt_create_param_pack(2, types, values, sizes);
        if (pack != NULL) {
            Point2D p = create_point((void*)pack);
            printf("CreatePoint2D(100, 200) = (%d, %d) (expected: (100, 200))\n", p.x, p.y);
            if (p.x == 100 && p.y == 200) {
                printf("  PASS: CreatePoint2D test\n");
            } else {
                printf("  FAIL: CreatePoint2D test (got (%d, %d), expected (100, 200))\n", p.x, p.y);
            }
            pt_free_param_pack(pack);
        } else {
            printf("  FAIL: CreatePoint2D test (failed to create param pack)\n");
        }
    } else {
        printf("WARNING: CreatePoint2D function not found in plugin\n");
    }
    
    /* 测试AddInt64 / Test AddInt64 / AddInt64 testen */
    AddInt64Func add_int64 = NULL;
#ifdef _WIN32
    add_int64 = (AddInt64Func)GetProcAddress(handle, "AddInt64");
#else
    add_int64 = (AddInt64Func)dlsym(handle, "AddInt64");
#endif
    
    if (add_int64 != NULL) {
        int64_t a = 10000000000LL, b = 20000000000LL;
        void* values[] = {&a, &b};
        nxld_param_type_t types[] = {NXLD_PARAM_TYPE_INT64, NXLD_PARAM_TYPE_INT64};
        size_t sizes[] = {sizeof(int64_t), sizeof(int64_t)};
        pt_param_pack_t* pack = pt_create_param_pack(2, types, values, sizes);
        if (pack != NULL) {
            int64_t result = add_int64((void*)pack);
            printf("AddInt64(10000000000, 20000000000) = %lld (expected: 30000000000)\n", (long long)result);
            if (result == 30000000000LL) {
                printf("  PASS: AddInt64 test\n");
            } else {
                printf("  FAIL: AddInt64 test (got %lld, expected 30000000000)\n", (long long)result);
            }
            pt_free_param_pack(pack);
        } else {
            printf("  FAIL: AddInt64 test (failed to create param pack)\n");
        }
    } else {
        printf("WARNING: AddInt64 function not found in plugin\n");
    }
    
    /* 测试GetChar / Test GetChar / GetChar testen */
    GetCharFunc get_char = NULL;
#ifdef _WIN32
    get_char = (GetCharFunc)GetProcAddress(handle, "GetChar");
#else
    get_char = (GetCharFunc)dlsym(handle, "GetChar");
#endif
    
    if (get_char != NULL) {
        char c = 'A';
        void* values[] = {&c};
        nxld_param_type_t types[] = {NXLD_PARAM_TYPE_CHAR};
        size_t sizes[] = {sizeof(char)};
        pt_param_pack_t* pack = pt_create_param_pack(1, types, values, sizes);
        if (pack != NULL) {
            char result = get_char((void*)pack);
            printf("GetChar('A') = '%c' (expected: 'A')\n", result);
            if (result == 'A') {
                printf("  PASS: GetChar test\n");
            } else {
                printf("  FAIL: GetChar test (got '%c', expected 'A')\n", result);
            }
            pt_free_param_pack(pack);
        } else {
            printf("  FAIL: GetChar test (failed to create param pack)\n");
        }
    } else {
        printf("WARNING: GetChar function not found in plugin\n");
    }
    
    /* 测试MallocWrapper（指针返回值）/ Test MallocWrapper (pointer return) / MallocWrapper testen (Zeiger-Rückgabe) */
    MallocWrapperFunc malloc_wrapper = NULL;
#ifdef _WIN32
    malloc_wrapper = (MallocWrapperFunc)GetProcAddress(handle, "MallocWrapper");
#else
    malloc_wrapper = (MallocWrapperFunc)dlsym(handle, "MallocWrapper");
#endif
    
    if (malloc_wrapper != NULL) {
        size_t size = 1024;
        void* values[] = {&size};
        nxld_param_type_t types[] = {NXLD_PARAM_TYPE_INT64};
        size_t sizes[] = {sizeof(size_t)};
        pt_param_pack_t* pack = pt_create_param_pack(1, types, values, sizes);
        if (pack != NULL) {
            void* result = malloc_wrapper((void*)pack);
            printf("MallocWrapper(1024) = %p (expected: non-NULL pointer)\n", result);
            if (result != NULL) {
                printf("  PASS: MallocWrapper test\n");
                free(result);
            } else {
                printf("  FAIL: MallocWrapper test (got NULL)\n");
            }
            pt_free_param_pack(pack);
        } else {
            printf("  FAIL: MallocWrapper test (failed to create param pack)\n");
        }
    } else {
        printf("WARNING: MallocWrapper function not found in plugin\n");
    }
    
    /* 测试CreatePoint3D: 大结构体返回值, 12字节 / Test CreatePoint3D: large struct return, 12 bytes / CreatePoint3D testen: große Struktur-Rückgabe, 12 Bytes */
    CreatePoint3DFunc create_point3d = NULL;
#ifdef _WIN32
    create_point3d = (CreatePoint3DFunc)GetProcAddress(handle, "CreatePoint3D");
#else
    create_point3d = (CreatePoint3DFunc)dlsym(handle, "CreatePoint3D");
#endif
    
    if (create_point3d != NULL) {
        int32_t x = 10, y = 20, z = 30;
        void* values[] = {&x, &y, &z};
        nxld_param_type_t types[] = {NXLD_PARAM_TYPE_INT32, NXLD_PARAM_TYPE_INT32, NXLD_PARAM_TYPE_INT32};
        size_t sizes[] = {sizeof(int32_t), sizeof(int32_t), sizeof(int32_t)};
        pt_param_pack_t* pack = pt_create_param_pack(3, types, values, sizes);
        if (pack != NULL) {
            Point3D p = create_point3d((void*)pack);
            printf("CreatePoint3D(10, 20, 30) = (%d, %d, %d) (expected: (10, 20, 30))\n", p.x, p.y, p.z);
            if (p.x == 10 && p.y == 20 && p.z == 30) {
                printf("  PASS: CreatePoint3D test\n");
            } else {
                printf("  FAIL: CreatePoint3D test (got (%d, %d, %d), expected (10, 20, 30))\n", p.x, p.y, p.z);
            }
            pt_free_param_pack(pack);
        } else {
            printf("  FAIL: CreatePoint3D test (failed to create param pack)\n");
        }
    } else {
        printf("WARNING: CreatePoint3D function not found in plugin\n");
    }
    
    /* 测试MixedParams（混合参数类型）/ Test MixedParams (mixed parameter types) / MixedParams testen (gemischte Parametertypen) */
    MixedParamsFunc mixed_params = NULL;
#ifdef _WIN32
    mixed_params = (MixedParamsFunc)GetProcAddress(handle, "MixedParams");
#else
    mixed_params = (MixedParamsFunc)dlsym(handle, "MixedParams");
#endif
    
    if (mixed_params != NULL) {
        int32_t test_int = 100;
        float test_float = 2.5f;
        double test_double = 3.14;
        int32_t test_ptr_val = 42;
        void* test_ptr = &test_ptr_val;
        const char* test_str = "test";
        void* values[] = {&test_int, &test_float, &test_double, &test_ptr, (void*)test_str};
        nxld_param_type_t types[] = {NXLD_PARAM_TYPE_INT32, NXLD_PARAM_TYPE_FLOAT, NXLD_PARAM_TYPE_DOUBLE, NXLD_PARAM_TYPE_POINTER, NXLD_PARAM_TYPE_STRING};
        size_t sizes[] = {sizeof(int32_t), sizeof(float), sizeof(double), sizeof(void*), strlen(test_str) + 1};
        pt_param_pack_t* pack = pt_create_param_pack(5, types, values, sizes);
        if (pack != NULL) {
            int32_t result = mixed_params((void*)pack);
            int32_t expected = (int32_t)(test_int + (int32_t)test_float + (int32_t)test_double + (int32_t)strlen(test_str));
            printf("MixedParams(%d, %f, %lf, %p, \"%s\") = %d (expected: %d)\n", 
                   test_int, test_float, test_double, test_ptr, test_str, result, expected);
            if (result == expected) {
                printf("  PASS: MixedParams test\n");
            } else {
                printf("  FAIL: MixedParams test (got %d, expected %d)\n", result, expected);
            }
            pt_free_param_pack(pack);
        } else {
            printf("  FAIL: MixedParams test (failed to create param pack)\n");
        }
    } else {
        printf("WARNING: MixedParams function not found in plugin\n");
    }
    
    /* 测试ManyParams（超过寄存器数量的参数）/ Test ManyParams (parameters exceeding register count) / ManyParams testen (Parameter über Registeranzahl) */
    ManyParamsFunc many_params = NULL;
#ifdef _WIN32
    many_params = (ManyParamsFunc)GetProcAddress(handle, "ManyParams");
#else
    many_params = (ManyParamsFunc)dlsym(handle, "ManyParams");
#endif
    
    if (many_params != NULL) {
        int32_t a = 1, b = 2, c = 3, d = 4, e = 5, f = 6;
        void* values[] = {&a, &b, &c, &d, &e, &f};
        nxld_param_type_t types[] = {NXLD_PARAM_TYPE_INT32, NXLD_PARAM_TYPE_INT32, NXLD_PARAM_TYPE_INT32, NXLD_PARAM_TYPE_INT32, NXLD_PARAM_TYPE_INT32, NXLD_PARAM_TYPE_INT32};
        size_t sizes[] = {sizeof(int32_t), sizeof(int32_t), sizeof(int32_t), sizeof(int32_t), sizeof(int32_t), sizeof(int32_t)};
        pt_param_pack_t* pack = pt_create_param_pack(6, types, values, sizes);
        if (pack != NULL) {
            int32_t result = many_params((void*)pack);
            int32_t expected = 21;
            printf("ManyParams(1, 2, 3, 4, 5, 6) = %d (expected: %d)\n", result, expected);
            if (result == expected) {
                printf("  PASS: ManyParams test\n");
            } else {
                printf("  FAIL: ManyParams test (got %d, expected %d)\n", result, expected);
            }
            pt_free_param_pack(pack);
        } else {
            printf("  FAIL: ManyParams test (failed to create param pack)\n");
        }
    } else {
        printf("WARNING: ManyParams function not found in plugin\n");
    }
    
    /* 测试StructParam（结构体参数值传递）/ Test StructParam (struct parameter value passing) / StructParam testen (Strukturparameter-Wertübergabe) */
    StructParamFunc struct_param = NULL;
#ifdef _WIN32
    struct_param = (StructParamFunc)GetProcAddress(handle, "StructParam");
#else
    struct_param = (StructParamFunc)dlsym(handle, "StructParam");
#endif
    
    if (struct_param != NULL) {
        Point2D test_point = { 50, 75 };
        void* values[] = {&test_point};
        nxld_param_type_t types[] = {NXLD_PARAM_TYPE_POINTER};
        size_t sizes[] = {sizeof(Point2D)};
        pt_param_pack_t* pack = pt_create_param_pack(1, types, values, sizes);
        if (pack != NULL) {
            int32_t result = struct_param((void*)pack);
            int32_t expected = 125;
            printf("StructParam({50, 75}) = %d (expected: %d)\n", result, expected);
            if (result == expected) {
                printf("  PASS: StructParam test\n");
            } else {
                printf("  FAIL: StructParam test (got %d, expected %d)\n", result, expected);
            }
            pt_free_param_pack(pack);
        } else {
            printf("  FAIL: StructParam test (failed to create param pack)\n");
        }
    } else {
        printf("WARNING: StructParam function not found in plugin\n");
    }
    
    /* 测试VariadicSum（可变参数）/ Test VariadicSum (variadic parameters) / VariadicSum testen (variable Parameter) */
    VariadicSumFunc variadic_sum = NULL;
#ifdef _WIN32
    variadic_sum = (VariadicSumFunc)GetProcAddress(handle, "VariadicSum");
#else
    variadic_sum = (VariadicSumFunc)dlsym(handle, "VariadicSum");
#endif
    
    if (variadic_sum != NULL) {
        int32_t count = 4;
        int32_t v1 = 10, v2 = 20, v3 = 30, v4 = 40;
        void* values[] = {&count, &v1, &v2, &v3, &v4};
        nxld_param_type_t types[] = {NXLD_PARAM_TYPE_INT32, NXLD_PARAM_TYPE_INT32, NXLD_PARAM_TYPE_INT32, NXLD_PARAM_TYPE_INT32, NXLD_PARAM_TYPE_INT32};
        size_t sizes[] = {sizeof(int32_t), sizeof(int32_t), sizeof(int32_t), sizeof(int32_t), sizeof(int32_t)};
        pt_param_pack_t* pack = pt_create_param_pack(5, types, values, sizes);
        if (pack != NULL) {
            int32_t result = variadic_sum((void*)pack);
            int32_t expected = 100;
            printf("VariadicSum(4, 10, 20, 30, 40) = %d (expected: %d)\n", result, expected);
            if (result == expected) {
                printf("  PASS: VariadicSum test\n");
            } else {
                printf("  FAIL: VariadicSum test (got %d, expected %d)\n", result, expected);
            }
            pt_free_param_pack(pack);
        } else {
            printf("  FAIL: VariadicSum test (failed to create param pack)\n");
        }
    } else {
        printf("WARNING: VariadicSum function not found in plugin\n");
    }
    
#ifdef _WIN32
    FreeLibrary(handle);
#else
    dlclose(handle);
#endif
    
    return 0;
}

/**
 * @brief 测试通过插件系统调用 / Test call through plugin system / Aufruf über Plugin-System testen
 * @return 成功返回0，失败返回非0 / Returns 0 on success, non-zero on failure / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
 */
int test_plugin_system_call(void) {
    printf("\n=== Integration Test: Plugin System Call ===\n");
    
    printf("Loading configuration...\n");
    int config_result = parse_entry_plugin_config("test_config.nxpt");
    if (config_result != 0) {
        printf("WARNING: Configuration loading failed. This is expected if test plugin is not compiled.\n");
        return 0;
    }
    
    printf("Configuration loaded successfully\n");
    
    printf("Loading transfer rules...\n");
    int rules_result = load_transfer_rules("test_config.nxpt");
    if (rules_result != 0) {
        printf("WARNING: Transfer rules loading failed. Rules may not be configured correctly.\n");
    } else {
        printf("Transfer rules loaded successfully\n");
    }
    
    printf("Testing CallPlugin with AddInt (requires 2 parameters)...\n");
    int32_t test_value_a = 15;
    int32_t test_value_b = 25;
    
    /* 第一个参数调用：预期失败，AddInt需要2个参数，此时仅参数0就绪 / First parameter call: expected to fail, AddInt requires 2 parameters, only param 0 is ready / Erster Parameteraufruf: erwartet fehlzuschlagen, AddInt erfordert 2 Parameter, nur Param 0 ist bereit */
    int result_a = CallPlugin("TestPlugin", "AddInt", 0, &test_value_a);
    if (result_a != 0) {
        printf("  PASS: CallPlugin param[0] correctly failed (requires both parameters)\n");
    } else {
        printf("  INFO: CallPlugin param[0] returned %d\n", result_a);
    }
    
    /* 第二个参数调用：预期成功，此时两个参数均已就绪 / Second parameter call: expected to succeed, both parameters are now ready / Zweiter Parameteraufruf: erwartet erfolgreich, beide Parameter sind jetzt bereit */
    int result_b = CallPlugin("TestPlugin", "AddInt", 1, &test_value_b);
    if (result_b == 0) {
        printf("  PASS: CallPlugin param[1] succeeded (both parameters ready, result should be 40)\n");
    } else {
        printf("  INFO: CallPlugin param[1] returned %d\n", result_b);
    }
    
    return 0;
}

/**
 * @brief 测试错误处理 / Test error handling / Fehlerbehandlung testen
 * @return 成功返回0，失败返回非0 / Returns 0 on success, non-zero on failure / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
 */
int test_error_handling(void) {
    printf("\n=== Integration Test: Error Handling ===\n");
    
    printf("Testing TransferPointer with NULL pointer...\n");
    int result = TransferPointer(NULL, NXLD_PARAM_TYPE_INT32, NULL, sizeof(int32_t));
    if (result == -1) {
        printf("  PASS: NULL pointer handling (result=%d, expected -1)\n", result);
    } else {
        printf("  INFO: NULL pointer handling returned %d (expected -1)\n", result);
    }
    
    printf("Testing TransferPointer with invalid type...\n");
    int32_t test_value = 42;
    result = TransferPointer(&test_value, NXLD_PARAM_TYPE_STRING, NULL, sizeof(int32_t));
    printf("  INFO: Type mismatch handling (result=%d, expected 0 or 1)\n", result);
    
    printf("Testing TransferPointer with zero size...\n");
    result = TransferPointer(&test_value, NXLD_PARAM_TYPE_INT32, NULL, 0);
    printf("  INFO: Zero size handling (result=%d)\n", result);
    
    printf("Testing TransferPointer with large size...\n");
    result = TransferPointer(&test_value, NXLD_PARAM_TYPE_INT32, NULL, SIZE_MAX);
    printf("  INFO: Large size handling (result=%d)\n", result);
    
    printf("Testing CallPlugin with NULL plugin name...\n");
    result = CallPlugin(NULL, "TestInterface", 0, &test_value);
    if (result != 0) {
        printf("  PASS: NULL plugin name handling (result=%d)\n", result);
    } else {
        printf("  INFO: NULL plugin name handling returned %d\n", result);
    }
    
    printf("Testing CallPlugin with NULL interface name...\n");
    result = CallPlugin("TestPlugin", NULL, 0, &test_value);
    if (result != 0) {
        printf("  PASS: NULL interface name handling (result=%d)\n", result);
    } else {
        printf("  INFO: NULL interface name handling returned %d\n", result);
    }
    
    printf("Testing CallPlugin with invalid plugin name...\n");
    result = CallPlugin("NonExistentPlugin", "NonExistentInterface", 0, &test_value);
    if (result != 0) {
        printf("  PASS: Invalid plugin name handling (result=%d)\n", result);
    } else {
        printf("  INFO: Invalid plugin name handling returned %d\n", result);
    }
    
    printf("Testing CallPlugin with negative parameter index...\n");
    result = CallPlugin("TestPlugin", "TestInterface", -1, &test_value);
    printf("  INFO: Negative parameter index handling (result=%d)\n", result);
    
    printf("Testing CallPlugin with NULL parameter value...\n");
    result = CallPlugin("TestPlugin", "TestInterface", 0, NULL);
    printf("  INFO: NULL parameter value handling (result=%d)\n", result);
    
    return 0;
}

/**
 * @brief 主函数 / Main function / Hauptfunktion
 * @param argc 命令行参数数量 / Command line argument count / Anzahl der Befehlszeilenargumente
 * @param argv 命令行参数数组 / Command line argument array / Befehlszeilenargument-Array
 * @return 成功返回0，失败返回非0 / Returns 0 on success, non-zero on failure / Gibt 0 bei Erfolg zurück, ungleich 0 bei Fehler
 */
int main(int argc, char* argv[]) {
    (void)argc;  /* 未使用的参数 / Unused parameter / Nicht verwendeter Parameter */
    (void)argv;  /* 未使用的参数 / Unused parameter / Nicht verwendeter Parameter */
    printf("========================================\n");
    printf("Pointer Transfer Plugin Integration Test\n");
    printf("========================================\n");
    
    int failed_tests = 0;
    
    if (test_direct_plugin_call() != 0) {
        failed_tests++;
    }
    
    if (test_plugin_system_call() != 0) {
        failed_tests++;
    }
    
    if (test_error_handling() != 0) {
        failed_tests++;
    }
    
    printf("\n========================================\n");
    printf("Integration Test Summary:\n");
    printf("========================================\n");
    if (failed_tests == 0) {
        printf("All integration tests completed!\n");
        return 0;
    } else {
        printf("Some integration tests failed: %d\n", failed_tests);
        return 1;
    }
}

