#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <signal.h>
#include <glib.h>

#include "../include/rbipod-sync.h"
#include "../include/rbipod-files.h"
#include "../include/rbipod-logging.h"
#include "../include/rbipod-utils.h"

// =============================================================================
// SYNCHRONIZATION OPERATIONS (STUB IMPLEMENTATION)
// =============================================================================

int count_audio_files_recursive(const char *dir_path) {
    if (!dir_path) return 0;
    
    DIR *dir = opendir(dir_path);
    if (!dir) return 0;
    
    int count = 0;
    struct dirent *entry;
    
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
        
        struct stat file_stat;
        if (stat(full_path, &file_stat) != 0) continue;
        
        if (S_ISDIR(file_stat.st_mode)) {
            count += count_audio_files_recursive(full_path);
        } else if (S_ISREG(file_stat.st_mode) && is_supported_audio_file(entry->d_name)) {
            count++;
        }
    }
    
    closedir(dir);
    return count;
}

gboolean sync_directory_recursive(RbIpodDb *db, const char *dir_path, int *current_file, int total_files) {
    if (!db || !dir_path) return FALSE;
    
    DIR *dir = opendir(dir_path);
    if (!dir) {
        log_message(LOG_ERROR, "Cannot open directory: %s", dir_path);
        return FALSE;
    }
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (g_sync_ctx.cancellation_requested) {
            printf("\nSync cancelled by user\n");
            closedir(dir);
            return FALSE;
        }
        
        if (entry->d_name[0] == '.') continue;
        
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
        
        struct stat file_stat;
        if (stat(full_path, &file_stat) != 0) continue;
        
        if (S_ISDIR(file_stat.st_mode)) {
            if (!sync_directory_recursive(db, full_path, current_file, total_files)) {
                closedir(dir);
                return FALSE;
            }
        } else if (S_ISREG(file_stat.st_mode) && is_supported_audio_file(entry->d_name)) {
            if (current_file && total_files) {
                (*current_file)++;
                int progress = ((*current_file) * 100) / total_files;
                printf("\rProgress: %d%% (%d/%d)", progress, *current_file, total_files);
                fflush(stdout);
            }
            
            add_file_to_ipod(db, full_path);
        }
    }
    
    closedir(dir);
    return TRUE;
}

gboolean sync_single_file(RbIpodDb *db, const char *file_path) {
    if (!file_path || !db) {
        log_message(LOG_ERROR, "sync_single_file called with NULL parameters");
        return FALSE;
    }
    
    struct stat file_stat;
    if (stat(file_path, &file_stat) != 0) {
        log_message(LOG_ERROR, "Cannot access file: %s", file_path);
        return FALSE;
    }
    
    if (!S_ISREG(file_stat.st_mode)) {
        log_message(LOG_ERROR, "Path is not a regular file: %s", file_path);
        return FALSE;
    }
    
    char *filename = g_path_get_basename(file_path);
    if (!is_supported_audio_file(filename)) {
        log_message(LOG_ERROR, "Unsupported file type: %s", file_path);
        g_free(filename);
        return FALSE;
    }
    g_free(filename);
    
    printf("Syncing single file: %s\n", file_path);
    
    gboolean result = add_file_to_ipod(db, file_path);
    if (result) {
        printf("Successfully synced: %s\n", file_path);
    } else {
        printf("Failed to sync: %s\n", file_path);
    }
    
    return result;
}

gboolean sync_folder_filtered(RbIpodDb *db, const char *dir_path, 
                              guint32 filter_mediatype, 
                              int *current_file, int total_files) {
    if (!db || !dir_path) {
        log_message(LOG_ERROR, "sync_folder_filtered called with NULL parameters");
        return FALSE;
    }
    
    DIR *dir = opendir(dir_path);
    if (!dir) {
        log_message(LOG_ERROR, "Cannot open directory: %s", dir_path);
        return FALSE;
    }
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (g_sync_ctx.cancellation_requested) {
            printf("\nSync cancelled by user\n");
            closedir(dir);
            return FALSE;
        }
        
        if (entry->d_name[0] == '.') continue;
        
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
        
        struct stat file_stat;
        if (stat(full_path, &file_stat) != 0) continue;
        
        if (S_ISDIR(file_stat.st_mode)) {
            if (!sync_folder_filtered(db, full_path, filter_mediatype, current_file, total_files)) {
                closedir(dir);
                return FALSE;
            }
        } else if (S_ISREG(file_stat.st_mode) && is_supported_audio_file(entry->d_name)) {
            if (current_file && total_files) {
                (*current_file)++;
                int progress = ((*current_file) * 100) / total_files;
                printf("\rProgress: %d%% (%d/%d)", progress, *current_file, total_files);
                fflush(stdout);
            }
            
            // Temporarily override media type for this file
            guint32 saved_force_mediatype = g_sync_ctx.force_mediatype;
            gboolean saved_use_force_mediatype = g_sync_ctx.use_force_mediatype;
            
            g_sync_ctx.force_mediatype = filter_mediatype;
            g_sync_ctx.use_force_mediatype = TRUE;
            
            gboolean result = add_file_to_ipod(db, full_path);
            
            // Restore original settings
            g_sync_ctx.force_mediatype = saved_force_mediatype;
            g_sync_ctx.use_force_mediatype = saved_use_force_mediatype;
            
            if (!result) {
                printf("\nFailed to add file: %s\n", full_path);
            }
        }
    }
    
    closedir(dir);
    return TRUE;
}

// Signal handling for graceful shutdown
static void signal_handler(int signal) {
    switch (signal) {
        case SIGINT:
        case SIGTERM:
            printf("\nReceived signal %d, requesting graceful shutdown...\n", signal);
            g_sync_ctx.cancellation_requested = TRUE;
            break;
    }
}

void setup_signal_handlers(void) {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
}