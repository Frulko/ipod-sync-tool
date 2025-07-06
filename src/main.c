/*
 * iPod Sync Tool - Based on Rhythmbox's Robust iPod Management
 * ============================================================
 * 
 * This tool implements Rhythmbox's proven strategies for reliable iPod synchronization:
 * - Asynchronous database operations with proper queuing
 * - Thread-safe read-only/read-write state management
 * - Comprehensive backup and recovery mechanisms
 * - Proper libgpod integration patterns
 * - Robust error handling and validation
 * 
 * Based on Rhythmbox's rb-ipod-source.c and rb-ipod-db.c architecture
 * 
 * To compile this program, you need the following dependencies:
 * 
 * Ubuntu/Debian:
 *   sudo apt-get install libgpod-dev libglib2.0-dev udisks2 util-linux
 * 
 * CentOS/RHEL:
 *   sudo yum install libgpod-devel glib2-devel udisks2 util-linux
 * 
 * Usage Examples:
 * 
 * 1. Auto-detect and mount iPod:
 *    ./rhythmbox-ipod-sync auto-mount
 * 
 * 2. Manual mount and sync:
 *    ./rhythmbox-ipod-sync mount /dev/sdb1 /media/ipod
 *    ./rhythmbox-ipod-sync sync /media/ipod ~/Music
 *    ./rhythmbox-ipod-sync unmount /media/ipod
 * 
 * 3. Sync single file with media type:
 *    ./rhythmbox-ipod-sync sync-file /media/ipod ~/podcast.mp3 --mediatype podcast
 * 
 * 4. Sync folder with specific media type:
 *    ./rhythmbox-ipod-sync sync-folder-filtered /media/ipod ~/Audiobooks audiobook
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "../include/rbipod-types.h"
#include "../include/rbipod-utils.h"
#include "../include/rbipod-commands.h"
#include "../include/rbipod-metadata.h"
#include "../include/rbipod-logging.h"

// =============================================================================
// MAIN FUNCTION
// =============================================================================

int main(int argc, char *argv[]) {
    // Check minimum arguments
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    const char *command = argv[1];
    char *mediatype_str = NULL;
    
    // Handle special commands that don't need mount points
    if (strcmp(command, "version") == 0) {
        print_version();
        return 0;
    }
    
    if (strcmp(command, "help") == 0) {
        print_usage(argv[0]);
        return 0;
    }
    
    if (strcmp(command, "auto-mount") == 0) {
        return command_auto_mount();
    }
    
    // Handle mount command (special case with variable arguments)
    if (strcmp(command, "mount") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: mount command requires a mount point\n");
            fprintf(stderr, "Usage: %s mount [device] <mount_point>\n", argv[0]);
            return 1;
        }
        
        if (argc >= 4) {
            // Both device and mount point specified
            return command_mount_ipod(argv[2], argv[3]);
        } else {
            // Only mount point specified, auto-detect device
            return command_mount_ipod(NULL, argv[2]);
        }
    }
    
    // Handle unmount command
    if (strcmp(command, "unmount") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: unmount command requires mount point\n");
            fprintf(stderr, "Usage: %s unmount <mount_point>\n", argv[0]);
            return 1;
        }
        return command_unmount_ipod(argv[2]);
    }
    
    // All other commands require a mount point
    if (argc < 3) {
        fprintf(stderr, "Error: %s command requires mount point\n", command);
        print_usage(argv[0]);
        return 1;
    }
    
    const char *mount_point = argv[2];
    
    // Validate mount point
    struct stat mount_stat;
    if (stat(mount_point, &mount_stat) != 0) {
        fprintf(stderr, "Error: Mount point does not exist: %s\n", mount_point);
        return 1;
    }
    
    if (!S_ISDIR(mount_stat.st_mode)) {
        fprintf(stderr, "Error: Mount point is not a directory: %s\n", mount_point);
        return 1;
    }
    
    // Initialize application
    if (!init_application(mount_point)) {
        fprintf(stderr, "Error: Failed to initialize application\n");
        return 1;
    }
    
    // Parse mediatype option for sync/playlist/sync-file commands
    if (strcmp(command, "sync") == 0 || strcmp(command, "playlist") == 0 || strcmp(command, "sync-file") == 0) {
        parse_mediatype_arg(argc, argv, 4, &mediatype_str);
        
        if (mediatype_str) {
            g_sync_ctx.force_mediatype = parse_media_type_string(mediatype_str);
            if (g_sync_ctx.force_mediatype == ITDB_MEDIATYPE_AUDIO && 
                strcmp(mediatype_str, "audio") != 0) {
                fprintf(stderr, "Error: Invalid media type '%s'\n", mediatype_str);
                fprintf(stderr, "Valid types: audio, movie, podcast, audiobook, musicvideo, tvshow, ringtone, rental, itunes-extra, memo, itunes-u\n");
                cleanup_application();
                return 1;
            }
            g_sync_ctx.use_force_mediatype = TRUE;
            printf("Media type override: %s\n", get_media_type_name(g_sync_ctx.force_mediatype));
        }
    }
    
    int result = 1;
    
    // Dispatch commands
    if (strcmp(command, "sync") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Error: sync command requires source directory\n");
            fprintf(stderr, "Usage: %s sync <mount_point> <source_directory>\n", argv[0]);
            result = 1;
        } else {
            result = command_sync_directory(mount_point, argv[3]);
        }
    } else if (strcmp(command, "list") == 0) {
        result = command_list_tracks(mount_point);
    } else if (strcmp(command, "info") == 0) {
        result = command_show_info(mount_point);
    } else if (strcmp(command, "sync-file") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Error: sync-file command requires file path\n");
            fprintf(stderr, "Usage: %s sync-file <mount_point> <file_path> [--mediatype type]\n", argv[0]);
            result = 1;
        } else {
            result = command_sync_file(mount_point, argv[3]);
        }
    } else if (strcmp(command, "sync-folder-filtered") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Error: sync-folder-filtered command requires folder path and media type\n");
            fprintf(stderr, "Usage: %s sync-folder-filtered <mount_point> <folder_path> <mediatype>\n", argv[0]);
            result = 1;
        } else if (argc < 5) {
            fprintf(stderr, "Error: sync-folder-filtered command requires media type\n");
            fprintf(stderr, "Usage: %s sync-folder-filtered <mount_point> <folder_path> <mediatype>\n", argv[0]);
            result = 1;
        } else {
            result = command_sync_folder_filtered(mount_point, argv[3], argv[4]);
        }
    } else {
        fprintf(stderr, "Error: Unknown command '%s'\n\n", command);
        print_usage(argv[0]);
        result = 1;
    }
    
    // Cleanup and exit
    cleanup_application();
    return result;
}