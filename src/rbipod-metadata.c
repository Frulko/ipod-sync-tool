#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <glib.h>
#include <gpod/itdb.h>

#include "../include/rbipod-metadata.h"
#include "../include/rbipod-logging.h"
#include "../include/rbipod-utils.h"

// =============================================================================
// METADATA EXTRACTION AND MANAGEMENT
// =============================================================================

guint32 parse_media_type_string(const char *media_type_str) {
    if (!media_type_str) return ITDB_MEDIATYPE_AUDIO;
    
    if (g_ascii_strcasecmp(media_type_str, "audio") == 0) {
        return ITDB_MEDIATYPE_AUDIO;
    } else if (g_ascii_strcasecmp(media_type_str, "movie") == 0 ||
               g_ascii_strcasecmp(media_type_str, "video") == 0) {
        return ITDB_MEDIATYPE_MOVIE;
    } else if (g_ascii_strcasecmp(media_type_str, "podcast") == 0) {
        return ITDB_MEDIATYPE_PODCAST;
    } else if (g_ascii_strcasecmp(media_type_str, "audiobook") == 0) {
        return ITDB_MEDIATYPE_AUDIOBOOK;
    } else if (g_ascii_strcasecmp(media_type_str, "musicvideo") == 0 ||
               g_ascii_strcasecmp(media_type_str, "music-video") == 0) {
        return ITDB_MEDIATYPE_MUSICVIDEO;
    } else if (g_ascii_strcasecmp(media_type_str, "tvshow") == 0 ||
               g_ascii_strcasecmp(media_type_str, "tv-show") == 0) {
        return ITDB_MEDIATYPE_TVSHOW;
    } else if (g_ascii_strcasecmp(media_type_str, "ringtone") == 0) {
        return ITDB_MEDIATYPE_RINGTONE;
    } else if (g_ascii_strcasecmp(media_type_str, "rental") == 0) {
        return ITDB_MEDIATYPE_RENTAL;
    } else if (g_ascii_strcasecmp(media_type_str, "itunes-extra") == 0 ||
               g_ascii_strcasecmp(media_type_str, "extra") == 0) {
        return ITDB_MEDIATYPE_ITUNES_EXTRA;
    } else if (g_ascii_strcasecmp(media_type_str, "memo") == 0) {
        return ITDB_MEDIATYPE_MEMO;
    } else if (g_ascii_strcasecmp(media_type_str, "itunes-u") == 0 ||
               g_ascii_strcasecmp(media_type_str, "itunesu") == 0) {
        return ITDB_MEDIATYPE_ITUNES_U;
    }
    
    return ITDB_MEDIATYPE_AUDIO; // Default fallback
}

const char* get_media_type_name(guint32 mediatype) {
    switch (mediatype) {
        case ITDB_MEDIATYPE_AUDIO: return "Audio";
        case ITDB_MEDIATYPE_MOVIE: return "Movie";
        case ITDB_MEDIATYPE_PODCAST: return "Podcast";
        case ITDB_MEDIATYPE_AUDIOBOOK: return "Audiobook";
        case ITDB_MEDIATYPE_MUSICVIDEO: return "Music Video";
        case ITDB_MEDIATYPE_TVSHOW: return "TV Show";
        case ITDB_MEDIATYPE_RINGTONE: return "Ringtone";
        case ITDB_MEDIATYPE_RENTAL: return "Rental";
        case ITDB_MEDIATYPE_ITUNES_EXTRA: return "iTunes Extra";
        case ITDB_MEDIATYPE_MEMO: return "Memo";
        case ITDB_MEDIATYPE_ITUNES_U: return "iTunes U";
        default: return "Unknown";
    }
}

void parse_mediatype_arg(int argc, char *argv[], int start_index, char **mediatype_str) {
    *mediatype_str = NULL;
    for (int i = start_index; i < argc - 1; i++) {
        if (strcmp(argv[i], "--mediatype") == 0) {
            *mediatype_str = argv[i + 1];
            break;
        }
    }
}

