/**
 * @file pointer_transfer_context_ignore_check.c
 * @brief 忽略插件列表检查操作 / Ignore Plugin List Check Operations / Ignorier-Plugin-Listen-Prüfungsoperationen
 */

#include "pointer_transfer_context.h"
#include <string.h>

/**
 * @brief 检查插件路径是否在忽略列表中 / Check if plugin path is in ignore list / Prüfen, ob Plugin-Pfad in Ignorierliste ist
 * @param plugin_path 插件路径（可以是绝对路径或相对路径） / Plugin path (can be absolute or relative path) / Plugin-Pfad (kann absoluter oder relativer Pfad sein)
 * @return 在忽略列表中返回1，否则返回0 / Returns 1 if in ignore list, 0 otherwise / Gibt 1 zurück, wenn in Ignorierliste, sonst 0
 */
int is_plugin_ignored(const char* plugin_path) {
    if (plugin_path == NULL) {
        return 0;
    }
    
    pointer_transfer_context_t* ctx = get_global_context();
    if (ctx == NULL) {
        return 0;
    }
    
    if (ctx->ignore_plugins == NULL || ctx->ignore_plugin_count == 0) {
        return 0;
    }
    
    /* 将路径转换为相对路径格式（统一使用正斜杠） / Convert path to relative path format (use forward slash uniformly) / Pfad in relatives Pfadformat konvertieren (einheitlich Schrägstrich verwenden) */
    char normalized_path[1024];
#ifdef _WIN32
    strncpy_s(normalized_path, sizeof(normalized_path), plugin_path, _TRUNCATE);
#else
    strncpy(normalized_path, plugin_path, sizeof(normalized_path) - 1);
    normalized_path[sizeof(normalized_path) - 1] = '\0';
#endif
    
    /* 将反斜杠转换为正斜杠（Windows兼容） / Convert backslashes to forward slashes (Windows compatible) / Umgekehrte Schrägstriche in Schrägstriche konvertieren (Windows-kompatibel) */
    for (size_t i = 0; i < strlen(normalized_path); i++) {
        if (normalized_path[i] == '\\') {
            normalized_path[i] = '/';
        }
    }
    
    /* 提取相对路径部分（从 "plugins/" 开始） / Extract relative path part (starting from "plugins/") / Relativen Pfadteil extrahieren (beginnend mit "plugins/") */
    const char* plugins_pos = strstr(normalized_path, "plugins/");
    if (plugins_pos == NULL) {
        /* 未找到 "plugins/"，返回未匹配 / If "plugins/" not found, return no match / Wenn "plugins/" nicht gefunden, keine Übereinstimmung zurückgeben */
        return 0;
    }
    
    /* 提取自 "plugins/" 开始的相对路径 / Extract relative path starting from "plugins/" / Relativen Pfad ab "plugins/" extrahieren */
    const char* relative_path = plugins_pos;
    
    /* 执行相对路径的精确匹配 / Perform exact match of relative path / Exakte Übereinstimmung des relativen Pfads durchführen */
    for (size_t i = 0; i < ctx->ignore_plugin_count; i++) {
        if (ctx->ignore_plugins[i] != NULL) {
            if (strcmp(relative_path, ctx->ignore_plugins[i]) == 0) {
                return 1;
            }
        }
    }
    
    return 0;
}

