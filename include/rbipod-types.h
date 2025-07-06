#ifndef RBIPOD_TYPES_H
#define RBIPOD_TYPES_H

#include <glib.h>
#include <gpod/itdb.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>

#include "rbipod-config.h"

// =============================================================================
// TYPE DEFINITIONS
// =============================================================================

typedef enum {
    RB_IPOD_ACTION_SET_NAME,
    RB_IPOD_ACTION_ADD_TRACK,
    RB_IPOD_ACTION_REMOVE_TRACK
} RbIpodDelayedActionType;

typedef struct {
    RbIpodDelayedActionType type;
    union {
        gchar *name;
        Itdb_Track *track;
    };
} RbIpodDelayedAction;

typedef struct {
    Itdb_iTunesDB *itdb;
    gchar *mount_point;
    GMutex *mutex;
    gboolean is_read_only;
    gboolean is_saving;
    GQueue *delayed_actions;
    gboolean has_delayed_actions;
    gboolean shutdown_requested;
    
    // Database backup fields
    char backup_path[MAX_PATH_LEN];
    char working_path[MAX_PATH_LEN];
    gboolean backup_created;
} RbIpodDb;

typedef struct {
    char *title;
    char *artist;
    char *album;
    char *genre;
    char *composer;
    char *albumartist;
    char *sort_artist;
    char *sort_album;
    char *sort_albumartist;
    int year;
    int track_number;
    int disc_number;
    int duration;
    int bitrate;
    gint64 file_size;
    double rating;
    guint play_count;
    time_t time_added;
    time_t time_played;
    guint32 mediatype;
    gboolean remember_playback_position;
    gboolean skip_when_shuffling;
    guint32 bookmark_time;
    // Podcast-specific fields
    char *podcasturl;
    char *podcastrss;
    char *description;
    char *subtitle;
    char *category;
    time_t time_released;
    gboolean mark_unplayed;
    
    // Podcast episode metadata
    int season_number;     // Season number (CD number for libgpod)
    int episode_number;    // Episode number (track number for libgpod)
    char *episode_id;      // Unique episode identifier
    char *podcast_name;    // Podcast show name (for display)
    char *episode_summary; // Episode summary/description
    
    // Artwork/Cover fields
    guchar *artwork_data;
    gsize artwork_size;
    char *artwork_format;  // "jpeg", "png", etc.
} AudioMetadata;

typedef struct {
    int files_added;
    int files_skipped;
    int files_failed;
    int files_updated;
    int total_tracks_before;
    int total_tracks_after;
    gint64 bytes_transferred;
    time_t operation_start;
    time_t operation_end;
    double average_speed;
} OperationStats;

typedef struct {
    RbIpodDb *ipod_db;
    OperationStats stats;
    gboolean cancellation_requested;
    FILE *log_file;
    pthread_mutex_t log_mutex;
    guint32 force_mediatype;
    gboolean use_force_mediatype;
} SyncContext;

typedef enum {
    FILESYSTEM_FAT32,
    FILESYSTEM_HFS_PLUS,
    FILESYSTEM_EXFAT,
    FILESYSTEM_UNKNOWN
} FilesystemType;

typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_CRITICAL
} LogLevel;

#endif // RBIPOD_TYPES_H