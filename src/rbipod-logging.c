#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

#include "../include/rbipod-logging.h"
#include "../include/rbipod-utils.h"

// =============================================================================
// LOGGING SYSTEM
// =============================================================================

const char* log_level_names[] = {
    "DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL"
};

void log_message(LogLevel level, const char *format, ...) {
    if (!g_sync_ctx.log_file) return;
    
    pthread_mutex_lock(&g_sync_ctx.log_mutex);
    
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm *tm_info = localtime(&tv.tv_sec);
    
    char timestamp[32];
    snprintf(timestamp, sizeof(timestamp), 
             "%04d-%02d-%02d %02d:%02d:%02d.%03ld",
             tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday,
             tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec,
             tv.tv_usec / 1000);
    
    fprintf(g_sync_ctx.log_file, "[%s] [%s] ", timestamp, log_level_names[level]);
    
    va_list args;
    va_start(args, format);
    vfprintf(g_sync_ctx.log_file, format, args);
    va_end(args);
    
    fprintf(g_sync_ctx.log_file, "\n");
    fflush(g_sync_ctx.log_file);
    
    // Also print to console for important messages
    if (level >= LOG_WARNING) {
        printf("[%s] ", log_level_names[level]);
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
        printf("\n");
    }
    
    pthread_mutex_unlock(&g_sync_ctx.log_mutex);
}

gboolean init_logging(const char *log_file_path) {
    g_sync_ctx.log_file = fopen(log_file_path, "a");
    if (!g_sync_ctx.log_file) {
        fprintf(stderr, "Warning: Could not open log file %s\n", log_file_path);
        return FALSE;
    }
    
    pthread_mutex_init(&g_sync_ctx.log_mutex, NULL);
    log_message(LOG_INFO, "=== Logging initialized ===");
    return TRUE;
}

void cleanup_logging(void) {
    if (g_sync_ctx.log_file) {
        log_message(LOG_INFO, "=== Logging shutdown ===");
        fclose(g_sync_ctx.log_file);
        g_sync_ctx.log_file = NULL;
    }
    pthread_mutex_destroy(&g_sync_ctx.log_mutex);
}