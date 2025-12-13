/**
 * @file pointer_transfer_currying_validate_test.c
 * @brief 验证测试参数包创建 / Validation Test Parameter Pack Creation / Validierungs-Testparameterpaket-Erstellung
 */

#include "pointer_transfer_currying.h"
#include "pointer_transfer_utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/**
 * @brief 创建测试参数包 / Create test parameter pack / Testparameterpaket erstellen
 */
pt_param_pack_t* pt_create_test_param_pack(int expected_param_count) {
    if (expected_param_count < 0 || expected_param_count > 256) {
        return NULL;
    }
    
    if (expected_param_count == 0) {
        return pt_create_param_pack(0, NULL, NULL, NULL);
    }
    
    /* 动态分配测试参数数组，支持任意数量的参数 / Dynamically allocate test parameter arrays to support arbitrary parameter counts / Testparameter-Arrays dynamisch zuweisen, um beliebige Parameteranzahlen zu unterstützen */
    nxld_param_type_t* types = (nxld_param_type_t*)malloc(expected_param_count * sizeof(nxld_param_type_t));
    int32_t* values = (int32_t*)malloc(expected_param_count * sizeof(int32_t));
    void** value_ptrs = (void**)malloc(expected_param_count * sizeof(void*));
    size_t* sizes = (size_t*)malloc(expected_param_count * sizeof(size_t));
    
    if (types == NULL || values == NULL || value_ptrs == NULL || sizes == NULL) {
        internal_log_write("ERROR", "Failed to allocate memory for test parameters (%d params)", expected_param_count);
        if (types != NULL) free(types);
        if (values != NULL) free(values);
        if (value_ptrs != NULL) free(value_ptrs);
        if (sizes != NULL) free(sizes);
        return NULL;
    }
    
    /* 初始化测试参数：使用INT32类型和递增的测试值 / Initialize test parameters: use INT32 type and incrementing test values / Testparameter initialisieren: INT32-Typ und inkrementelle Testwerte verwenden */
    for (int i = 0; i < expected_param_count; i++) {
        types[i] = NXLD_PARAM_TYPE_INT32;
        values[i] = i + 1;
        value_ptrs[i] = &values[i];
        sizes[i] = sizeof(int32_t);
    }
    
    pt_param_pack_t* test_pack = pt_create_param_pack(expected_param_count, types, value_ptrs, sizes);
    
    /* 释放临时分配的内存 / Free temporarily allocated memory / Temporär zugewiesenen Speicher freigeben */
    free(types);
    free(values);
    free(value_ptrs);
    free(sizes);
    
    return test_pack;
}

