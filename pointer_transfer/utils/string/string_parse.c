/**
 * @file string_parse.c
 * @brief 解析键值对字符串函数 / Parse key-value pair string function / Schlüssel-Wert-Paar-Zeichenfolge parsen Funktion
 */

#include "pointer_transfer_utils.h"
#include <stdlib.h>
#include <string.h>

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

