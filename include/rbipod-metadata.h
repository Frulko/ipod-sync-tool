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

#endif // RBIPOD_METADATA_H