/**
 * @file log.c
 * @brief 日志输出函数 / Log output function / Protokollausgabefunktion
 */

#include "pointer_transfer_utils.h"
#include "pointer_transfer_context.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

/**
 * @brief 输出日志消息 / Output log message / Protokollnachricht ausgeben
 * @param level 日志级别字符串 / Log level string / Protokollierungsebenen-Zeichenfolge
 * @param format 格式化字符串 / Format string / Formatzeichenfolge
 * @param ... 可变参数 / Variable arguments / Variable Argumente
 */
void internal_log_write(const char* level, const char* format, ...) {
    if (level == NULL || format == NULL) {
        return;
    }
    
    if (strcmp(level, "INFO") == 0) {
        pointer_transfer_context_t* ctx = get_global_context();
        if (ctx != NULL && ctx->disable_info_log != 0) {
            return;
        }
    }
    
    va_list args;
    va_start(args, format);
    
    time_t now;
    time(&now);
    struct tm* timeinfo = localtime(&now);
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", timeinfo);
    
    fprintf(stderr, "[%s] [%s] [PointerTransferPlugin] ", time_str, level);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    fflush(stderr);
    
    va_end(args);
}

