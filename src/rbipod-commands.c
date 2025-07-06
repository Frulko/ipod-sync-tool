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
    printf("Total tracks: %d\n", g_list_length(db->itdb->tracks));
    printf("Total playlists: %d\n\n", g_list_length(db->itdb->playlists));
    
    // Show playlists and their track counts
    printf("PLAYLISTS:\n");
    for (GList *pl_item = db->itdb->playlists; pl_item; pl_item = pl_item->next) {
        Itdb_Playlist *pl = (Itdb_Playlist*)pl_item->data;
        const char *playlist_type = "";
        
        if (itdb_playlist_is_mpl(pl)) {
            playlist_type = " (Master)";
        } else if (itdb_playlist_is_podcasts(pl)) {
            playlist_type = " (Podcasts)";
        } else if (pl->podcastflag) {
            playlist_type = " (Podcast-flagged)";
        }
        
        printf("  %-20s: %d tracks%s\n", 
               pl->name ? pl->name : "(Unnamed)", 
               g_list_length(pl->members),
               playlist_type);
    }
    
    // Show tracks by media type
    printf("\nTRACKS BY MEDIA TYPE:\n");
    int audio_count = 0, podcast_count = 0, audiobook_count = 0, video_count = 0, other_count = 0;
    
    for (GList *item = db->itdb->tracks; item; item = item->next) {
        Itdb_Track *track = (Itdb_Track*)item->data;
        
        switch (track->mediatype) {
            case ITDB_MEDIATYPE_AUDIO:
                audio_count++;
                break;
            case ITDB_MEDIATYPE_PODCAST:
                podcast_count++;
                break;
            case ITDB_MEDIATYPE_AUDIOBOOK:
                audiobook_count++;
                break;
            case ITDB_MEDIATYPE_MOVIE:
            case ITDB_MEDIATYPE_MUSICVIDEO:
            case ITDB_MEDIATYPE_TVSHOW:
                video_count++;
                break;
            default:
                other_count++;
                break;
        }
    }
    
    printf("  Audio/Music:     %d\n", audio_count);
    printf("  Podcasts:        %d\n", podcast_count);
    printf("  Audiobooks:      %d\n", audiobook_count);
    printf("  Videos:          %d\n", video_count);
    printf("  Other:           %d\n", other_count);
    
    // Show some recent tracks
    printf("\nRECENT TRACKS (last 10):\n");
    int count = 0;
    for (GList *item = g_list_last(db->itdb->tracks); item && count < 10; item = item->prev, count++) {
        Itdb_Track *track = (Itdb_Track*)item->data;
        const char *media_type_name = "Audio";
        
        switch (track->mediatype) {
            case ITDB_MEDIATYPE_PODCAST: media_type_name = "Podcast"; break;
            case ITDB_MEDIATYPE_AUDIOBOOK: media_type_name = "Audiobook"; break;
            case ITDB_MEDIATYPE_MOVIE: media_type_name = "Movie"; break;
            case ITDB_MEDIATYPE_MUSICVIDEO: media_type_name = "Music Video"; break;
            case ITDB_MEDIATYPE_TVSHOW: media_type_name = "TV Show"; break;
        }
        
        printf("  [%s] %s - %s\n", 
               media_type_name,
               track->artist ? track->artist : "Unknown Artist",
               track->title ? track->title : "Unknown Title");
    }
    
    rb_ipod_db_free(db);
    return 0;
}

