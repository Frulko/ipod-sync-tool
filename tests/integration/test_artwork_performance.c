/* Test de performance pour l'artwork
 * Compare les différentes méthodes d'extraction et d'ajout d'artwork
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <glib.h>
#include <gpod/itdb.h>

// Simulation de notre structure AudioMetadata
typedef struct {
    char *title;
    char *artist;
    char *album;
    guchar *artwork_data;
    gsize artwork_size;
    char *artwork_format;
    int duration;
    int bitrate;
} TestAudioMetadata;

// Déclarations des fonctions TagLib (externes)
extern gboolean extract_artwork_taglib_native(const char *file_path, TestAudioMetadata *meta);

double get_time_diff(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

int test_artwork_extraction_performance(const char *test_file) {
    printf("🎵 Testing artwork extraction performance\n");
    printf("File: %s\n\n", test_file);
    
    struct stat st;
    if (stat(test_file, &st) != 0) {
        printf("❌ Test file not found: %s\n", test_file);
        return 0;
    }
    
    struct timespec start, end;
    
    // Test 1: TagLib native extraction
    printf("1️⃣  TagLib Native Extraction:\n");
    TestAudioMetadata meta = {0};
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    gboolean taglib_result = extract_artwork_taglib_native(test_file, &meta);
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    double taglib_time = get_time_diff(start, end);
    
    if (taglib_result) {
        printf("   ✓ Success: %zu bytes extracted in %.3f ms\n", 
               meta.artwork_size, taglib_time * 1000);
        printf("   Format: %s\n", meta.artwork_format ? meta.artwork_format : "unknown");
    } else {
        printf("   ❌ Failed\n");
    }
    
    // Nettoyer
    if (meta.artwork_data) g_free(meta.artwork_data);
    if (meta.artwork_format) g_free(meta.artwork_format);
    
    // Test 2: ffmpeg extraction (pour comparaison)
    printf("\n2️⃣  ffmpeg Extraction (fallback method):\n");
    
    char temp_file[256];
    snprintf(temp_file, sizeof(temp_file), "/tmp/perf_test_artwork_%d.jpg", getpid());
    
    char command[1024];
    snprintf(command, sizeof(command), 
             "ffmpeg -i \"%s\" -map 0:v:0 -c:v mjpeg -q:v 2 \"%s\" -y 2>/dev/null",
             test_file, temp_file);
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    int ffmpeg_result = system(command);
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    double ffmpeg_time = get_time_diff(start, end);
    
    if (ffmpeg_result == 0) {
        struct stat artwork_st;
        if (stat(temp_file, &artwork_st) == 0) {
            printf("   ✓ Success: %ld bytes extracted in %.3f ms\n", 
                   artwork_st.st_size, ffmpeg_time * 1000);
            unlink(temp_file);
        } else {
            printf("   ❌ File not created\n");
        }
    } else {
        printf("   ❌ ffmpeg failed\n");
    }
    
    // Comparaison
    printf("\n📊 Performance Comparison:\n");
    if (taglib_result && ffmpeg_result == 0) {
        printf("   TagLib: %.3f ms\n", taglib_time * 1000);
        printf("   ffmpeg: %.3f ms\n", ffmpeg_time * 1000);
        
        if (taglib_time < ffmpeg_time) {
            double speedup = ffmpeg_time / taglib_time;
            printf("   🚀 TagLib is %.1fx faster!\n", speedup);
        } else {
            double slowdown = taglib_time / ffmpeg_time;
            printf("   🐌 TagLib is %.1fx slower\n", slowdown);
        }
    }
    
    return 1;
}

int test_libgpod_artwork_performance(const char *mount_point, const char *artwork_file, int num_tracks) {
    printf("\n🎯 Testing libgpod artwork assignment performance\n");
    printf("Mount point: %s\n", mount_point);
    printf("Artwork file: %s\n", artwork_file);
    printf("Number of test tracks: %d\n\n", num_tracks);
    
    // Vérifier les fichiers
    struct stat st;
    if (stat(artwork_file, &st) != 0) {
        printf("❌ Artwork file not found: %s\n", artwork_file);
        return 0;
    }
    
    // Charger la base iPod
    Itdb_iTunesDB *itdb = itdb_parse(mount_point, NULL);
    if (!itdb) {
        printf("❌ Failed to load iPod database\n");
        return 0;
    }
    
    printf("✓ iPod database loaded\n");
    
    // Créer des tracks de test
    printf("📝 Creating %d test tracks...\n", num_tracks);
    GList *test_tracks = NULL;
    
    for (int i = 0; i < num_tracks; i++) {
        Itdb_Track *track = itdb_track_new();
        track->title = g_strdup_printf("Test Track %d", i + 1);
        track->artist = g_strdup("Performance Test Artist");
        track->album = g_strdup("Performance Test Album");
        track->tracklen = 180000; // 3 minutes
        track->bitrate = 128;
        track->size = 5000000; // 5MB
        track->filetype = g_strdup("mp3");
        track->mediatype = ITDB_MEDIATYPE_AUDIO;
        track->time_added = time(NULL);
        track->ipod_path = g_strdup_printf("/iPod_Control/Music/F00/TEST%04d.mp3", i);
        
        itdb_track_add(itdb, track, -1);
        test_tracks = g_list_prepend(test_tracks, track);
    }
    
    // Ajouter à la Master Playlist
    Itdb_Playlist *mpl = itdb_playlist_mpl(itdb);
    if (mpl) {
        GList *track_it = test_tracks;
        while (track_it) {
            itdb_playlist_add_track(mpl, (Itdb_Track*)track_it->data, -1);
            track_it = g_list_next(track_it);
        }
    }
    
    printf("✓ Test tracks created and added to database\n");
    
    // Test d'assignation d'artwork
    printf("\n🖼️  Testing artwork assignment...\n");
    
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    int successful_assignments = 0;
    GList *track_it = test_tracks;
    while (track_it) {
        Itdb_Track *track = (Itdb_Track*)track_it->data;
        gboolean result = itdb_track_set_thumbnails(track, artwork_file);
        if (result) {
            successful_assignments++;
        }
        track_it = g_list_next(track_it);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    double assignment_time = get_time_diff(start, end);
    
    printf("   ✓ Assigned artwork to %d/%d tracks in %.3f ms\n",
           successful_assignments, num_tracks, assignment_time * 1000);
    printf("   📈 Average time per track: %.3f ms\n", 
           (assignment_time * 1000) / num_tracks);
    
    // Sauvegarder la base
    printf("\n💾 Saving database...\n");
    clock_gettime(CLOCK_MONOTONIC, &start);
    gboolean save_result = itdb_write(itdb, NULL);
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    double save_time = get_time_diff(start, end);
    
    if (save_result) {
        printf("   ✓ Database saved in %.3f ms\n", save_time * 1000);
    } else {
        printf("   ❌ Failed to save database\n");
    }
    
    // Nettoyer les tracks de test
    printf("\n🧹 Cleaning up test tracks...\n");
    track_it = test_tracks;
    while (track_it) {
        Itdb_Track *track = (Itdb_Track*)track_it->data;
        if (mpl) {
            itdb_playlist_remove_track(mpl, track);
        }
        itdb_track_remove(track);
        track_it = g_list_next(track_it);
    }
    
    // Sauvegarder après nettoyage
    itdb_write(itdb, NULL);
    printf("✓ Test tracks removed\n");
    
    g_list_free(test_tracks);
    itdb_free(itdb);
    
    // Résumé des performances
    printf("\n📊 Performance Summary:\n");
    printf("   Artwork assignments: %.3f ms total, %.3f ms average\n",
           assignment_time * 1000, (assignment_time * 1000) / num_tracks);
    printf("   Database save: %.3f ms\n", save_time * 1000);
    printf("   Success rate: %d/%d (%.1f%%)\n", 
           successful_assignments, num_tracks, 
           (100.0 * successful_assignments) / num_tracks);
    
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <test_audio_file> [ipod_mount_point] [artwork_file] [num_test_tracks]\n");
        printf("\nExamples:\n");
        printf("  %s /path/to/test.mp3\n", argv[0]);
        printf("  %s /path/to/test.mp3 /media/ipod /path/to/artwork.jpg 10\n", argv[0]);
        return 1;
    }
    
    const char *test_file = argv[1];
    const char *mount_point = argc > 2 ? argv[2] : NULL;
    const char *artwork_file = argc > 3 ? argv[3] : NULL; 
    int num_tracks = argc > 4 ? atoi(argv[4]) : 5;
    
    printf("🚀 Artwork Performance Test Suite\n");
    printf("=================================\n\n");
    
    // Test 1: Extraction performance
    if (!test_artwork_extraction_performance(test_file)) {
        return 1;
    }
    
    // Test 2: libgpod performance (si iPod disponible)
    if (mount_point && artwork_file) {
        if (!test_libgpod_artwork_performance(mount_point, artwork_file, num_tracks)) {
            printf("⚠️  libgpod performance test skipped (iPod not available)\n");
        }
    } else {
        printf("\n⚠️  libgpod tests skipped (mount point or artwork file not provided)\n");
        printf("   Provide all arguments to run full performance tests\n");
    }
    
    printf("\n🎉 Performance tests completed!\n");
    return 0;
}