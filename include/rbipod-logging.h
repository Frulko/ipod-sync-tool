#ifndef RBIPOD_LOGGING_H
#define RBIPOD_LOGGING_H

#include "rbipod-types.h"

// =============================================================================
// LOGGING SYSTEM
// =============================================================================

// Global log level names
extern const char* log_level_names[];

// Logging functions
void log_message(LogLevel level, const char *format, ...);
gboolean init_logging(const char *log_file_path);
void cleanup_logging(void);

#endif // RBIPOD_LOGGING_H