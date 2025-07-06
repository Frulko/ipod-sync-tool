#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <glib.h>
#include <gpod/itdb.h>

#include "../include/rbipod-files.h"
#include "../include/rbipod-logging.h"
#include "../include/rbipod-metadata.h"
#include "../include/rbipod-database.h"
#include "../include/rbipod-utils.h"

// =============================================================================
// FILE OPERATIONS AND TRACK MANAGEMENT (STUB IMPLEMENTATION)
// =============================================================================

char* build_ipod_dir_name(const char *mount_point) {
    if (!mount_point) return NULL;
    
    // TODO: Implement proper iPod directory naming
    return g_strdup_printf("%s/iPod_Control/Music", mount_point);
}

char* utf8_to_ascii(const char *utf8_string) {
    if (!utf8_string) return NULL;
    
    // TODO: Implement proper UTF-8 to ASCII conversion
    // For now, just duplicate the string
    return g_strdup(utf8_string);
}

char* generate_ipod_filename(const char *mount_point, const char *original_filename) {
    if (!mount_point || !original_filename) return NULL;
    
    // Generate a proper iPod path using F00 directory structure
    char *basename = g_path_get_basename(original_filename);
    char *result = g_strdup_printf("%s/iPod_Control/Music/F00/%s", mount_point, basename);
    
    g_free(basename);
    return result;
}

gboolean ensure_ipod_directory_structure(const char *mount_point) {
    if (!mount_point) return FALSE;
    
    char ipod_control[1024];
    char music_dir[1024];
    char f00_dir[1024];
    
    snprintf(ipod_control, sizeof(ipod_control), "%s/iPod_Control", mount_point);
    snprintf(music_dir, sizeof(music_dir), "%s/iPod_Control/Music", mount_point);
    snprintf(f00_dir, sizeof(f00_dir), "%s/iPod_Control/Music/F00", mount_point);
    
    // Create iPod_Control directory
    if (mkdir(ipod_control, 0755) != 0 && errno != EEXIST) {
        log_message(LOG_ERROR, "Failed to create iPod_Control directory: %s", strerror(errno));
        return FALSE;
    }
    
    // Create Music directory
    if (mkdir(music_dir, 0755) != 0 && errno != EEXIST) {
        log_message(LOG_ERROR, "Failed to create Music directory: %s", strerror(errno));
        return FALSE;
    }
    
    // Create F00 subdirectory (iPods use F00, F01, etc. for organization)
    if (mkdir(f00_dir, 0755) != 0 && errno != EEXIST) {
        log_message(LOG_ERROR, "Failed to create F00 directory: %s", strerror(errno));
        return FALSE;
    }
    
    log_message(LOG_DEBUG, "iPod directory structure ensured");
    return TRUE;
}

gboolean copy_file_to_ipod(const char *source_path, const char *dest_path) {
    if (!source_path || !dest_path) return FALSE;
    
    log_message(LOG_INFO, "Copying file from %s to %s", source_path, dest_path);
    
    // Check if source exists
    struct stat source_stat;
    if (stat(source_path, &source_stat) != 0) {
        log_message(LOG_ERROR, "Source file does not exist: %s", source_path);
        return FALSE;
    }
    
    // Ensure destination directory exists
    char *dest_dir = g_path_get_dirname(dest_path);
    if (mkdir(dest_dir, 0755) != 0 && errno != EEXIST) {
        log_message(LOG_WARNING, "Could not create destination directory: %s", dest_dir);
    }
    g_free(dest_dir);
    
    // Open source file
    FILE *source = fopen(source_path, "rb");
    if (!source) {
        log_message(LOG_ERROR, "Cannot open source file: %s", source_path);
        return FALSE;
    }
    
    // Open destination file
    FILE *dest = fopen(dest_path, "wb");
    if (!dest) {
        log_message(LOG_ERROR, "Cannot create destination file: %s", dest_path);
        fclose(source);
        return FALSE;
    }
    
    // Copy file in chunks
    char buffer[8192];
    size_t bytes_read;
    gboolean success = TRUE;
    
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), source)) > 0) {
        if (fwrite(buffer, 1, bytes_read, dest) != bytes_read) {
            log_message(LOG_ERROR, "Error writing to destination file: %s", dest_path);
            success = FALSE;
            break;
        }
    }
    
    // Check for read errors
    if (ferror(source)) {
        log_message(LOG_ERROR, "Error reading from source file: %s", source_path);
        success = FALSE;
    }
    
    fclose(source);
    fclose(dest);
    
    if (success) {
        log_message(LOG_DEBUG, "File copied successfully: %s -> %s", source_path, dest_path);
    } else {
        // Clean up failed copy
        unlink(dest_path);
    }
    
    return success;
}

