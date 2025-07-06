#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <glib.h>
#include <gpod/itdb.h>
#include <tag_c.h>

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

gboolean extract_artwork_ffmpeg(const char *file_path, AudioMetadata *meta) {
    if (!file_path || !meta) return FALSE;
    
    // Create temporary file for artwork extraction
    char temp_artwork[256];
    snprintf(temp_artwork, sizeof(temp_artwork), "/tmp/artwork_%d", getpid());
    
    // Try multiple extraction methods to get artwork
    char command[2048];
    gboolean success = FALSE;
    
    // Method 1: Extract first video stream (cover art) to JPEG (better iPod compatibility)
    snprintf(command, sizeof(command), 
             "ffmpeg -i \"%s\" -map 0:v:0 -c:v mjpeg -q:v 2 \"%s.jpg\" -y 2>/dev/null", 
             file_path, temp_artwork);
    
    if (system(command) == 0) {
        char jpg_file[512];
        snprintf(jpg_file, sizeof(jpg_file), "%s.jpg", temp_artwork);
        
        FILE *artwork_file = fopen(jpg_file, "rb");
        if (artwork_file) {
            fseek(artwork_file, 0, SEEK_END);
            long size = ftell(artwork_file);
            fseek(artwork_file, 0, SEEK_SET);
            
            if (size > 100 && size < 5000000) { // Reasonable size check
                meta->artwork_data = g_malloc(size);
                meta->artwork_size = fread(meta->artwork_data, 1, size, artwork_file);
                meta->artwork_format = g_strdup("jpeg");
                success = TRUE;
                log_message(LOG_DEBUG, "Extracted artwork (JPEG): %zu bytes", meta->artwork_size);
            }
            fclose(artwork_file);
        }
        unlink(jpg_file);
    }
    
    // Method 2: If JPEG failed, try PNG
    if (!success) {
        snprintf(command, sizeof(command), 
                 "ffmpeg -i \"%s\" -map 0:v:0 -c:v png \"%s.png\" -y 2>/dev/null", 
                 file_path, temp_artwork);
        
        if (system(command) == 0) {
            char png_file[512];
            snprintf(png_file, sizeof(png_file), "%s.png", temp_artwork);
            
            FILE *artwork_file = fopen(png_file, "rb");
            if (artwork_file) {
                fseek(artwork_file, 0, SEEK_END);
                long size = ftell(artwork_file);
                fseek(artwork_file, 0, SEEK_SET);
                
                if (size > 100 && size < 5000000) { // Reasonable size check
                    meta->artwork_data = g_malloc(size);
                    meta->artwork_size = fread(meta->artwork_data, 1, size, artwork_file);
                    meta->artwork_format = g_strdup("png");
                    success = TRUE;
                    log_message(LOG_DEBUG, "Extracted artwork (PNG): %zu bytes", meta->artwork_size);
                }
                fclose(artwork_file);
            }
            unlink(png_file);
        }
    }
    
    // Method 3: Use ffprobe to check if artwork actually exists
    if (!success) {
        snprintf(command, sizeof(command), 
                 "ffprobe -v quiet -select_streams v:0 -show_entries stream=codec_name -of csv=p=0 \"%s\" 2>/dev/null", 
                 file_path);
        
        FILE *probe = popen(command, "r");
        if (probe) {
            char codec[64] = {0};
            if (fgets(codec, sizeof(codec), probe)) {
                // Remove newline
                char *newline = strchr(codec, '\n');
                if (newline) *newline = '\0';
                
                if (strlen(codec) > 0) {
                    log_message(LOG_DEBUG, "File has video stream (codec: %s) but extraction failed: %s", codec, file_path);
                } else {
                    log_message(LOG_DEBUG, "No artwork found in file: %s", file_path);
                }
            }
            pclose(probe);
        }
    }
    
    return success;
}

