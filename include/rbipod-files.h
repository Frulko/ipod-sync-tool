#ifndef RBIPOD_FILES_H
#define RBIPOD_FILES_H

#include "rbipod-types.h"

// =============================================================================
// FILE OPERATIONS AND TRACK MANAGEMENT
// =============================================================================

// iPod file naming
char* build_ipod_dir_name(const char *mount_point);
char* utf8_to_ascii(const char *utf8_string);
char* generate_ipod_filename(const char *mount_point, const char *original_filename);

// File operations
gboolean copy_file_to_ipod(const char *source_path, const char *dest_path);
gboolean is_supported_audio_file(const char *filename);

// Track creation and management
Itdb_Track* create_ipod_track_from_metadata(const AudioMetadata *meta, const char *ipod_path, const char *media_type);
gboolean add_file_to_ipod(RbIpodDb *db, const char *file_path);

// Audio metadata extraction
gboolean extract_audio_duration(const char *file_path, int *duration, int *bitrate);
gboolean probe_audio_file(const char *file_path, AudioMetadata *meta);

#endif // RBIPOD_FILES_H