gboolean is_supported_audio_file(const char *filename) {
    if (!filename) return FALSE;
    
    const char *ext = strrchr(filename, '.');
    if (!ext) return FALSE;
    
    ext++; // Skip the dot
    
    return (g_ascii_strcasecmp(ext, "mp3") == 0 ||
            g_ascii_strcasecmp(ext, "m4a") == 0 ||
            g_ascii_strcasecmp(ext, "aac") == 0 ||
            g_ascii_strcasecmp(ext, "wav") == 0 ||
            g_ascii_strcasecmp(ext, "aiff") == 0 ||
            g_ascii_strcasecmp(ext, "m4p") == 0 ||
            g_ascii_strcasecmp(ext, "mp4") == 0);
}

gboolean extract_audio_duration_mediainfo(const char *file_path, int *duration, int *bitrate) {
    if (!file_path || !duration || !bitrate) return FALSE;
    
    *duration = 0;
    *bitrate = 0;
    
    // Use mediainfo to extract precise metadata
    char command[2048];
    snprintf(command, sizeof(command), 
             "mediainfo --Output='Duration=%%Duration%%;Bitrate=%%BitRate%%' \"%s\" 2>/dev/null", 
             file_path);
    
    FILE *pipe = popen(command, "r");
    if (!pipe) {
        log_message(LOG_WARNING, "Failed to run mediainfo command");
        return FALSE;
    }
    
    char output[512];
    if (fgets(output, sizeof(output), pipe)) {
        // Parse mediainfo output format: Duration=XXXX;Bitrate=YYYY
        char *duration_str = strstr(output, "Duration=");
        char *bitrate_str = strstr(output, "Bitrate=");
        
        if (duration_str) {
            duration_str += 9; // Skip "Duration="
            long duration_ms = strtol(duration_str, NULL, 10);
            *duration = duration_ms / 1000; // Convert milliseconds to seconds
        }
        
        if (bitrate_str) {
            bitrate_str += 8; // Skip "Bitrate="
            *bitrate = strtol(bitrate_str, NULL, 10) / 1000; // Convert bps to kbps
        }
        
        log_message(LOG_DEBUG, "Mediainfo extracted: %s -> duration: %d sec, bitrate: %d kbps", 
                   file_path, *duration, *bitrate);
    }
    
    pclose(pipe);
    
    // If mediainfo failed, try alternative approach with ffprobe
    if (*duration == 0) {
        snprintf(command, sizeof(command), 
                 "ffprobe -v quiet -show_entries format=duration,bit_rate -of csv=p=0 \"%s\" 2>/dev/null", 
                 file_path);
        
        pipe = popen(command, "r");
        if (pipe) {
            if (fgets(output, sizeof(output), pipe)) {
                // Parse ffprobe CSV output: duration,bit_rate
                char *token = strtok(output, ",");
                if (token) {
                    double duration_float = strtod(token, NULL);
                    *duration = (int)duration_float;
                    
                    token = strtok(NULL, ",");
                    if (token) {
                        *bitrate = strtol(token, NULL, 10) / 1000; // Convert bps to kbps
                    }
                }
                
                log_message(LOG_DEBUG, "FFprobe extracted: %s -> duration: %d sec, bitrate: %d kbps", 
                           file_path, *duration, *bitrate);
            }
            pclose(pipe);
        }
    }
    
    // Set reasonable defaults if extraction failed
    if (*duration == 0) {
        // Try to estimate from file size as last resort
        FILE *file = fopen(file_path, "rb");
        if (file) {
            fseek(file, 0, SEEK_END);
            long file_size = ftell(file);
            fclose(file);
            
            if (file_size > 1024) {
                *bitrate = 128; // Default bitrate
                *duration = (file_size * 8) / (*bitrate * 1000); // Rough estimation
                log_message(LOG_DEBUG, "Using file size estimation: %d seconds", *duration);
            }
        }
    }
    
    return (*duration > 0);
}

gboolean extract_audio_duration(const char *file_path, int *duration, int *bitrate) {
    // Use the new robust extraction method
    return extract_audio_duration_mediainfo(file_path, duration, bitrate);
}

