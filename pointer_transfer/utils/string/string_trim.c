/**
 * @file string_trim.c
 * @brief 去除字符串首尾空白字符函数 / Remove leading and trailing whitespace from string function / Führende und nachfolgende Leerzeichen von Zeichenfolge entfernen Funktion
 */

#include "pointer_transfer_utils.h"
#include <stdlib.h>
#include <string.h>

/**
 * @brief 去除字符串首尾空白字符 / Remove leading and trailing whitespace from string / Führende und nachfolgende Leerzeichen von Zeichenfolge entfernen
 * @param str 输入字符串指针 / Input string pointer / Eingabezeichenfolgen-Zeiger
 * @return 处理后的字符串指针 / Processed string pointer / Verarbeiteter Zeichenfolgen-Zeiger
 */
char* trim_string(char* str) {
    if (str == NULL) {
        return NULL;
    }
    
    while (*str == ' ' || *str == '\t') {
        str++;
    }
    
    size_t len = strlen(str);
    while (len > 0 && (str[len - 1] == ' ' || str[len - 1] == '\t' || str[len - 1] == '\r' || str[len - 1] == '\n')) {
        str[len - 1] = '\0';
        len--;
    }
    
    return str;
}

