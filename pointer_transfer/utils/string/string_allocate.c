/**
 * @file string_allocate.c
 * @brief 分配字符串内存并复制内容函数 / Allocate string memory and copy content function / Zeichenfolgenspeicher zuweisen und Inhalt kopieren Funktion
 */

#include "pointer_transfer_utils.h"
#include <stdlib.h>
#include <string.h>

/**
 * @brief 分配字符串内存并复制内容 / Allocate string memory and copy content / Zeichenfolgenspeicher zuweisen und Inhalt kopieren
 * @param str 源字符串指针 / Source string pointer / Quellzeichenfolgen-Zeiger
 * @return 分配的字符串指针，失败返回NULL / Allocated string pointer, NULL on failure / Zugewiesener Zeichenfolgen-Zeiger, NULL bei Fehler
 */
char* allocate_string(const char* str) {
    if (str == NULL) {
        return NULL;
    }
    size_t len = strlen(str);
    char* result = (char*)malloc(len + 1);
    if (result != NULL) {
        memcpy(result, str, len);
        result[len] = '\0';
    }
    return result;
}

