#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <glib.h>

#include "../include/rbipod-utils.h"
#include "../include/rbipod-logging.h"
#include "../include/rbipod-config.h"

// =============================================================================
// GLOBAL VARIABLES
// =============================================================================

SyncContext g_sync_ctx = {0};
static gboolean g_shutdown_requested = FALSE;

// =============================================================================
// APPLICATION INITIALIZATION AND CLEANUP
// =============================================================================

gboolean init_application(const char *mount_point) {
    memset(&g_sync_ctx, 0, sizeof(g_sync_ctx));
    
    // Initialize mutex
    pthread_mutex_init(&g_sync_ctx.log_mutex, NULL);
    
    // Initialize logging
    if (!init_logging(LOG_FILE)) {
        fprintf(stderr, "Warning: Could not initialize logging\n");
        // Continue without logging
    }
    
    log_message(LOG_INFO, "=== %s v%s started ===", PROGRAM_NAME, PROGRAM_VERSION);
    log_message(LOG_INFO, "Mount point: %s", mount_point ? mount_point : "auto-detect");
    log_message(LOG_INFO, "PID: %d", getpid());
    
    return TRUE;
}

void cleanup_application(void) {
    log_message(LOG_INFO, "=== Application cleanup ===");
    
    // Clean up iPod database if it exists
    if (g_sync_ctx.ipod_db) {
        // Note: rb_ipod_db_free will be implemented in rbipod-database.c
        // rb_ipod_db_free(g_sync_ctx.ipod_db);
        g_sync_ctx.ipod_db = NULL;
    }
    
    // Cleanup logging
    cleanup_logging();
    
    // Destroy mutex
    pthread_mutex_destroy(&g_sync_ctx.log_mutex);
}

void print_version(void) {
    printf("%s version %s\n", PROGRAM_NAME, PROGRAM_VERSION);
    printf("Built with libgpod and GLib support\n");
    printf("Based on Rhythmbox iPod management architecture\n");
}

void print_usage(const char *program_name) {
    printf("Usage: %s <command> [options]\n\n", program_name);
    
    printf("MOUNT/UNMOUNT COMMANDS:\n");
    printf("  mount [device] <mount_point>   Mount iPod device with proper permissions\n");
    printf("  unmount <mount_point>          Safely unmount iPod device\n");
    printf("  auto-mount                     Auto-detect and mount iPod device\n\n");
    
    printf("SYNC COMMANDS:\n");
    printf("  sync <mount_point> <directory>             Synchronize directory with iPod\n");
    printf("  sync-file <mount_point> <file> [--mediatype type]   Synchronize single file with iPod\n");
    printf("  sync-folder-filtered <mount_point> <folder> <mediatype>  Synchronize folder with specific media type\n");
    printf("  list <mount_point>                         List all tracks on iPod\n");
    printf("  info <mount_point>                         Show detailed iPod information\n");
    printf("  reset <mount_point> <mediatype>           Remove all tracks of specified media type\n");
    printf("  reset <mount_point> all                   Remove ALL tracks and clean iPod completely\n\n");
    
    printf("OTHER COMMANDS:\n");
    printf("  version                        Show version information\n");
    printf("  help                          Show this help message\n\n");
    
    printf("MOUNT EXAMPLES:\n");
    printf("  %s auto-mount                              # Auto-detect and mount\n", program_name);
    printf("  %s mount /media/ipod                       # Auto-detect device, mount at /media/ipod\n", program_name);
    printf("  %s mount /dev/sdb1 /media/ipod            # Mount specific device\n", program_name);
    printf("  %s unmount /media/ipod                     # Safely unmount\n\n", program_name);
    
    printf("SYNC EXAMPLES:\n");
    printf("  %s sync /media/ipod /home/user/Music                     # Sync music directory\n", program_name);
    printf("  %s sync-file /media/ipod /home/user/podcast.mp3 --mediatype podcast  # Sync single file as podcast\n", program_name);
    printf("  %s sync-folder-filtered /media/ipod /home/user/Podcasts podcast      # Sync folder as podcasts\n", program_name);
    printf("  %s sync-folder-filtered /media/ipod /home/user/Audiobooks audiobook  # Sync folder as audiobooks\n", program_name);
    printf("  %s list /media/ipod                                      # List tracks\n", program_name);
    printf("  %s info /media/ipod                                      # Show device info\n", program_name);
    printf("  %s reset /media/ipod podcast                             # Remove all podcasts\n", program_name);
    printf("  %s reset /media/ipod all                                 # Clean iPod completely\n\n", program_name);
    
    printf("FILESYSTEM SUPPORT:\n");
    printf("  • FAT32 (most common iPod format)\n");
    printf("  • HFS+ (Mac-formatted iPods)\n");
    printf("  • exFAT (newer large capacity iPods)\n");
    printf("  • Automatic detection and appropriate mount options\n\n");
    
    printf("FEATURES:\n");
    printf("  • Automatic filesystem detection and optimal mount options\n");
    printf("  • Proper user permissions (no root required for operation)\n");
    printf("  • Write permission verification\n");
    printf("  • Rhythmbox-inspired robust error handling and recovery\n");
    printf("  • Asynchronous database operations with delayed actions\n");
    printf("  • Automatic database backup and validation\n");
    printf("  • Thread-safe operations with proper locking\n");
    printf("  • Progress tracking and detailed logging\n");
    printf("  • Duplicate detection and skipping\n");
    printf("  • Graceful shutdown on interruption\n");
    printf("  • Smart iPod filename generation\n\n");
    
    printf("FILES CREATED:\n");
    printf("  ipod_sync.log           Detailed operation log\n");
    printf("  iTunesDB.rbbackup.*     Automatic database backups\n");
    printf("  iTunesDB.rbwork.*       Working database copies\n\n");
    
    printf("SUPPORTED MEDIA TYPES:\n");
    printf("  audio, movie, podcast, audiobook, musicvideo, tvshow, ringtone,\n");
    printf("  rental, itunes-extra, memo, itunes-u\n\n");
    
    printf("SAFETY FEATURES:\n");
    printf("  • Database is backed up before any modifications\n");
    printf("  • Operations are queued during database saves\n");
    printf("  • Full validation before and after operations\n");
    printf("  • Automatic recovery from interrupted operations\n");
    printf("  • Safe mount/unmount with filesystem sync\n");
}