gboolean extract_podcast_specific_metadata(const char *file_path, AudioMetadata *meta) {
    if (!file_path || !meta) return FALSE;
    
    // Extract publication date from filename if it contains date pattern
    const char *filename = strrchr(file_path, '/');
    if (filename) filename++; else filename = file_path;
    
    // Look for date patterns like YYYY-MM-DD or DD.MM.YYYY
    int year = 0, month = 0, day = 0;
    char *date_start = strstr(filename, "2025");
    if (!date_start) date_start = strstr(filename, "2024");
    if (!date_start) date_start = strstr(filename, "2023");
    
    if (date_start) {
        // Try YYYY-MM-DD format
        if (sscanf(date_start, "%d-%d-%d", &year, &month, &day) == 3 ||
            sscanf(date_start, "%d.%d.%d", &day, &month, &year) == 3) {
            
            struct tm tm = {0};
            tm.tm_year = year - 1900;
            tm.tm_mon = month - 1;
            tm.tm_mday = day;
            meta->time_released = mktime(&tm);
            
            log_message(LOG_DEBUG, "Extracted podcast date: %04d-%02d-%02d", year, month, day);
        }
    }
    
    // Extract episode/season from filename patterns
    char *episode_pattern = strstr(filename, "Episode");
    if (!episode_pattern) episode_pattern = strstr(filename, "EP");
    if (!episode_pattern) episode_pattern = strstr(filename, "E");
    
    if (episode_pattern) {
        int episode_num = 0;
        if (sscanf(episode_pattern, "Episode %d", &episode_num) == 1 ||
            sscanf(episode_pattern, "EP%d", &episode_num) == 1 ||
            sscanf(episode_pattern, "E%d", &episode_num) == 1) {
            meta->track_number = episode_num;
            log_message(LOG_DEBUG, "Extracted episode number: %d", episode_num);
        }
    }
    
    // Set podcast-specific defaults
    meta->mark_unplayed = TRUE;  // New episodes should be marked as unplayed
    if (!meta->category) {
        meta->category = g_strdup("Podcast");
    }
    
    return TRUE;
}

gboolean extract_audio_metadata_taglib(const char *file_path, AudioMetadata *meta) {
    if (!file_path || !meta) return FALSE;
    
    // Open file with TagLib
    TagLib_File *file = taglib_file_new(file_path);
    if (!file || !taglib_file_is_valid(file)) {
        if (file) taglib_file_free(file);
        return FALSE;
    }
    
    // Get tag information
    TagLib_Tag *tag = taglib_file_tag(file);
    if (!tag) {
        taglib_file_free(file);
        return FALSE;
    }
    
    gboolean any_success = FALSE;
    
    // Extract metadata
    char *title = taglib_tag_title(tag);
    char *artist = taglib_tag_artist(tag);
    char *album = taglib_tag_album(tag);
    char *genre = taglib_tag_genre(tag);
    char *comment = taglib_tag_comment(tag);
    
    // Safely copy to metadata structure
    if (title && strlen(title) > 0) {
        g_free(meta->title);
        meta->title = g_strdup(title);
        any_success = TRUE;
    }
    if (artist && strlen(artist) > 0) {
        g_free(meta->artist);
        meta->artist = g_strdup(artist);
        any_success = TRUE;
    }
    if (album && strlen(album) > 0) {
        g_free(meta->album);
        meta->album = g_strdup(album);
        any_success = TRUE;
    }
    if (genre && strlen(genre) > 0) {
        g_free(meta->genre);
        meta->genre = g_strdup(genre);
        any_success = TRUE;
    }
    // Note: comment field not available in AudioMetadata structure
    // if (comment && strlen(comment) > 0) {
    //     meta->comment = g_strdup(comment);
    //     any_success = TRUE;
    // }
    
    // Extract numeric metadata
    unsigned int year = taglib_tag_year(tag);
    unsigned int track = taglib_tag_track(tag);
    
    if (year > 0) {
        meta->year = year;
        any_success = TRUE;
    }
    if (track > 0) {
        meta->track_number = track;
        any_success = TRUE;
    }
    
    // Get audio properties
    const TagLib_AudioProperties *props = taglib_file_audioproperties(file);
    if (props) {
        int duration = taglib_audioproperties_length(props);
        int bitrate = taglib_audioproperties_bitrate(props);
        
        if (duration > 0) {
            meta->duration = duration;
            any_success = TRUE;
        }
        if (bitrate > 0) {
            meta->bitrate = bitrate;
            any_success = TRUE;
        }
    }
    
    // Clean up
    taglib_file_free(file);
    
    // Extract artwork if TagLib extraction was successful
    if (any_success) {
        // Try TagLib native artwork extraction first
        if (!extract_artwork_taglib_native(file_path, meta)) {
            // Fallback to ffmpeg if TagLib fails
            extract_artwork_ffmpeg(file_path, meta);
        }
    }
    
    log_message(LOG_DEBUG, "TagLib extracted metadata: Title='%s', Artist='%s', Album='%s', Genre='%s', Year=%d, Track=%d, Duration=%d sec, Bitrate=%d kbps, Artwork=%zu bytes", 
               meta->title ? meta->title : "N/A", 
               meta->artist ? meta->artist : "N/A", 
               meta->album ? meta->album : "N/A", 
               meta->genre ? meta->genre : "N/A",
               meta->year, meta->track_number,
               meta->duration, meta->bitrate,
               meta->artwork_size);
    
    return any_success;
}

