#ifndef RBIPOD_FILESYSTEM_H
#define RBIPOD_FILESYSTEM_H

#include "rbipod-types.h"

// =============================================================================
// FILESYSTEM OPERATIONS
// =============================================================================

// Filesystem detection
FilesystemType detect_filesystem_type(const char *device_path);
const char* get_filesystem_name(FilesystemType fs_type);

// Mount/unmount operations
gboolean mount_ipod_device(const char *device_path, const char *mount_point, FilesystemType fs_type);
gboolean unmount_ipod_device(const char *mount_point);
gboolean auto_mount_ipod(char **mount_point_out);

// Filesystem validation
gboolean validate_ipod_filesystem(const char *mount_point);
gboolean is_ipod_mounted(const char *mount_point);

// Device detection
char* find_ipod_device(void);

#endif // RBIPOD_FILESYSTEM_H