gboolean extract_metadata_field(const char *file_path, const char *field_name, char **result) {
    if (!file_path || !field_name || !result) return FALSE;
    
    char command[1024];
    snprintf(command, sizeof(command), "mediainfo --Inform=\"General;%%%s%%\" \"%s\" 2>/dev/null", field_name, file_path);
    
    FILE *pipe = popen(command, "r");
    if (!pipe) return FALSE;
    
    char output[512];
    gboolean success = FALSE;
    
    if (fgets(output, sizeof(output), pipe)) {
        // Remove trailing newline
        char *newline = strchr(output, '\n');
        if (newline) *newline = '\0';
        
        // Only set if we got a non-empty result
        if (strlen(output) > 0) {
            *result = g_strdup(output);
            success = TRUE;
        }
    }
    
    pclose(pipe);
    return success;
}

gboolean extract_audio_metadata_full(const char *file_path, AudioMetadata *meta) {
    if (!file_path || !meta) return FALSE;
    
    gboolean any_success = FALSE;
    
    // Extract individual fields using mediainfo
    char *temp_str = NULL;
    
    // Extract title (Track name)
    if (extract_metadata_field(file_path, "Track name", &temp_str)) {
        g_free(meta->title);
        meta->title = temp_str;
        temp_str = NULL;
        any_success = TRUE;
    }
    
    // Extract artist (Performer)  
    if (extract_metadata_field(file_path, "Performer", &temp_str)) {
        g_free(meta->artist);
        meta->artist = temp_str;
        temp_str = NULL;
        any_success = TRUE;
    }
    
    // Extract album
    if (extract_metadata_field(file_path, "Album", &temp_str)) {
        g_free(meta->album);
        meta->album = temp_str;
        temp_str = NULL;
        any_success = TRUE;
    }
    
    // Extract genre
    if (extract_metadata_field(file_path, "Genre", &temp_str)) {
        g_free(meta->genre);
        meta->genre = temp_str;
        temp_str = NULL;
        any_success = TRUE;
    }
    
    // Extract description (USLT - unsynchronized lyrics/description)
    if (extract_metadata_field(file_path, "USLT", &temp_str)) {
        g_free(meta->description);
        meta->description = temp_str;
        temp_str = NULL;
        any_success = TRUE;
    }
    
    // Extract duration and bitrate using separate command
    char command[1024];
    snprintf(command, sizeof(command), "mediainfo --Inform=\"General;%%Duration%%,%%OverallBitRate%%\" \"%s\" 2>/dev/null", file_path);
    
    FILE *pipe = popen(command, "r");
    if (pipe) {
        char output[256];
        if (fgets(output, sizeof(output), pipe)) {
            // Parse duration,bitrate format
            char *token = strtok(output, ",");
            if (token) {
                long duration_ms = strtol(token, NULL, 10);
                meta->duration = duration_ms / 1000;  // Convert ms to seconds
                
                token = strtok(NULL, ",");
                if (token) {
                    meta->bitrate = strtol(token, NULL, 10) / 1000;  // Convert bps to kbps
                }
                any_success = TRUE;
            }
        }
        pclose(pipe);
    }
    
    log_message(LOG_DEBUG, "Extracted metadata: Title='%s', Artist='%s', Album='%s', Duration=%d sec, Bitrate=%d kbps", 
               meta->title ? meta->title : "N/A", 
               meta->artist ? meta->artist : "N/A", 
               meta->album ? meta->album : "N/A", 
               meta->duration, meta->bitrate);
    
    // If duration extraction failed, try the fallback method
    if (meta->duration == 0) {
        int duration = 0, bitrate = 0;
        if (extract_audio_duration(file_path, &duration, &bitrate)) {
            meta->duration = duration;
            if (meta->bitrate == 0) meta->bitrate = bitrate;
            any_success = TRUE;
        }
    }
    
    return any_success;
}

gboolean probe_audio_file(const char *file_path, AudioMetadata *meta) {
    if (!file_path || !meta) return FALSE;
    
    // Use the comprehensive metadata extraction
    if (extract_audio_metadata_full(file_path, meta)) {
        log_message(LOG_DEBUG, "Full metadata extraction successful for: %s", file_path);
        return TRUE;
    }
    
    // Fallback to duration-only extraction
    int duration = 0, bitrate = 0;
    if (extract_audio_duration(file_path, &duration, &bitrate)) {
        meta->duration = duration;
        meta->bitrate = bitrate;
        
        log_message(LOG_DEBUG, "Fallback duration extraction: %s -> %d sec, %d kbps", 
                   file_path, duration, bitrate);
        return TRUE;
    }
    
    return FALSE;
}

