/* Test basé sur le code officiel libgpod
 * Adapté pour rhythmbox-ipod-sync avec option skip thumbnails
 * 
 * Copyright (c) 2005, Christophe Fergeau <teuf@gnome.org>
 * Modifications (c) 2025, rhythmbox-ipod-sync project
 * 
 * Usage: test_libgpod_covers <mountpoint> <image-dir> [--skip-thumbnails]
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <glib.h>
#include <gpod/itdb.h>

static GList *
get_cover_list (const char *dirname)
{
    GDir *dir;
    const char *filename;
    GList *result;

    dir = g_dir_open (dirname, 0, NULL);
    if (dir == NULL) {
        return NULL;
    }
    result = NULL;
    filename = g_dir_read_name (dir);
    while (filename != NULL) {
        const char *ext;
        ext = strrchr (filename, '.');
        if (ext != NULL) {
            if ((g_ascii_strcasecmp (ext, ".png") == 0) 
                || (g_ascii_strcasecmp (ext, ".jpg") == 0)
                || (g_ascii_strcasecmp (ext, ".jpeg") == 0)) {
                char *fullpath;
                fullpath = g_build_filename (dirname, filename, NULL);
                result = g_list_prepend (result, fullpath);
            }
        }
        filename = g_dir_read_name (dir);
    }
    g_dir_close (dir);
    
    return g_list_reverse (result);
}

void print_usage(const char *program_name) {
    g_print ("Usage: %s <mountpoint> <image-dir> [--skip-thumbnails]\n", program_name);
    g_print ("\n");
    g_print ("Arguments:\n");
    g_print ("  mountpoint       Path to iPod mount point (e.g., /media/ipod)\n");
    g_print ("  image-dir        Directory containing cover images (.jpg, .jpeg, .png)\n");
    g_print ("  --skip-thumbnails Optional: Skip thumbnail processing (faster)\n");
    g_print ("\n");
    g_print ("Examples:\n");
    g_print ("  %s /media/ipod ./cover-images\n", program_name);
    g_print ("  %s /media/ipod ./fixtures --skip-thumbnails\n", program_name);
    g_print ("\n");
    g_print ("This test:\n");
    g_print ("  - Loads the iPod database\n");
    g_print ("  - Assigns random cover art to all tracks\n");
    g_print ("  - Optionally processes thumbnails\n");
    g_print ("  - Saves the database\n");
}

int
main (int argc, char **argv)
{
    Itdb_iTunesDB *db;
    GList *it;
    GList *covers;
    int nb_covers;
    gboolean skip_thumbnails = FALSE;
    const char *mountpoint;
    const char *image_dir;

    if (argc < 3) {
        print_usage(argv[0]);
        return 1;
    }
    
    mountpoint = argv[1];
    image_dir = argv[2];
    
    // Check for --skip-thumbnails flag
    if (argc >= 4 && strcmp(argv[3], "--skip-thumbnails") == 0) {
        skip_thumbnails = TRUE;
        g_print ("⚡ Thumbnail processing will be skipped for faster operation\n");
    }
    
    setlocale (LC_ALL, "");
    
    g_print ("=== libgpod Cover Art Test ===\n");
    g_print ("Mount point: %s\n", mountpoint);
    g_print ("Image directory: %s\n", image_dir);
    g_print ("Skip thumbnails: %s\n\n", skip_thumbnails ? "YES" : "NO");
    
    // Get list of cover images
    g_print ("📁 Scanning for cover images...\n");
    covers = get_cover_list (image_dir);
    if (covers == NULL) {
        g_print ("❌ Error: %s should be a directory containing pictures (.jpg, .jpeg, .png)\n", image_dir);
        return 1;
    }
    nb_covers = g_list_length (covers);
    g_print ("✓ Found %d cover image(s)\n", nb_covers);
    
    // List found covers
    GList *cover_it = covers;
    int i = 1;
    while (cover_it != NULL) {
        g_print ("  %d. %s\n", i++, (char*)cover_it->data);
        cover_it = g_list_next(cover_it);
    }
    g_print ("\n");
    
    // Load iPod database
    g_print ("💽 Loading iPod database...\n");
    db = itdb_parse (mountpoint, NULL);
    if (db == NULL) {
        g_print ("❌ Error reading iPod database from %s\n", mountpoint);
        g_print ("   Make sure the iPod is properly mounted and contains a valid database.\n");
        g_list_foreach (covers, (GFunc)g_free, NULL);
        g_list_free (covers);
        return 1;
    }
    
    int nb_tracks = g_list_length(db->tracks);
    g_print ("✓ Database loaded successfully\n");
    g_print ("📊 Found %d track(s) in database\n\n", nb_tracks);
    
    if (nb_tracks == 0) {
        g_print ("⚠️  No tracks found in database. Add some music first.\n");
        itdb_free (db);
        g_list_foreach (covers, (GFunc)g_free, NULL);
        g_list_free (covers);
        return 0;
    }
    
    // Process each track
    g_print ("🖼️  Assigning cover art to tracks...\n");
    int track_count = 0;
    
    for (it = db->tracks; it != NULL; it = it->next) {
        Itdb_Track *song;
        const char *coverpath;

        song = (Itdb_Track*)it->data;
        track_count++;
        
        if (!skip_thumbnails) {
            // Remove existing thumbnails (from official libgpod code)
            if (song->artwork) {
                itdb_artwork_remove_thumbnails (song->artwork);
            }
        }

        // Select a random cover from the available covers
        coverpath = g_list_nth_data (covers, 
                         g_random_int_range (0, nb_covers));
        
        g_print ("  %d/%d: %s - %s - %s gets %s\n", 
                track_count, nb_tracks,
                song->artist ? song->artist : "Unknown Artist",
                song->album ? song->album : "Unknown Album", 
                song->title ? song->title : "Unknown Title",
                g_path_get_basename(coverpath));

        if (!skip_thumbnails) {
            // Set the thumbnail using libgpod
            gboolean result = itdb_track_set_thumbnails (song, coverpath);
            if (!result) {
                g_print ("    ⚠️  Warning: Failed to set thumbnail\n");
            }
        } else {
            g_print ("    ⚡ Skipped thumbnail processing\n");
        }
    }
    
    g_print ("\n");
    
    // Save database
    g_print ("💾 Saving iPod database...\n");
    gboolean save_result = itdb_write (db, NULL);
    if (save_result) {
        g_print ("✓ Database saved successfully\n");
    } else {
        g_print ("❌ Error saving database\n");
        itdb_free (db);
        g_list_foreach (covers, (GFunc)g_free, NULL);
        g_list_free (covers);
        return 1;
    }
    
    // Cleanup
    g_print ("\n🧹 Cleaning up...\n");
    itdb_free (db);
    g_list_foreach (covers, (GFunc)g_free, NULL);
    g_list_free (covers);
    
    g_print ("✅ Test completed successfully!\n");
    g_print ("\n📋 Summary:\n");
    g_print ("  - Processed %d track(s)\n", track_count);
    g_print ("  - Used %d cover image(s)\n", nb_covers);
    g_print ("  - Thumbnails: %s\n", skip_thumbnails ? "Skipped" : "Processed");
    g_print ("  - Database saved\n");
    
    if (!skip_thumbnails) {
        g_print ("\n💡 Note: Check your iPod to see the new cover art!\n");
        g_print ("   If covers don't appear, try using --skip-thumbnails for compatibility.\n");
    }

    return 0;
}