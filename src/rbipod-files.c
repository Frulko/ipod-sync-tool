#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
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
    
    // TODO: Implement proper iPod filename generation
    // For now, generate a simple path
    char *dir = build_ipod_dir_name(mount_point);
    char *basename = g_path_get_basename(original_filename);
    char *result = g_strdup_printf("%s/%s", dir, basename);
    
    g_free(dir);
    g_free(basename);
    return result;
}

gboolean copy_file_to_ipod(const char *source_path, const char *dest_path) {
    if (!source_path || !dest_path) return FALSE;
    
    log_message(LOG_INFO, "Copying file from %s to %s", source_path, dest_path);
    
    // TODO: Implement proper file copying with error handling
    // For now, just check if source exists
    struct stat source_stat;
    if (stat(source_path, &source_stat) != 0) {
        log_message(LOG_ERROR, "Source file does not exist: %s", source_path);
        return FALSE;
    }
    
    return TRUE;
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
    
    // File information
    track->ipod_path = g_strdup(ipod_path);
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