int command_show_info(const char *mount_point) {
    log_message(LOG_INFO, "Showing iPod information");
    
    RbIpodDb *db = rb_ipod_db_new(mount_point);
    if (!db) return 1;
    
    printf("=== IPOD INFORMATION ===\n");
    printf("Mount point:      %s\n", mount_point);
    printf("Total tracks:     %d\n", g_list_length(db->itdb->tracks));
    printf("Total playlists:  %d\n", g_list_length(db->itdb->playlists));
    
    // Calculate statistics
    gint64 total_size = 0;
    gint64 total_duration = 0;
    int mp3_count = 0, m4a_count = 0, other_count = 0;
    
    for (GList *item = db->itdb->tracks; item; item = item->next) {
        Itdb_Track *track = (Itdb_Track*)item->data;
        
        total_size += track->size;
        total_duration += track->tracklen;
        
        if (track->filetype) {
            if (g_ascii_strcasecmp(track->filetype, "mp3") == 0) {
                mp3_count++;
            } else if (g_ascii_strcasecmp(track->filetype, "m4a") == 0) {
                m4a_count++;
            } else {
                other_count++;
            }
        } else {
            other_count++;
        }
    }
    
    if (total_duration > 0) {
        int hours = total_duration / (1000 * 3600);
        int minutes = (total_duration % (1000 * 3600)) / (1000 * 60);
        printf("Total duration:   %dh %dm\n", hours, minutes);
    }
    
    if (total_size > 0) {
        printf("Total size:       %.1f MB\n", total_size / (1024.0 * 1024.0));
    }
    
    printf("\nFile formats:\n");
    printf("  MP3:            %d\n", mp3_count);
    printf("  M4A/AAC:        %d\n", m4a_count);
    printf("  Other:          %d\n", other_count);
    
    if (db->itdb->device) {
        printf("\nDevice detected:  Yes\n");
        const Itdb_IpodInfo *info = itdb_device_get_ipod_info(db->itdb->device);
        if (info) {
            printf("Model:            %s\n", 
                   itdb_info_get_ipod_model_name_string(info->ipod_model));
            printf("Generation:       %s\n", 
                   itdb_info_get_ipod_generation_string(info->ipod_generation));
            
            // Add debug information
            printf("Debug - Model enum: %d\n", info->ipod_model);
            printf("Debug - Generation enum: %d\n", info->ipod_generation);
        } else {
            printf("Model:            Unable to detect (no device info)\n");
            printf("Generation:       Unable to detect (no device info)\n");
        }
        
        // Check for SysInfo file
        char sysinfo_path[MAX_PATH_LEN];
        snprintf(sysinfo_path, sizeof(sysinfo_path), "%s/iPod_Control/Device/SysInfo", mount_point);
        struct stat st;
        if (stat(sysinfo_path, &st) == 0) {
            printf("SysInfo file:     Found (%ld bytes)\n", st.st_size);
            if (st.st_size == 0) {
                printf("                  WARNING: SysInfo file is empty - this explains missing device info\n");
            }
        } else {
            printf("SysInfo file:     Not found\n");
        }
    } else {
        printf("\nDevice detected:  No (directory mode)\n");
        printf("Model:            N/A (directory mode)\n");
        printf("Generation:       N/A (directory mode)\n");
    }
    
    rb_ipod_db_free(db);
    return 0;
}