gboolean extract_audio_metadata_full(const char *file_path, AudioMetadata *meta) {
    if (!file_path || !meta) return FALSE;
    
    // Try TagLib first (most reliable)
    gboolean taglib_success = extract_audio_metadata_taglib(file_path, meta);
    if (taglib_success) {
        log_message(LOG_DEBUG, "TagLib extraction successful for: %s", file_path);
        return TRUE;
    }
    
    log_message(LOG_DEBUG, "TagLib extraction failed, trying fallback methods for: %s", file_path);
    
    gboolean any_success = FALSE;
    
    // Fallback: simple ffprobe for duration/bitrate only
    char command[2048];
    snprintf(command, sizeof(command), 
             "ffprobe -v quiet -show_format -of default=noprint_wrappers=1:nokey=1 -select_streams a:0 \"%s\" 2>/dev/null", 
             file_path);
    
    FILE *pipe = popen(command, "r");
    if (pipe) {
        char line[256];
        while (fgets(line, sizeof(line), pipe)) {
            // Simple parsing for basic info
            if (strstr(line, "duration=")) {
                float duration = 0;
                if (sscanf(line, "duration=%f", &duration) == 1) {
                    meta->duration = (int)duration;
                    any_success = TRUE;
                }
            } else if (strstr(line, "bit_rate=")) {
                int bitrate = 0;
                if (sscanf(line, "bit_rate=%d", &bitrate) == 1) {
                    meta->bitrate = bitrate / 1000; // Convert to kbps
                    any_success = TRUE;
                }
            }
        }
        pclose(pipe);
    }
    
    return any_success;
}


