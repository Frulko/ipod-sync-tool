#ifndef RBIPOD_CONFIG_H
#define RBIPOD_CONFIG_H

// =============================================================================
// PROGRAM CONFIGURATION
// =============================================================================

#define PROGRAM_NAME "rhythmbox-ipod-sync"
#define PROGRAM_VERSION "1.0.0"
#define MAX_PATH_LEN 4096
#define MAX_RETRY_ATTEMPTS 5
#define SYNC_DELAY_SECONDS 2
#define BACKUP_EXTENSION ".rbbackup"
#define WORKING_EXTENSION ".rbwork"
#define LOG_FILE "ipod_sync.log"

// iPod filesystem limits (based on Rhythmbox's constants)
#define IPOD_MAX_PATH_LEN 56
#define MAX_TRIES 5

#endif // RBIPOD_CONFIG_H