Itdb_Track* create_ipod_track_from_metadata(const AudioMetadata *meta, const char *ipod_path, const char *media_type) {
    if (!meta || !ipod_path) return NULL;
    
    Itdb_Track *track = itdb_track_new();
    if (!track) return NULL;
    
    // Set basic metadata
    track->title = g_strdup(meta->title ? meta->title : "Unknown Title");
    track->artist = g_strdup(meta->artist ? meta->artist : "Unknown Artist");
    track->album = g_strdup(meta->album ? meta->album : "Unknown Album");
    track->genre = g_strdup(meta->genre ? meta->genre : "Unknown");
    
    if (meta->composer) track->composer = g_strdup(meta->composer);
    if (meta->albumartist) track->albumartist = g_strdup(meta->albumartist);
    
    // Set track numbers and metadata
    track->track_nr = meta->track_number;
    track->cd_nr = meta->disc_number;
    track->year = meta->year;
    track->bitrate = meta->bitrate;
    track->tracklen = meta->duration * 1000; // Convert to milliseconds
    track->size = meta->file_size;
    track->rating = (guint32)(meta->rating * 20); // Convert to 0-100 scale
    track->playcount = meta->play_count;
    
    // Timestamps
    track->time_added = meta->time_added;
    track->time_modified = meta->time_added;
    track->time_played = meta->time_played;
    
    // File information - iPod path should be relative to mount point
    // Extract the relative path from the full path
    const char *relative_path = ipod_path;
    if (g_str_has_prefix(ipod_path, "/media/ipod")) {
        relative_path = ipod_path + strlen("/media/ipod");
    }
    track->ipod_path = g_strdup(relative_path);
    track->mediatype = meta->mediatype;
    
    // Set file type based on extension
    if (media_type) {
        if (g_str_has_suffix(media_type, "mp3")) {
            track->filetype = g_strdup("mp3");
        } else if (g_str_has_suffix(media_type, "m4a") || g_str_has_suffix(media_type, "aac")) {
            track->filetype = g_strdup("m4a");
        } else {
            track->filetype = g_strdup("mp3"); // Default fallback
        }
    }
    
    // Set podcast-specific attributes for proper playback
    if (track->mediatype == ITDB_MEDIATYPE_PODCAST) {
        // Essential podcast flags for iPod firmware
        track->flag4 = 0x01;  // Podcast display flag
        track->mark_unplayed = meta->mark_unplayed ? 0x02 : 0x01;  // New episode marker
        
        // Podcast URLs and metadata
        if (meta->podcasturl) track->podcasturl = g_strdup(meta->podcasturl);
        if (meta->podcastrss) track->podcastrss = g_strdup(meta->podcastrss);
        if (meta->description) track->description = g_strdup(meta->description);
        if (meta->subtitle) track->subtitle = g_strdup(meta->subtitle);
        if (meta->category) track->category = g_strdup(meta->category);
        
        // Release date for podcast episodes
        if (meta->time_released > 0) {
            track->time_released = meta->time_released;
        }
        
        // Podcast behavior settings
        track->remember_playback_position = TRUE;
        track->skip_when_shuffling = TRUE;
        
        // Set bookmark time to 0 for new episodes
        track->bookmark_time = 0;
        
        log_message(LOG_DEBUG, "Set podcast-specific attributes for track: %s", track->title);
    } else {
        // Regular audio track
        track->flag4 = 0x00;
        track->mark_unplayed = 0x00;
        track->remember_playback_position = FALSE;
        track->skip_when_shuffling = FALSE;
    }
    
    log_message(LOG_DEBUG, "Created track: %s by %s", track->title, track->artist);
    return track;
}

