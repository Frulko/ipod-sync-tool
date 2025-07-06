#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <glib.h>
#include <gpod/itdb.h>

#include "../include/rbipod-commands.h"
#include "../include/rbipod-database.h"
#include "../include/rbipod-filesystem.h"
#include "../include/rbipod-sync.h"
#include "../include/rbipod-metadata.h"
#include "../include/rbipod-logging.h"
#include "../include/rbipod-utils.h"

// =============================================================================
// COMMAND IMPLEMENTATIONS (STUB IMPLEMENTATION)
// =============================================================================

int command_mount_ipod(const char *device_path, const char *mount_point) {
    log_message(LOG_INFO, "Mounting iPod: device=%s, mount_point=%s", 
               device_path ? device_path : "auto", mount_point);
    
    FilesystemType fs_type = detect_filesystem_type(device_path);
    
    if (mount_ipod_device(device_path, mount_point, fs_type)) {
        printf("Successfully mounted iPod at %s\n", mount_point);
        return 0;
    } else {
        fprintf(stderr, "Failed to mount iPod\n");
        return 1;
    }
}

int command_unmount_ipod(const char *mount_point) {
    log_message(LOG_INFO, "Unmounting iPod at %s", mount_point);
    
    if (unmount_ipod_device(mount_point)) {
        printf("Successfully unmounted iPod from %s\n", mount_point);
        return 0;
    } else {
        fprintf(stderr, "Failed to unmount iPod\n");
        return 1;
    }
}

int command_auto_mount(void) {
    log_message(LOG_INFO, "Auto-mounting iPod");
    
    char *mount_point = NULL;
    if (auto_mount_ipod(&mount_point)) {
        printf("Successfully auto-mounted iPod at %s\n", mount_point);
        g_free(mount_point);
        return 0;
    } else {
        fprintf(stderr, "Failed to auto-mount iPod\n");
        return 1;
    }
}

int command_sync_directory(const char *mount_point, const char *sync_dir) {
    log_message(LOG_INFO, "Starting directory sync from %s to %s", sync_dir, mount_point);
    
    // Validate sync directory
    struct stat sync_stat;
    if (stat(sync_dir, &sync_stat) != 0 || !S_ISDIR(sync_stat.st_mode)) {
        fprintf(stderr, "Error: Sync directory does not exist or is not a directory: %s\n", sync_dir);
        return 1;
    }
    
    // Initialize database
    g_sync_ctx.ipod_db = rb_ipod_db_new(mount_point);
    if (!g_sync_ctx.ipod_db) {
        fprintf(stderr, "Error: Failed to initialize iPod database\n");
        return 1;
    }
    
    // Reset statistics
    memset(&g_sync_ctx.stats, 0, sizeof(g_sync_ctx.stats));
    
    time_t start_time = time(NULL);
    int tracks_before = itdb_tracks_number(g_sync_ctx.ipod_db->itdb);
    
    // Count files for progress tracking
    printf("Counting files in %s...\n", sync_dir);
    int total_files = count_audio_files_recursive(sync_dir);
    
    if (total_files == 0) {
        printf("No audio files found in %s\n", sync_dir);
        rb_ipod_db_free(g_sync_ctx.ipod_db);
        g_sync_ctx.ipod_db = NULL;
        return 0;
    }
    
    printf("Found %d audio files to sync\n", total_files);
    
    // Perform sync
    int current_file = 0;
    gboolean success = sync_directory_recursive(g_sync_ctx.ipod_db, sync_dir, &current_file, total_files);
    
    time_t end_time = time(NULL);
    int tracks_after = itdb_tracks_number(g_sync_ctx.ipod_db->itdb);
    
    // Save database
    if (!rb_ipod_db_save_sync(g_sync_ctx.ipod_db)) {
        fprintf(stderr, "Error: Failed to save iPod database\n");
        rb_ipod_db_free(g_sync_ctx.ipod_db);
        g_sync_ctx.ipod_db = NULL;
        return 1;
    }
    
    // Report results
    printf("\n=== Sync Complete ===\n");
    printf("Result: %s\n", success ? "SUCCESS" : "FAILED");
    printf("Files added: %d\n", g_sync_ctx.stats.files_added);
    printf("Files skipped: %d\n", g_sync_ctx.stats.files_skipped);
    printf("Files failed: %d\n", g_sync_ctx.stats.files_failed);
    printf("Tracks before: %d\n", tracks_before);
    printf("Tracks after: %d\n", tracks_after);
    printf("Duration: %ld seconds\n", end_time - start_time);
    
    rb_ipod_db_free(g_sync_ctx.ipod_db);
    g_sync_ctx.ipod_db = NULL;
    return success ? 0 : 1;
}