int command_reset_media_type(const char *mount_point, const char *media_type_str) {
    log_message(LOG_INFO, "Starting reset of media type: %s", media_type_str);
    
    if (!media_type_str) {
        fprintf(stderr, "Error: Media type is required\n");
        return 1;
    }
    
    // Parse the media type
    guint32 target_mediatype = parse_media_type_string(media_type_str);
    if (target_mediatype == ITDB_MEDIATYPE_AUDIO && strcmp(media_type_str, "audio") != 0) {
        fprintf(stderr, "Error: Invalid media type '%s'\n", media_type_str);
        fprintf(stderr, "Valid types: audio, movie, podcast, audiobook, musicvideo, tvshow, ringtone, rental, itunes-extra, memo, itunes-u\n");
        return 1;
    }
    
    RbIpodDb *db = rb_ipod_db_new(mount_point);
    if (!db) {
        fprintf(stderr, "Error: Failed to initialize iPod database\n");
        return 1;
    }
    
    printf("=== RESET MEDIA TYPE: %s ===\n", get_media_type_name(target_mediatype));
    printf("WARNING: This will remove ALL tracks of type '%s' from the iPod\n", media_type_str);
    printf("Files will be deleted from the device and removed from database\n");
    printf("Continue? (y/N): ");
    
    // Simple confirmation
    char response;
    if (scanf("%c", &response) != 1 || (response != 'y' && response != 'Y')) {
        printf("Operation cancelled\n");
        rb_ipod_db_free(db);
        return 0;
    }
    
    int removed_tracks = 0;
    int removed_files = 0;
    
    // Create a list of tracks to remove (to avoid modifying list while iterating)
    GList *tracks_to_remove = NULL;
    
    for (GList *item = db->itdb->tracks; item; item = item->next) {
        Itdb_Track *track = (Itdb_Track*)item->data;
        if (track->mediatype == target_mediatype) {
            tracks_to_remove = g_list_prepend(tracks_to_remove, track);
        }
    }
    
    printf("Found %d tracks of type '%s' to remove\n", g_list_length(tracks_to_remove), media_type_str);
    
    // Remove tracks from playlists and database
    for (GList *item = tracks_to_remove; item; item = item->next) {
        Itdb_Track *track = (Itdb_Track*)item->data;
        
        // Store track info before removal (since track memory will be freed)
        char *artist_copy = track->artist ? g_strdup(track->artist) : g_strdup("Unknown Artist");
        char *title_copy = track->title ? g_strdup(track->title) : g_strdup("Unknown Title");
        char *ipod_path_copy = track->ipod_path ? g_strdup(track->ipod_path) : NULL;
        
        // Remove file from iPod if path exists
        if (ipod_path_copy) {
            char full_path[1024];
            snprintf(full_path, sizeof(full_path), "%s%s", mount_point, ipod_path_copy);
            
            if (unlink(full_path) == 0) {
                removed_files++;
                log_message(LOG_DEBUG, "Deleted file: %s", ipod_path_copy);
            } else {
                log_message(LOG_WARNING, "Could not delete file: %s", ipod_path_copy);
            }
        }
        
        // Remove from all playlists
        for (GList *pl_item = db->itdb->playlists; pl_item; pl_item = pl_item->next) {
            Itdb_Playlist *playlist = (Itdb_Playlist*)pl_item->data;
            itdb_playlist_remove_track(playlist, track);
        }
        
        // Remove from main database
        itdb_track_remove(track);
        removed_tracks++;
        
        printf("Removed: %s - %s\n", artist_copy, title_copy);
        
        // Free our copies
        g_free(artist_copy);
        g_free(title_copy);
        if (ipod_path_copy) g_free(ipod_path_copy);
    }
    
    g_list_free(tracks_to_remove);
    
    // Clean up empty playlists if needed
    if (target_mediatype == ITDB_MEDIATYPE_PODCAST) {
        // Remove empty podcasts playlist
        Itdb_Playlist *podcasts_pl = itdb_playlist_podcasts(db->itdb);
        if (podcasts_pl && g_list_length(podcasts_pl->members) == 0) {
            itdb_playlist_remove(podcasts_pl);
            log_message(LOG_INFO, "Removed empty Podcasts playlist");
        }
    }
    
    // Save the database
    if (!rb_ipod_db_save_sync(db)) {
        fprintf(stderr, "Error: Failed to save iPod database\n");
        rb_ipod_db_free(db);
        return 1;
    }
    
    printf("\n=== RESET COMPLETE ===\n");
    printf("Media type:      %s\n", get_media_type_name(target_mediatype));
    printf("Tracks removed:  %d\n", removed_tracks);
    printf("Files deleted:   %d\n", removed_files);
    printf("Database saved:  YES\n");
    
    rb_ipod_db_free(db);
    log_message(LOG_INFO, "Reset completed: %d tracks of type %s removed", removed_tracks, media_type_str);
    return 0;
}

