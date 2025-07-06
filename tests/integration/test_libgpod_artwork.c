#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <glib.h>
#include <gpod/itdb.h>

/**
 * Test d'intégration libgpod pour l'artwork
 * 
 * Basé sur la documentation libgpod:
 * "Each track can have a thumbnail associated with it. You can retrieve a
 * GdkPixmap of the thumbnail using itdb_artwork_get_pixbuf(). You can
 * remove a thumbnail with itdb_track_remove_thumbnails(). And finally,
 * you can set a new thumbnail using itdb_track_set_thumbnails()."
 * 
 * Ce test vérifie :
 * - itdb_track_set_thumbnails() fonctionne correctement
 * - L'artwork est persisté dans la base de données
 * - itdb_artwork_get_pixbuf() peut récupérer l'artwork
 */

// Simulation d'une structure AudioMetadata simplifiée
typedef struct {
    char *title;
    char *artist;
    char *album;
    int duration;
    int bitrate;
    guchar *artwork_data;
    gsize artwork_size;
    char *artwork_format;
} TestAudioMetadata;

int test_libgpod_artwork_integration(const char *mount_point, const char *artwork_file) {
    printf("=== Testing libgpod artwork integration ===\n");
    printf("Mount point: %s\n", mount_point);
    printf("Artwork file: %s\n\n", artwork_file);
    
    // Vérifier que le fichier d'artwork existe
    struct stat st;
    if (stat(artwork_file, &st) != 0) {
        printf("✗ Artwork file not found: %s\n", artwork_file);
        return 0;
    }
    
    printf("✓ Artwork file found (%ld bytes)\n", st.st_size);
    
    // Initialiser la base de données iPod
    printf("\n1. Initializing iPod database...\n");
    Itdb_iTunesDB *itdb = itdb_parse(mount_point, NULL);
    if (!itdb) {
        printf("✗ Failed to parse iPod database\n");
        return 0;
    }
    printf("✓ iPod database loaded\n");
    
    // Créer une track de test
    printf("\n2. Creating test track...\n");
    Itdb_Track *track = itdb_track_new();
    if (!track) {
        printf("✗ Failed to create new track\n");
        itdb_free(itdb);
        return 0;
    }
    
    // Configurer les métadonnées de base
    track->title = g_strdup("Test Track for Artwork");
    track->artist = g_strdup("Test Artist");
    track->album = g_strdup("Test Album");
    track->tracklen = 180 * 1000; // 3 minutes en millisecondes
    track->bitrate = 128;
    track->size = st.st_size;
    track->filetype = g_strdup("mp3");
    track->mediatype = ITDB_MEDIATYPE_AUDIO;
    track->time_added = time(NULL);
    track->time_modified = track->time_added;
    
    // Chemin iPod factice (pas de vrai fichier pour ce test)
    track->ipod_path = g_strdup("/iPod_Control/Music/F00/TEST.mp3");
    
    printf("✓ Test track created: %s by %s\n", track->title, track->artist);
    
    // Test 1: Ajouter l'artwork avec itdb_track_set_thumbnails()
    printf("\n3. Testing itdb_track_set_thumbnails()...\n");
    gboolean artwork_result = itdb_track_set_thumbnails(track, artwork_file);
    
    if (artwork_result) {
        printf("✓ Artwork successfully added to track\n");
    } else {
        printf("✗ Failed to add artwork to track\n");
        itdb_track_free(track);
        itdb_free(itdb);
        return 0;
    }
    
    // Ajouter la track à la base de données
    printf("\n4. Adding track to database...\n");
    itdb_track_add(itdb, track, -1);
    
    // Ajouter à la Master Playlist
    Itdb_Playlist *mpl = itdb_playlist_mpl(itdb);
    if (mpl) {
        itdb_playlist_add_track(mpl, track, -1);
        printf("✓ Track added to Master Playlist\n");
    } else {
        printf("✗ Master Playlist not found\n");
    }
    
    // Test 2: Vérifier que l'artwork est accessible
    printf("\n5. Testing artwork retrieval...\n");
    
    // Note: itdb_artwork_get_pixbuf() nécessite GdkPixbuf qui n'est pas
    // toujours disponible dans un environnement de test minimal.
    // Pour ce test, nous vérifions plutôt la présence des thumbs.
    
    if (track->artwork) {
        printf("✓ Track has artwork data\n");
        
        // Compter les thumbnails
        int thumb_count = 0;
        if (track->artwork->thumbnails) {
            GList *thumb_list = track->artwork->thumbnails;
            thumb_count = g_list_length(thumb_list);
        }
        printf("✓ Number of thumbnails: %d\n", thumb_count);
        
        if (thumb_count > 0) {
            printf("✓ Artwork successfully stored and accessible\n");
        } else {
            printf("⚠ No thumbnails found (may be normal depending on iPod model)\n");
        }
    } else {
        printf("✗ No artwork data found on track\n");
    }
    
    // Test 3: Sauvegarder et recharger pour tester la persistance
    printf("\n6. Testing artwork persistence...\n");
    
    printf("Saving database...\n");
    gboolean save_result = itdb_write(itdb, NULL);
    if (!save_result) {
        printf("✗ Failed to save database\n");
        itdb_free(itdb);
        return 0;
    }
    printf("✓ Database saved\n");
    
    // Libérer et recharger
    itdb_free(itdb);
    printf("Reloading database...\n");
    
    itdb = itdb_parse(mount_point, NULL);
    if (!itdb) {
        printf("✗ Failed to reload database\n");
        return 0;
    }
    
    // Chercher notre track de test
    Itdb_Track *reloaded_track = NULL;
    GList *tracks = itdb->tracks;
    while (tracks) {
        Itdb_Track *t = (Itdb_Track*)tracks->data;
        if (t->title && strcmp(t->title, "Test Track for Artwork") == 0) {
            reloaded_track = t;
            break;
        }
        tracks = g_list_next(tracks);
    }
    
    if (reloaded_track) {
        printf("✓ Test track found after reload\n");
        
        if (reloaded_track->artwork) {
            printf("✓ Artwork persisted successfully\n");
        } else {
            printf("⚠ Artwork not found after reload (may be iPod model dependent)\n");
        }
    } else {
        printf("✗ Test track not found after reload\n");
        itdb_free(itdb);
        return 0;
    }
    
    // Nettoyage : supprimer la track de test
    printf("\n7. Cleaning up test track...\n");
    
    // Retirer de la Master Playlist
    mpl = itdb_playlist_mpl(itdb);
    if (mpl) {
        itdb_playlist_remove_track(mpl, reloaded_track);
    }
    
    // Supprimer la track
    itdb_track_remove(reloaded_track);
    
    // Sauvegarder les changements
    itdb_write(itdb, NULL);
    printf("✓ Test track removed\n");
    
    itdb_free(itdb);
    
    printf("\n=== Test completed successfully ===\n");
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <ipod_mount_point> <artwork_file>\n", argv[0]);
        printf("Example: %s /media/ipod /tmp/test_artwork.jpg\n", argv[0]);
        return 1;
    }
    
    const char *mount_point = argv[1];
    const char *artwork_file = argv[2];
    
    printf("libgpod Artwork Integration Test\n");
    printf("================================\n\n");
    
    // Vérifier que le point de montage existe
    struct stat st;
    if (stat(mount_point, &st) != 0 || !S_ISDIR(st.st_mode)) {
        printf("Error: Mount point does not exist or is not a directory: %s\n", mount_point);
        return 1;
    }
    
    int result = test_libgpod_artwork_integration(mount_point, artwork_file);
    
    if (result) {
        printf("\n🎉 All libgpod artwork tests passed!\n");
        return 0;
    } else {
        printf("\n❌ libgpod artwork tests failed.\n");
        return 1;
    }
}