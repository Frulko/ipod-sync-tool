#ifndef RBIPOD_DATABASE_H
#define RBIPOD_DATABASE_H

#include "rbipod-types.h"

// =============================================================================
// DATABASE MANAGEMENT
// =============================================================================

// Database creation and cleanup
RbIpodDb* rb_ipod_db_new(const char *mount_point);
void rb_ipod_db_free(RbIpodDb *db);

// Database saving operations
gboolean rb_ipod_db_save_sync(RbIpodDb *db);
gboolean rb_ipod_db_save_async(RbIpodDb *db);

// Database backup and recovery
gboolean create_database_backup(RbIpodDb *db);
gboolean restore_database_backup(RbIpodDb *db);
void cleanup_backup_files(RbIpodDb *db);

// Database validation
gboolean validate_ipod_database(RbIpodDb *db);

#endif // RBIPOD_DATABASE_H