int command_reset_all(const char *mount_point) {
    log_message(LOG_INFO, "Starting complete iPod reset");
    
    RbIpodDb *db = rb_ipod_db_new(mount_point);
    if (!db) {
        fprintf(stderr, "Error: Failed to initialize iPod database\n");
        return 1;
    }
    
    printf("=== COMPLETE IPOD RESET ===\n");
    printf("WARNING: This will remove ALL tracks and playlists from the iPod\n");
    printf("ALL music files will be deleted from the device\n");
    printf("This operation cannot be undone!\n");
    printf("Continue? (y/N): ");
    
    // Simple confirmation
    char response;
    if (scanf("%c", &response) != 1 || (response != 'y' && response != 'Y')) {
        printf("Operation cancelled\n");
        rb_ipod_db_free(db);
        return 0;
    }
    
    int total_tracks = g_list_length(db->itdb->tracks);
    int total_playlists = g_list_length(db->itdb->playlists);
    int removed_tracks = 0;
    int removed_files = 0;
    int removed_playlists = 0;
    
    printf("Found %d tracks and %d playlists to remove\n", total_tracks, total_playlists);
    
    // Create a list of tracks to remove (to avoid modifying list while iterating)
    GList *tracks_to_remove = NULL;
    for (GList *item = db->itdb->tracks; item; item = item->next) {
        tracks_to_remove = g_list_prepend(tracks_to_remove, item->data);
    }
    
    // Remove all tracks and their files
    for (GList *item = tracks_to_remove; item; item = item->next) {
        Itdb_Track *track = (Itdb_Track*)item->data;
        
        // Store track info before removal
        char *artist_copy = track->artist ? g_strdup(track->artist) : g_strdup("Unknown Artist");
        char *title_copy = track->title ? g_strdup(track->title) : g_strdup("Unknown Title");
        char *ipod_path_copy = track->ipod_path ? g_strdup(track->ipod_path) : NULL;
        
        // Remove file from iPod if path exists
        if (ipod_path_copy) {
            char full_path[1024];
            snprintf(full_path, sizeof(full_path), "%s%s", mount_point, ipod_path_copy);
            
            if (unlink(full_path) == 0) {
                removed_files++;
                log_message(LOG_DEBUG, "Deleted file: %s", ipod_path_copy);
            } else {
                log_message(LOG_DEBUG, "Could not delete file: %s (may not exist)", ipod_path_copy);
            }
        }
        
        // Remove from all playlists
        for (GList *pl_item = db->itdb->playlists; pl_item; pl_item = pl_item->next) {
            Itdb_Playlist *playlist = (Itdb_Playlist*)pl_item->data;
            itdb_playlist_remove_track(playlist, track);
        }
        
        // Remove from main database
        itdb_track_remove(track);
        removed_tracks++;
        
        if (removed_tracks % 10 == 0 || removed_tracks == total_tracks) {
            printf("Progress: %d/%d tracks removed\n", removed_tracks, total_tracks);
        }
        
        // Free our copies
        g_free(artist_copy);
        g_free(title_copy);
        if (ipod_path_copy) g_free(ipod_path_copy);
    }
    
    g_list_free(tracks_to_remove);
    
    // Remove all non-master playlists
    GList *playlists_to_remove = NULL;
    for (GList *pl_item = db->itdb->playlists; pl_item; pl_item = pl_item->next) {
        Itdb_Playlist *playlist = (Itdb_Playlist*)pl_item->data;
        // Keep only the master playlist
        if (!itdb_playlist_is_mpl(playlist)) {
            playlists_to_remove = g_list_prepend(playlists_to_remove, playlist);
        }
    }
    
    for (GList *item = playlists_to_remove; item; item = item->next) {
        Itdb_Playlist *playlist = (Itdb_Playlist*)item->data;
        log_message(LOG_DEBUG, "Removing playlist: %s", playlist->name ? playlist->name : "(Unnamed)");
        itdb_playlist_remove(playlist);
        removed_playlists++;
    }
    
    g_list_free(playlists_to_remove);
    
    // Clean up empty music directories
    char music_dir[1024];
    snprintf(music_dir, sizeof(music_dir), "%s/iPod_Control/Music", mount_point);
    
    // Try to remove all F** directories
    for (int i = 0; i < 100; i++) {
        char subdir[1024];
        snprintf(subdir, sizeof(subdir), "%s/F%02d", music_dir, i);
        
        // Remove all files in the directory first
        char command[2048];
        snprintf(command, sizeof(command), "rm -f \"%s\"/* 2>/dev/null", subdir);
        system(command);
        
        // Try to remove the directory (will only work if empty)
        if (rmdir(subdir) == 0) {
            log_message(LOG_DEBUG, "Removed empty directory: %s", subdir);
        }
    }
    
    // Save the cleaned database
    if (!rb_ipod_db_save_sync(db)) {
        fprintf(stderr, "Error: Failed to save iPod database\n");
        rb_ipod_db_free(db);
        return 1;
    }
    
    printf("\n=== COMPLETE RESET FINISHED ===\n");
    printf("Tracks removed:    %d\n", removed_tracks);
    printf("Files deleted:     %d\n", removed_files);
    printf("Playlists removed: %d\n", removed_playlists);
    printf("Database cleared:  YES\n");
    printf("iPod is now completely clean\n");
    
    rb_ipod_db_free(db);
    log_message(LOG_INFO, "Complete reset finished: %d tracks, %d files, %d playlists removed", 
               removed_tracks, removed_files, removed_playlists);
    return 0;
}