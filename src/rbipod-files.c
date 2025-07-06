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

// Global counter for iPod filename generation
static guint32 g_ipod_file_counter = 0;

// Initialize the iPod file counter based on existing files
void initialize_ipod_file_counter(const char *mount_point) {
    if (!mount_point) return;
    
    guint32 max_counter = 0;
    
    // Scan all F00-F49 directories for existing files
    for (int dir_num = 0; dir_num < 50; dir_num++) {
        char f_dir[1024];
        snprintf(f_dir, sizeof(f_dir), "%s/iPod_Control/Music/F%02d", mount_point, dir_num);
        
        GDir *dir = g_dir_open(f_dir, 0, NULL);
        if (!dir) continue;
        
        const char *filename;
        while ((filename = g_dir_read_name(dir)) != NULL) {
            // Check if filename matches iPod pattern (4 uppercase letters + extension)
            if (strlen(filename) >= 5) {
                char name_part[5];
                strncpy(name_part, filename, 4);
                name_part[4] = '\0';
                
                // Convert 4-letter name back to counter value
                if (name_part[0] >= 'A' && name_part[0] <= 'Z' &&
                    name_part[1] >= 'A' && name_part[1] <= 'Z' &&
                    name_part[2] >= 'A' && name_part[2] <= 'Z' &&
                    name_part[3] >= 'A' && name_part[3] <= 'Z') {
                    
                    guint32 name_value = 
                        (name_part[0] - 'A') * (26 * 26 * 26) +
                        (name_part[1] - 'A') * (26 * 26) +
                        (name_part[2] - 'A') * 26 +
                        (name_part[3] - 'A');
                    
                    guint32 estimated_counter = dir_num * 100 + name_value;
                    if (estimated_counter > max_counter) {
                        max_counter = estimated_counter;
                    }
                }
            }
        }
        g_dir_close(dir);
    }
    
    // Start counter after the highest existing file
    g_ipod_file_counter = max_counter + 1;
    
    log_message(LOG_DEBUG, "Initialized iPod file counter to %u", g_ipod_file_counter);
}

char* generate_ipod_filename(const char *mount_point, const char *original_filename) {
    if (!mount_point || !original_filename) return NULL;
    
    // Get file extension from original filename
    const char *ext = strrchr(original_filename, '.');
    if (!ext) ext = ".mp3"; // Default extension
    
    // Calculate which F directory to use (0-49, distributed evenly)
    int dir_num = (g_ipod_file_counter / 100) % 50;
    
    // Generate 4-character filename in the style of Apple/gtkpod
    // Format: [A-Z][A-Z][A-Z][A-Z] (uppercase letters)
    char ipod_name[5];
    guint32 name_id = g_ipod_file_counter % (26 * 26 * 26 * 26); // 456976 combinations
    
    ipod_name[0] = 'A' + (name_id / (26 * 26 * 26)) % 26;
    ipod_name[1] = 'A' + (name_id / (26 * 26)) % 26;
    ipod_name[2] = 'A' + (name_id / 26) % 26;
    ipod_name[3] = 'A' + name_id % 26;
    ipod_name[4] = '\0';
    
    // Increment counter for next file
    g_ipod_file_counter++;
    
    char *result = g_strdup_printf("%s/iPod_Control/Music/F%02d/%s%s", 
                                   mount_point, dir_num, ipod_name, ext);
    
    log_message(LOG_DEBUG, "Generated iPod filename: %s (counter: %u)", result, g_ipod_file_counter - 1);
    return result;
}