gboolean add_file_to_ipod(RbIpodDb *db, const char *file_path) {
    if (!db || !file_path) return FALSE;
    
    log_message(LOG_INFO, "Adding file to iPod: %s", file_path);
    
    // Extract metadata from filename
    AudioMetadata *meta = extract_metadata_from_filename(file_path);
    if (!meta) {
        log_message(LOG_ERROR, "Failed to extract metadata from %s", file_path);
        return FALSE;
    }
    
    // Probe audio file for duration and bitrate
    if (!probe_audio_file(file_path, meta)) {
        log_message(LOG_WARNING, "Could not extract audio duration from %s, using defaults", file_path);
    }
    
    // Ensure iPod directory structure exists
    if (!ensure_ipod_directory_structure(db->mount_point)) {
        log_message(LOG_ERROR, "Failed to create iPod directory structure");
        free_metadata(meta);
        return FALSE;
    }
    
    // Generate iPod filename
    char *ipod_path = generate_ipod_filename(db->mount_point, file_path);
    if (!ipod_path) {
        log_message(LOG_ERROR, "Failed to generate iPod path for %s", file_path);
        free_metadata(meta);
        return FALSE;
    }
    
    // Copy file to iPod
    if (!copy_file_to_ipod(file_path, ipod_path)) {
        log_message(LOG_ERROR, "Failed to copy file to iPod");
        g_free(ipod_path);
        free_metadata(meta);
        return FALSE;
    }
    
    // Create track from metadata
    Itdb_Track *track = create_ipod_track_from_metadata(meta, ipod_path, strrchr(file_path, '.'));
    if (!track) {
        log_message(LOG_ERROR, "Failed to create track from metadata");
        g_free(ipod_path);
        free_metadata(meta);
        return FALSE;
    }
    
    // Add track to database
    itdb_track_add(db->itdb, track, -1);
    
    // Add track to appropriate special playlist based on media type
    switch (track->mediatype) {
        case ITDB_MEDIATYPE_PODCAST: {
            Itdb_Playlist *podcasts_pl = itdb_playlist_podcasts(db->itdb);
            if (!podcasts_pl) {
                // Create podcasts playlist if it doesn't exist
                podcasts_pl = itdb_playlist_new("Podcasts", FALSE);
                itdb_playlist_set_podcasts(podcasts_pl);
                itdb_playlist_add(db->itdb, podcasts_pl, -1);
                log_message(LOG_INFO, "Created Podcasts playlist");
            }
            itdb_playlist_add_track(podcasts_pl, track, -1);
            log_message(LOG_DEBUG, "Added podcast track to Podcasts playlist: %s", track->title);
            break;
        }
        case ITDB_MEDIATYPE_AUDIOBOOK: {
            // Look for existing audiobooks playlist or create one
            Itdb_Playlist *audiobooks_pl = NULL;
            for (GList *pl_item = db->itdb->playlists; pl_item; pl_item = pl_item->next) {
                Itdb_Playlist *pl = (Itdb_Playlist*)pl_item->data;
                if (pl->name && g_ascii_strcasecmp(pl->name, "Audiobooks") == 0) {
                    audiobooks_pl = pl;
                    break;
                }
            }
            if (!audiobooks_pl) {
                audiobooks_pl = itdb_playlist_new("Audiobooks", FALSE);
                itdb_playlist_add(db->itdb, audiobooks_pl, -1);
                log_message(LOG_INFO, "Created Audiobooks playlist");
            }
            itdb_playlist_add_track(audiobooks_pl, track, -1);
            log_message(LOG_DEBUG, "Added audiobook track to Audiobooks playlist: %s", track->title);
            break;
        }
        case ITDB_MEDIATYPE_MOVIE:
        case ITDB_MEDIATYPE_MUSICVIDEO:
        case ITDB_MEDIATYPE_TVSHOW: {
            // Look for existing videos playlist or create one
            Itdb_Playlist *videos_pl = NULL;
            for (GList *pl_item = db->itdb->playlists; pl_item; pl_item = pl_item->next) {
                Itdb_Playlist *pl = (Itdb_Playlist*)pl_item->data;
                if (pl->name && g_ascii_strcasecmp(pl->name, "Videos") == 0) {
                    videos_pl = pl;
                    break;
                }
            }
            if (!videos_pl) {
                videos_pl = itdb_playlist_new("Videos", FALSE);
                itdb_playlist_add(db->itdb, videos_pl, -1);
                log_message(LOG_INFO, "Created Videos playlist");
            }
            itdb_playlist_add_track(videos_pl, track, -1);
            log_message(LOG_DEBUG, "Added video track to Videos playlist: %s", track->title);
            break;
        }
        case ITDB_MEDIATYPE_AUDIO:
        default:
            // Regular music tracks are automatically available in the Music menu
            // No special playlist needed
            log_message(LOG_DEBUG, "Added music track to main library: %s", track->title);
            break;
    }
    
    // Update statistics
    g_sync_ctx.stats.files_added++;
    
    g_free(ipod_path);
    free_metadata(meta);
    
    log_message(LOG_DEBUG, "Successfully added file to iPod database");
    return TRUE;
}