int command_sync_file(const char *mount_point, const char *file_path) {
    log_message(LOG_INFO, "Starting single file sync from %s to %s", file_path, mount_point);
    
    if (!file_path || access(file_path, F_OK) != 0) {
        fprintf(stderr, "Error: File does not exist: %s\n", file_path);
        return 1;
    }
    
    g_sync_ctx.ipod_db = rb_ipod_db_new(mount_point);
    if (!g_sync_ctx.ipod_db) {
        fprintf(stderr, "Error: Failed to initialize iPod database\n");
        return 1;
    }
    
    memset(&g_sync_ctx.stats, 0, sizeof(g_sync_ctx.stats));
    
    time_t start_time = time(NULL);
    int tracks_before = itdb_tracks_number(g_sync_ctx.ipod_db->itdb);
    
    printf("Synchronizing file: %s\n", file_path);
    
    gboolean success = sync_single_file(g_sync_ctx.ipod_db, file_path);
    
    time_t end_time = time(NULL);
    int tracks_after = itdb_tracks_number(g_sync_ctx.ipod_db->itdb);
    
    if (!rb_ipod_db_save_sync(g_sync_ctx.ipod_db)) {
        fprintf(stderr, "Error: Failed to save iPod database\n");
        rb_ipod_db_free(g_sync_ctx.ipod_db);
        g_sync_ctx.ipod_db = NULL;
        return 1;
    }
    
    printf("\n=== Sync Complete ===\n");
    printf("Result: %s\n", success ? "SUCCESS" : "FAILED");
    printf("Files added: %d\n", g_sync_ctx.stats.files_added);
    printf("Files skipped: %d\n", g_sync_ctx.stats.files_skipped);
    printf("Files failed: %d\n", g_sync_ctx.stats.files_failed);
    printf("Tracks before: %d\n", tracks_before);
    printf("Tracks after: %d\n", tracks_after);
    printf("Duration: %ld seconds\n", end_time - start_time);
    
    rb_ipod_db_free(g_sync_ctx.ipod_db);
    g_sync_ctx.ipod_db = NULL;
    return success ? 0 : 1;
}