gboolean ensure_ipod_directory_structure(const char *mount_point) {
    if (!mount_point) return FALSE;
    
    char ipod_control[1024];
    char music_dir[1024];
    
    snprintf(ipod_control, sizeof(ipod_control), "%s/iPod_Control", mount_point);
    snprintf(music_dir, sizeof(music_dir), "%s/iPod_Control/Music", mount_point);
    
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
    
    // Create all F00-F49 subdirectories (standard iPod structure)
    for (int i = 0; i < 50; i++) {
        char f_dir[1024];
        snprintf(f_dir, sizeof(f_dir), "%s/iPod_Control/Music/F%02d", mount_point, i);
        
        if (mkdir(f_dir, 0755) != 0 && errno != EEXIST) {
            log_message(LOG_WARNING, "Failed to create directory F%02d: %s", i, strerror(errno));
            // Continue with other directories even if one fails
        }
    }
    
    log_message(LOG_DEBUG, "iPod directory structure ensured (F00-F49)");
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
    
    // Set only essential metadata to avoid database corruption
    track->title = g_strdup(meta->title ? meta->title : "Unknown Title");
    track->artist = g_strdup(meta->artist ? meta->artist : "Unknown Artist");
    track->album = g_strdup(meta->album ? meta->album : "Unknown Album");
    
    // Essential timing and quality information
    track->tracklen = meta->duration * 1000; // Convert to milliseconds (CRITICAL for playback)
    track->bitrate = meta->bitrate > 0 ? meta->bitrate : 128; // Default bitrate if not available
    
    // Get file size from the actual file
    struct stat file_stat;
    if (stat(ipod_path, &file_stat) == 0) {
        track->size = file_stat.st_size;
    }
    
    // Set iPod path - must be relative to mount point
    const char *relative_path = ipod_path;
    // Find the mount point dynamically by looking for /iPod_Control/
    char *ipod_control_pos = strstr(ipod_path, "/iPod_Control/");
    if (ipod_control_pos) {
        relative_path = ipod_control_pos;
    }
    track->ipod_path = g_strdup(relative_path);
    
    // Set media type
    track->mediatype = meta->mediatype;
    
    // Set file type based on actual file extension
    const char *file_ext = strrchr(ipod_path, '.');
    if (file_ext) {
        file_ext++; // Skip the dot
        if (g_ascii_strcasecmp(file_ext, "mp3") == 0) {
            track->filetype = g_strdup("mp3");
        } else if (g_ascii_strcasecmp(file_ext, "m4a") == 0 || 
                   g_ascii_strcasecmp(file_ext, "aac") == 0) {
            track->filetype = g_strdup("m4a");
        } else {
            track->filetype = g_strdup("mp3"); // Safe default
        }
    } else {
        track->filetype = g_strdup("mp3"); // Safe default
    }
    
    // Set current time for addition
    track->time_added = time(NULL);
    track->time_modified = track->time_added;
    
    // Set media-type specific attributes
    if (track->mediatype == ITDB_MEDIATYPE_PODCAST) {
        track->flag4 = 0x01;  // Podcast flag
        track->mark_unplayed = 0x02;  // Mark as new
        track->remember_playback_position = TRUE;
        track->skip_when_shuffling = TRUE;
        track->bookmark_time = 0;
        
        log_message(LOG_DEBUG, "Set podcast attributes for track: %s", track->title);
    } else if (track->mediatype == ITDB_MEDIATYPE_AUDIO) {
        // CRITICAL: Ensure music tracks have proper attributes for iPod menu access
        track->flag4 = 0x00;  // No special flags for music
        track->mark_unplayed = 0x00;  // Not applicable for music
        track->remember_playback_position = FALSE;
        track->skip_when_shuffling = FALSE;
        track->bookmark_time = 0;
        
        // Essential for music tracks to appear in iPod Music menu
        track->year = meta->year > 0 ? meta->year : 2024;  // Set a valid year
        track->track_nr = meta->track_number > 0 ? meta->track_number : 1;  // Track number
        track->cd_nr = meta->disc_number > 0 ? meta->disc_number : 1;  // Disc number
        
        // Genre is important for music categorization
        if (meta->genre && strlen(meta->genre) > 0) {
            track->genre = g_strdup(meta->genre);
        } else {
            track->genre = g_strdup("Unknown");  // Default genre
        }
        
        log_message(LOG_DEBUG, "Set music attributes for track: %s (year: %d, track: %d)", 
                   track->title, track->year, track->track_nr);
    }
    
    log_message(LOG_DEBUG, "Created simplified track: %s by %s (duration: %d ms, size: %d bytes)", 
               track->title, track->artist, track->tracklen, track->size);
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
    
    // Initialize the iPod file counter based on existing files
    initialize_ipod_file_counter(db->mount_point);
    
    // Generate iPod filename using Apple/gtkpod naming convention
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
    
    // CRITICAL: Add ALL tracks to Master Playlist for iPod menu visibility
    Itdb_Playlist *master_pl = itdb_playlist_mpl(db->itdb);
    if (master_pl) {
        itdb_playlist_add_track(master_pl, track, -1);
        log_message(LOG_DEBUG, "Added track to Master Playlist: %s", track->title);
    } else {
        log_message(LOG_ERROR, "Master Playlist not found - track may not be visible in iPod menu!");
    }
    
    // Add to media-type specific playlists 
    if (track->mediatype == ITDB_MEDIATYPE_PODCAST) {
        // Podcast-specific playlist
        Itdb_Playlist *podcasts_pl = itdb_playlist_podcasts(db->itdb);
        if (!podcasts_pl) {
            podcasts_pl = itdb_playlist_new("Podcasts", FALSE);
            itdb_playlist_set_podcasts(podcasts_pl);
            itdb_playlist_add(db->itdb, podcasts_pl, -1);
            log_message(LOG_INFO, "Created essential Podcasts playlist");
        }
        itdb_playlist_add_track(podcasts_pl, track, -1);
        log_message(LOG_DEBUG, "Added podcast track to Podcasts playlist: %s", track->title);
    } else if (track->mediatype == ITDB_MEDIATYPE_AUDIO) {
        // Music tracks are accessible through Master Playlist - no special playlist needed
        log_message(LOG_DEBUG, "Music track added to Master Playlist: %s", track->title);
    } else {
        log_message(LOG_DEBUG, "Added %s track to Master Playlist: %s", 
                   get_media_type_name(track->mediatype), track->title);
    }
    
    // Update statistics
    g_sync_ctx.stats.files_added++;
    
    g_free(ipod_path);
    free_metadata(meta);
    
    log_message(LOG_DEBUG, "Successfully added file to iPod database");
    return TRUE;
}