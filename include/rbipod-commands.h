#ifndef RBIPOD_COMMANDS_H
#define RBIPOD_COMMANDS_H

#include "rbipod-types.h"

// =============================================================================
// COMMAND IMPLEMENTATIONS
// =============================================================================

// Mount/unmount commands
int command_mount_ipod(const char *device_path, const char *mount_point);
int command_unmount_ipod(const char *mount_point);
int command_auto_mount(void);

// Sync commands
int command_sync_directory(const char *mount_point, const char *sync_dir);
int command_sync_file(const char *mount_point, const char *file_path);
int command_sync_folder_filtered(const char *mount_point, const char *folder_path, const char *mediatype_str);

// Info commands
int command_list_tracks(const char *mount_point);
int command_show_info(const char *mount_point);

// Reset commands
int command_reset_media_type(const char *mount_point, const char *media_type_str);

#endif // RBIPOD_COMMANDS_H