int command_sync_folder_filtered(const char *mount_point, const char *folder_path, const char *mediatype_str) {
    log_message(LOG_INFO, "Starting filtered folder sync from %s to %s with media type %s", 
                folder_path, mount_point, mediatype_str ? mediatype_str : "default");
    
    if (!folder_path || access(folder_path, F_OK) != 0) {
        fprintf(stderr, "Error: Folder does not exist: %s\n", folder_path);
        return 1;
    }
    
    struct stat folder_stat;
    if (stat(folder_path, &folder_stat) != 0 || !S_ISDIR(folder_stat.st_mode)) {
        fprintf(stderr, "Error: Path is not a directory: %s\n", folder_path);
        return 1;
    }
    
    guint32 filter_mediatype = ITDB_MEDIATYPE_AUDIO;
    if (mediatype_str) {
        filter_mediatype = parse_media_type_string(mediatype_str);
        if (filter_mediatype == ITDB_MEDIATYPE_AUDIO && strcmp(mediatype_str, "audio") != 0) {
            fprintf(stderr, "Error: Invalid media type '%s'\n", mediatype_str);
            fprintf(stderr, "Valid types: audio, movie, podcast, audiobook, musicvideo, tvshow, ringtone, rental, itunes-extra, memo, itunes-u\n");
            return 1;
        }
    }
    
    g_sync_ctx.ipod_db = rb_ipod_db_new(mount_point);
    if (!g_sync_ctx.ipod_db) {
        fprintf(stderr, "Error: Failed to initialize iPod database\n");
        return 1;
    }
    
    memset(&g_sync_ctx.stats, 0, sizeof(g_sync_ctx.stats));
    
    time_t start_time = time(NULL);
    int tracks_before = itdb_tracks_number(g_sync_ctx.ipod_db->itdb);
    
    printf("Counting files in %s...\n", folder_path);
    int total_files = count_audio_files_recursive(folder_path);
    
    if (total_files == 0) {
        printf("No audio files found in %s\n", folder_path);
        rb_ipod_db_free(g_sync_ctx.ipod_db);
        g_sync_ctx.ipod_db = NULL;
        return 0;
    }
    
    printf("Found %d audio files to sync with media type: %s\n", total_files, get_media_type_name(filter_mediatype));
    
    int current_file = 0;
    gboolean success = sync_folder_filtered(g_sync_ctx.ipod_db, folder_path, filter_mediatype, &current_file, total_files);
    
    time_t end_time = time(NULL);
    int tracks_after = itdb_tracks_number(g_sync_ctx.ipod_db->itdb);
    
    if (!rb_ipod_db_save_sync(g_sync_ctx.ipod_db)) {
        fprintf(stderr, "Error: Failed to save iPod database\n");
        rb_ipod_db_free(g_sync_ctx.ipod_db);
        g_sync_ctx.ipod_db = NULL;
        return 1;
    }
    
    printf("\n=== Sync Complete ===\n");
    printf("Result: %s\n", success ? "SUCCESS" : "FAILED");
    printf("Media type: %s\n", get_media_type_name(filter_mediatype));
    printf("Files added: %d\n", g_sync_ctx.stats.files_added);
    printf("Files skipped: %d\n", g_sync_ctx.stats.files_skipped);
    printf("Files failed: %d\n", g_sync_ctx.stats.files_failed);
    printf("Tracks before: %d\n", tracks_before);
    printf("Tracks after: %d\n", tracks_after);
    printf("Duration: %ld seconds\n", end_time - start_time);
    
    rb_ipod_db_free(g_sync_ctx.ipod_db);
    g_sync_ctx.ipod_db = NULL;
    return success ? 0 : 1;
}

int command_list_tracks(const char *mount_point) {
    log_message(LOG_INFO, "Listing iPod tracks");
    
    RbIpodDb *db = rb_ipod_db_new(mount_point);
    if (!db) return 1;
    
    printf("=== IPOD TRACK LISTING ===\n");
    printf("Total tracks: %d\n\n", g_list_length(db->itdb->tracks));
    
    // TODO: Implement detailed track listing
    printf("Track listing not yet implemented\n");
    
    rb_ipod_db_free(db);
    return 0;
}

int command_show_info(const char *mount_point) {
    log_message(LOG_INFO, "Showing iPod information");
    
    RbIpodDb *db = rb_ipod_db_new(mount_point);
    if (!db) return 1;
    
    printf("=== IPOD INFORMATION ===\n");
    printf("Mount point: %s\n", mount_point);
    
    if (db->itdb) {
        printf("Total tracks: %d\n", g_list_length(db->itdb->tracks));
        printf("Total playlists: %d\n", g_list_length(db->itdb->playlists));
        
        // TODO: Show more detailed device information
        printf("Device information display not yet fully implemented\n");
    }
    
    rb_ipod_db_free(db);
    return 0;
}