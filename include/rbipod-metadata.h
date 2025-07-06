#ifndef RBIPOD_METADATA_H
#define RBIPOD_METADATA_H

#include "rbipod-types.h"

// =============================================================================
// METADATA EXTRACTION AND MANAGEMENT
// =============================================================================

// Metadata operations
AudioMetadata* extract_metadata_from_filename(const char *filename);
void free_metadata(AudioMetadata *meta);

// Media type handling
guint32 parse_media_type_string(const char *media_type_str);
const char* get_media_type_name(guint32 mediatype);
void parse_mediatype_arg(int argc, char *argv[], int start_index, char **mediatype_str);

// Podcast-specific metadata utilities
void set_podcast_metadata(AudioMetadata *meta, const char *podcast_name, int season, int episode, 
                         const char *episode_id, time_t release_date);
void set_podcast_description(AudioMetadata *meta, const char *description, const char *summary);
void set_podcast_urls(AudioMetadata *meta, const char *podcast_url, const char *rss_url);

#endif // RBIPOD_METADATA_H