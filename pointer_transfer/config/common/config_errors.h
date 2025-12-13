/**
 * @file config_errors.h
 * @brief 配置错误码定义 / Configuration Error Code Definitions / Konfigurations-Fehlercode-Definitionen
 */

#ifndef CONFIG_ERRORS_H
#define CONFIG_ERRORS_H

#ifdef __cplusplus
extern "C" {
#endif

/* 错误码定义 / Error code definitions / Fehlercode-Definitionen */
#define CONFIG_ERR_SUCCESS           0   /**< 成功 / Success / Erfolg */
#define CONFIG_ERR_INVALID_PARAM    -1   /**< 无效参数 / Invalid parameter / Ungültiger Parameter */
#define CONFIG_ERR_FILE_OPEN        -2   /**< 文件打开失败 / File open failed / Dateiöffnung fehlgeschlagen */
#define CONFIG_ERR_MEMORY           -3   /**< 内存分配失败 / Memory allocation failed / Speicherzuweisung fehlgeschlagen */
#define CONFIG_ERR_INCOMPLETE       -4   /**< 配置不完整 / Configuration incomplete / Konfiguration unvollständig */
#define CONFIG_ERR_OVERFLOW         -5   /**< 溢出错误 / Overflow error / Überlauffehler */

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_ERRORS_H */

