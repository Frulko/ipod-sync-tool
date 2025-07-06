#include <stdio.h>
#include <stdlib.h>
#include <glib.h>

#include "../include/rbipod-actions.h"
#include "../include/rbipod-logging.h"

// =============================================================================
// DELAYED ACTION MANAGEMENT (STUB IMPLEMENTATION)
// =============================================================================

void rb_ipod_add_delayed_action(RbIpodDb *db, RbIpodDelayedActionType type, gpointer data) {
    if (!db) return;
    
    RbIpodDelayedAction *action = g_malloc0(sizeof(RbIpodDelayedAction));
    action->type = type;
    
    switch (type) {
        case RB_IPOD_ACTION_SET_NAME:
            action->name = g_strdup((const gchar*)data);
            break;
        case RB_IPOD_ACTION_ADD_TRACK:
        case RB_IPOD_ACTION_REMOVE_TRACK:
            action->track = (Itdb_Track*)data;
            break;
    }
    
    g_queue_push_tail(db->delayed_actions, action);
    db->has_delayed_actions = TRUE;
    
    log_message(LOG_DEBUG, "Added delayed action type %d", type);
}

void rb_ipod_remove_delayed_action(RbIpodDb *db, RbIpodDelayedActionType type, gpointer data) {
    if (!db || !db->delayed_actions) return;
    
    // TODO: Implement removal of specific delayed actions
    log_message(LOG_DEBUG, "Removing delayed action type %d", type);
    (void)data; // Suppress unused parameter warning
}

void rb_ipod_set_name_delayed_action(RbIpodDb *db, const gchar *name) {
    rb_ipod_add_delayed_action(db, RB_IPOD_ACTION_SET_NAME, (gpointer)name);
}

void rb_ipod_add_track_delayed_action(RbIpodDb *db, Itdb_Track *track) {
    rb_ipod_add_delayed_action(db, RB_IPOD_ACTION_ADD_TRACK, track);
}

void rb_ipod_remove_track_delayed_action(RbIpodDb *db, Itdb_Track *track) {
    rb_ipod_add_delayed_action(db, RB_IPOD_ACTION_REMOVE_TRACK, track);
}

gboolean rb_ipod_db_process_delayed_actions(RbIpodDb *db) {
    if (!db || !db->has_delayed_actions) return TRUE;
    
    log_message(LOG_INFO, "Processing delayed actions");
    
    while (!g_queue_is_empty(db->delayed_actions)) {
        RbIpodDelayedAction *action = g_queue_pop_head(db->delayed_actions);
        
        switch (action->type) {
            case RB_IPOD_ACTION_SET_NAME:
                if (db->itdb && action->name) {
                    // Set iPod name
                    log_message(LOG_DEBUG, "Setting iPod name to: %s", action->name);
                }
                break;
                
            case RB_IPOD_ACTION_ADD_TRACK:
                if (db->itdb && action->track) {
                    itdb_track_add(db->itdb, action->track, -1);
                    log_message(LOG_DEBUG, "Added track to database");
                }
                break;
                
            case RB_IPOD_ACTION_REMOVE_TRACK:
                if (db->itdb && action->track) {
                    itdb_track_remove(action->track);
                    log_message(LOG_DEBUG, "Removed track from database");
                }
                break;
        }
        
        free_delayed_action(action);
    }
    
    db->has_delayed_actions = FALSE;
    return TRUE;
}

void free_delayed_action(RbIpodDelayedAction *action) {
    if (!action) return;
    
    if (action->type == RB_IPOD_ACTION_SET_NAME && action->name) {
        g_free(action->name);
    }
    
    g_free(action);
}