gboolean probe_audio_file(const char *file_path, AudioMetadata *meta) {
    if (!file_path || !meta) return FALSE;
    
    // First, try comprehensive metadata extraction from the audio file
    gboolean metadata_success = extract_audio_metadata_full(file_path, meta);
    
    if (!metadata_success) {
        // If ffprobe fails, extract metadata from filename as fallback
        log_message(LOG_DEBUG, "ffprobe failed, extracting from filename: %s", file_path);
        
        // Save current basic info
        guint32 current_mediatype = meta->mediatype;
        gsize current_file_size = meta->file_size;
        time_t current_time_added = meta->time_added;
        
        // Clear current metadata
        free_metadata(meta);
        
        // Re-extract from filename
        AudioMetadata *filename_meta = extract_metadata_from_filename(file_path);
        if (filename_meta) {
            *meta = *filename_meta;
            g_free(filename_meta); // Free the container, not the content
            
            // Restore important fields
            meta->mediatype = current_mediatype;
            meta->file_size = current_file_size;
            meta->time_added = current_time_added;
        }
    }
    
    // Always try to get duration/bitrate if not already set
    if (meta->duration == 0) {
        int duration = 0, bitrate = 0;
        if (extract_audio_duration(file_path, &duration, &bitrate)) {
            meta->duration = duration;
            meta->bitrate = bitrate;
            log_message(LOG_DEBUG, "Extracted duration/bitrate: %s -> %d sec, %d kbps", 
                       file_path, duration, bitrate);
        }
    }
    
    // If this is a podcast, extract podcast-specific metadata
    if (meta->mediatype == ITDB_MEDIATYPE_PODCAST) {
        extract_podcast_specific_metadata(file_path, meta);
        log_message(LOG_DEBUG, "Extracted podcast metadata for: %s (Episode: %d, Released: %ld)", 
                   file_path, meta->track_number, meta->time_released);
    }
    
    log_message(LOG_DEBUG, "Final metadata: Title='%s', Artist='%s', Album='%s'", 
               meta->title ? meta->title : "N/A",
               meta->artist ? meta->artist : "N/A", 
               meta->album ? meta->album : "N/A");
    
    return TRUE;
}

