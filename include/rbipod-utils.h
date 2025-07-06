#ifndef RBIPOD_UTILS_H
#define RBIPOD_UTILS_H

#include "rbipod-types.h"

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

// Application lifecycle
gboolean init_application(const char *mount_point);
void cleanup_application(void);

// Usage and version information
void print_usage(const char *program_name);
void print_version(void);

// Global sync context access
extern SyncContext g_sync_ctx;

#endif // RBIPOD_UTILS_H