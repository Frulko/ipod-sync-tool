#ifndef RBIPOD_ACTIONS_H
#define RBIPOD_ACTIONS_H

#include "rbipod-types.h"

// =============================================================================
// DELAYED ACTION MANAGEMENT
// =============================================================================

// Delayed action operations
void rb_ipod_add_delayed_action(RbIpodDb *db, RbIpodDelayedActionType type, gpointer data);
void rb_ipod_remove_delayed_action(RbIpodDb *db, RbIpodDelayedActionType type, gpointer data);
void rb_ipod_set_name_delayed_action(RbIpodDb *db, const gchar *name);
void rb_ipod_add_track_delayed_action(RbIpodDb *db, Itdb_Track *track);
void rb_ipod_remove_track_delayed_action(RbIpodDb *db, Itdb_Track *track);

// Action processing
gboolean rb_ipod_db_process_delayed_actions(RbIpodDb *db);

// Action cleanup
void free_delayed_action(RbIpodDelayedAction *action);

#endif // RBIPOD_ACTIONS_H