Itdb_Track* create_ipod_track_from_metadata(const AudioMetadata *meta, const char *ipod_path, const char *media_type) {
    if (!meta || !ipod_path) return NULL;
    
    Itdb_Track *track = itdb_track_new();
    if (!track) return NULL;
    
    // Set comprehensive metadata for better iPod display
    track->title = g_strdup(meta->title ? meta->title : "Unknown Title");
    track->artist = g_strdup(meta->artist ? meta->artist : "Unknown Artist");
    track->album = g_strdup(meta->album ? meta->album : "Unknown Album");
    track->genre = g_strdup(meta->genre ? meta->genre : "Unknown");
    
    // Optional metadata fields that enhance the iPod experience
    if (meta->composer && strlen(meta->composer) > 0) {
        track->composer = g_strdup(meta->composer);
    }
    if (meta->albumartist && strlen(meta->albumartist) > 0) {
        track->albumartist = g_strdup(meta->albumartist);
    }
    
    // Additional metadata for better organization
    track->year = meta->year > 0 ? meta->year : 2024;
    track->track_nr = meta->track_number > 0 ? meta->track_number : 1;
    track->cd_nr = meta->disc_number > 0 ? meta->disc_number : 1;
    
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
    
    // Add artwork using libgpod API if available
    if (meta->artwork_data && meta->artwork_size > 0) {
        // Create temporary file for libgpod artwork functions
        char temp_artwork[256];
        const char *ext = (meta->artwork_format && strcmp(meta->artwork_format, "png") == 0) ? "png" : "jpg";
        snprintf(temp_artwork, sizeof(temp_artwork), "/tmp/track_artwork_%d.%s", getpid(), ext);
        
        FILE *temp_file = fopen(temp_artwork, "wb");
        if (temp_file) {
            size_t written = fwrite(meta->artwork_data, 1, meta->artwork_size, temp_file);
            fclose(temp_file);
            
            if (written == meta->artwork_size) {
                // Use libgpod to set artwork (proper method that won't interfere with metadata)
                GError *error = NULL;
                gboolean artwork_added = itdb_track_set_thumbnails(track, temp_artwork);
                
                if (artwork_added) {
                    log_message(LOG_DEBUG, "Successfully added artwork to track: %s (%zu bytes, format: %s)", 
                               track->title, meta->artwork_size, ext);
                } else {
                    log_message(LOG_WARNING, "Failed to add artwork to track: %s (libgpod error)", track->title);
                    
                    // Try alternative: extract artwork again using ffmpeg directly
                    char ffmpeg_cmd[1024];
                    snprintf(ffmpeg_cmd, sizeof(ffmpeg_cmd), 
                             "ffmpeg -i \"%s\" -an -vcodec copy \"%s_direct.jpg\" -y 2>/dev/null", 
                             track->ipod_path, temp_artwork);
                    
                    if (system(ffmpeg_cmd) == 0) {
                        char direct_artwork[512];
                        snprintf(direct_artwork, sizeof(direct_artwork), "%s_direct.jpg", temp_artwork);
                        
                        if (itdb_track_set_thumbnails(track, direct_artwork)) {
                            log_message(LOG_DEBUG, "Successfully added artwork using direct extraction: %s", track->title);
                        } else {
                            log_message(LOG_WARNING, "Direct artwork extraction also failed for: %s", track->title);
                        }
                        //unlink(direct_artwork);
                    }
                }
            } else {
                log_message(LOG_WARNING, "Failed to write complete artwork file for track: %s (%zu/%zu bytes)", 
                           track->title, written, meta->artwork_size);
            }
            
            // Clean up temporary file
            //unlink(temp_artwork);
        } else {
            log_message(LOG_WARNING, "Failed to create temporary artwork file for track: %s", track->title);
        }
    }
    
    // Set media-type specific attributes
    if (track->mediatype == ITDB_MEDIATYPE_PODCAST) {
        track->flag4 = 0x01;  // Podcast flag
        track->mark_unplayed = meta->mark_unplayed ? 0x02 : 0x01;  // Mark as new if specified
        track->remember_playback_position = TRUE;
        track->skip_when_shuffling = TRUE;
        track->bookmark_time = 0;
        
        // Set podcast-specific metadata for better iPod display
        if (meta->description && strlen(meta->description) > 0) {
            track->description = g_strdup(meta->description);
        }
        if (meta->subtitle && strlen(meta->subtitle) > 0) {
            track->subtitle = g_strdup(meta->subtitle);
        }
        if (meta->category && strlen(meta->category) > 0) {
            track->category = g_strdup(meta->category);
        }
        if (meta->podcasturl && strlen(meta->podcasturl) > 0) {
            track->podcasturl = g_strdup(meta->podcasturl);
        }
        if (meta->podcastrss && strlen(meta->podcastrss) > 0) {
            track->podcastrss = g_strdup(meta->podcastrss);
        }
        
        // Set release date for proper podcast chronology
        if (meta->time_released > 0) {
            track->time_released = meta->time_released;
            track->time_modified = meta->time_released;  // Use release date as modification time
        }
        
        log_message(LOG_DEBUG, "Set podcast attributes for track: %s (Episode: %d, Season: %d, Released: %ld)", 
                   track->title, track->track_nr, track->cd_nr, track->time_released);
    } else if (track->mediatype == ITDB_MEDIATYPE_AUDIO) {
        // CRITICAL: Ensure music tracks have proper attributes for iPod menu access
        track->flag4 = 0x00;  // No special flags for music
        track->mark_unplayed = 0x00;  // Not applicable for music
        track->remember_playback_position = FALSE;
        track->skip_when_shuffling = FALSE;
        track->bookmark_time = 0;
        
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
    
    // Initialize metadata structure
    AudioMetadata *meta = g_malloc0(sizeof(AudioMetadata));
    if (!meta) {
        log_message(LOG_ERROR, "Failed to allocate metadata structure");
        return FALSE;
    }
    
    // Set media type if forced
    if (g_sync_ctx.use_force_mediatype) {
        meta->mediatype = g_sync_ctx.force_mediatype;
    } else {
        meta->mediatype = ITDB_MEDIATYPE_AUDIO; // Default
    }
    
    // Set basic file info
    struct stat file_stat;
    if (stat(file_path, &file_stat) == 0) {
        meta->file_size = file_stat.st_size;
    }
    meta->time_added = time(NULL);
    
    // Probe audio file for all metadata (will fallback to filename if needed)
    if (!probe_audio_file(file_path, meta)) {
        log_message(LOG_ERROR, "Failed to extract any metadata from %s", file_path);
        free_metadata(meta);
        return FALSE;
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