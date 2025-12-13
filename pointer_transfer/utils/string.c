/**
 * @file string.c
 * @brief 字符串操作函数 / String operation functions / Zeichenfolgen-Operationsfunktionen
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

/**
 * @brief 解析键值对字符串 / Parse key-value pair string / Schlüssel-Wert-Paar-Zeichenfolge parsen
 * @param line 输入行字符串 / Input line string / Eingabezeilen-Zeichenfolge
 * @param key 输出键缓冲区 / Output key buffer / Ausgabe-Schlüssel-Puffer
 * @param key_size 键缓冲区大小 / Key buffer size / Schlüssel-Puffergröße
 * @param value 输出值缓冲区 / Output value buffer / Ausgabe-Wert-Puffer
 * @param value_size 值缓冲区大小 / Value buffer size / Wert-Puffergröße
 * @return 成功返回1，失败返回0 / Returns 1 on success, 0 on failure / Gibt 1 bei Erfolg zurück, 0 bei Fehler
 */
int parse_key_value_simple(const char* line, char* key, size_t key_size, char* value, size_t value_size) {
    if (line == NULL || key == NULL || value == NULL) {
        return 0;
    }
    
    const char* eq_pos = strchr(line, '=');
    if (eq_pos == NULL) {
        return 0;
    }
    
    size_t key_len = eq_pos - line;
    if (key_len >= key_size) {
        key_len = key_size - 1;
    }
    memcpy(key, line, key_len);
    key[key_len] = '\0';
    
    const char* value_start = eq_pos + 1;
    size_t value_len = strlen(value_start);
    if (value_len >= value_size) {
        value_len = value_size - 1;
    }
    memcpy(value, value_start, value_len);
    value[value_len] = '\0';
    
    return 1;
}

