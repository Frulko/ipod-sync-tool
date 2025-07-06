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
void initialize_ipod_file_counter(const char *mount_point);

// File operations
gboolean ensure_ipod_directory_structure(const char *mount_point);
gboolean copy_file_to_ipod(const char *source_path, const char *dest_path);
gboolean is_supported_audio_file(const char *filename);

// Track creation and management
Itdb_Track* create_ipod_track_from_metadata(const AudioMetadata *meta, const char *ipod_path, const char *media_type);
gboolean add_file_to_ipod(RbIpodDb *db, const char *file_path);

// Audio metadata extraction
gboolean extract_audio_duration(const char *file_path, int *duration, int *bitrate);
gboolean extract_audio_duration_mediainfo(const char *file_path, int *duration, int *bitrate);
gboolean extract_metadata_field(const char *file_path, const char *field_name, char **result);
gboolean extract_audio_metadata_full(const char *file_path, AudioMetadata *meta);
gboolean extract_audio_metadata_taglib(const char *file_path, AudioMetadata *meta);
gboolean extract_artwork_ffmpeg(const char *file_path, AudioMetadata *meta);
gboolean extract_podcast_specific_metadata(const char *file_path, AudioMetadata *meta);
gboolean probe_audio_file(const char *file_path, AudioMetadata *meta);

// TagLib native artwork extraction (C++ functions)
#ifdef __cplusplus
extern "C" {
#endif
gboolean extract_artwork_taglib_native(const char *file_path, AudioMetadata *meta);
gboolean extract_artwork_mp3_id3v2(const char *file_path, AudioMetadata *meta);
gboolean extract_artwork_flac(const char *file_path, AudioMetadata *meta);
gboolean extract_artwork_mp4(const char *file_path, AudioMetadata *meta);
gboolean extract_extended_podcast_metadata(const char *file_path, AudioMetadata *meta);
#ifdef __cplusplus
}
#endif

#endif // RBIPOD_FILES_H