AudioMetadata* extract_metadata_from_filename(const char *filename) {
    if (!filename) return NULL;
    
    AudioMetadata *meta = g_malloc0(sizeof(AudioMetadata));
    if (!meta) return NULL;
    
    char *basename = g_path_get_basename(filename);
    char *name_without_ext = basename;
    
    // Remove file extension
    char *dot = strrchr(name_without_ext, '.');
    if (dot) *dot = '\0';
    
    // Try different parsing patterns
    char **parts = g_strsplit(name_without_ext, " - ", -1);
    int num_parts = g_strv_length(parts);
    
    if (num_parts >= 4) {
        // Format: Artist - Album - Track - Title
        meta->artist = g_strdup(g_strstrip(parts[0]));
        meta->album = g_strdup(g_strstrip(parts[1]));
        meta->track_number = atoi(g_strstrip(parts[2]));
        meta->title = g_strdup(g_strstrip(parts[3]));
    } else if (num_parts == 3) {
        // Check if first part is a number (Track - Artist - Title)
        char *trimmed_first = g_strstrip(parts[0]);
        if (g_ascii_isdigit(trimmed_first[0])) {
            meta->track_number = atoi(trimmed_first);
            meta->artist = g_strdup(g_strstrip(parts[1]));
            meta->title = g_strdup(g_strstrip(parts[2]));
        } else {
            // Format: Artist - Album - Title
            meta->artist = g_strdup(trimmed_first);
            meta->album = g_strdup(g_strstrip(parts[1]));
            meta->title = g_strdup(g_strstrip(parts[2]));
        }
    } else if (num_parts == 2) {
        // Format: Artist - Title
        meta->artist = g_strdup(g_strstrip(parts[0]));
        meta->title = g_strdup(g_strstrip(parts[1]));
    } else {
        // Single part, use as title
        meta->title = g_strdup(g_strstrip(name_without_ext));
    }
    
    g_strfreev(parts);
    
    // Set defaults for missing fields
    if (!meta->title) meta->title = g_strdup("Unknown Title");
    if (!meta->artist) meta->artist = g_strdup("Unknown Artist");
    if (!meta->album) meta->album = g_strdup("Unknown Album");
    if (!meta->genre) meta->genre = g_strdup("Unknown");
    
    // Get file size
    struct stat file_stat;
    if (stat(filename, &file_stat) == 0) {
        meta->file_size = file_stat.st_size;
    }
    
    // Set timestamps
    meta->time_added = time(NULL);
    
    // Set media type - use force type if specified, otherwise auto-detect
    if (g_sync_ctx.use_force_mediatype) {
        meta->mediatype = g_sync_ctx.force_mediatype;
    } else {
        meta->mediatype = ITDB_MEDIATYPE_AUDIO; // Default for now
    }
    
    meta->bookmark_time = 0;
    
    // Set playback options for non-audio media
    meta->remember_playback_position = (meta->mediatype != ITDB_MEDIATYPE_AUDIO);
    meta->skip_when_shuffling = (meta->mediatype != ITDB_MEDIATYPE_AUDIO);
    
    // Set podcast-specific metadata if this is a podcast
    if (meta->mediatype == ITDB_MEDIATYPE_PODCAST) {
        // Generate synthetic podcast URL based on filename
        meta->podcasturl = g_strdup_printf("file://%s", filename);
        meta->podcastrss = g_strdup("http://localhost/podcast.rss");
        
        // Set podcast category and description
        meta->category = g_strdup("Podcasts");
        meta->description = g_strdup_printf("Podcast episode: %s", meta->title);
        meta->subtitle = g_strdup(meta->title);
        
        // Set release time to current time if not specified
        meta->time_released = time(NULL);
        
        // Mark as unplayed (new episode)
        meta->mark_unplayed = TRUE;
        
        // Ensure proper podcast behavior
        meta->remember_playback_position = TRUE;
        meta->skip_when_shuffling = TRUE;
        
        log_message(LOG_DEBUG, "Set podcast-specific metadata for: %s", meta->title);
    } else {
        // Initialize podcast fields to NULL for non-podcasts
        meta->podcasturl = NULL;
        meta->podcastrss = NULL;
        meta->description = NULL;
        meta->subtitle = NULL;
        meta->category = NULL;
        meta->time_released = 0;
        meta->mark_unplayed = FALSE;
    }
    
    g_free(basename);
    return meta;
}

void free_metadata(AudioMetadata *meta) {
    if (!meta) return;
    
    g_free(meta->title);
    g_free(meta->artist);
    g_free(meta->album);
    g_free(meta->genre);
    g_free(meta->composer);
    g_free(meta->albumartist);
    g_free(meta->sort_artist);
    g_free(meta->sort_album);
    g_free(meta->sort_albumartist);
    // Free podcast-specific fields
    g_free(meta->podcasturl);
    g_free(meta->podcastrss);
    g_free(meta->description);
    g_free(meta->subtitle);
    g_free(meta->category);
    g_free(meta);
}