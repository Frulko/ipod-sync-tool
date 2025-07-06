#ifndef RBIPOD_SYNC_H
#define RBIPOD_SYNC_H

#include "rbipod-types.h"

// =============================================================================
// SYNCHRONIZATION OPERATIONS
// =============================================================================

// Directory scanning
int count_audio_files_recursive(const char *dir_path);

// Synchronization functions
gboolean sync_directory_recursive(RbIpodDb *db, const char *dir_path, int *current_file, int total_files);
gboolean sync_single_file(RbIpodDb *db, const char *file_path);
gboolean sync_folder_filtered(RbIpodDb *db, const char *dir_path, guint32 filter_mediatype, int *current_file, int total_files);

// Signal handling
void setup_signal_handlers(void);

#endif // RBIPOD_SYNC_H