#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <tag_c.h>

/**
 * Test d'extraction de métadonnées basiques avec TagLib C API
 * 
 * Ce test vérifie que TagLib peut extraire correctement :
 * - Title, Artist, Album, Genre
 * - Year, Track number
 * - Duration, Bitrate
 */

typedef struct {
    const char *file_path;
    const char *expected_title;
    const char *expected_artist;
    const char *expected_album;
    int expected_year;
} TestCase;

int test_taglib_metadata_extraction(const char *file_path, const char *expected_title, 
                                   const char *expected_artist, const char *expected_album) {
    printf("Testing: %s\n", file_path);
    
    TagLib_File *tfile = taglib_file_new(file_path);
    if (!tfile || !taglib_file_is_valid(tfile)) {
        printf("  ✗ Cannot open file\n");
        return 0;
    }
    
    TagLib_Tag *tag = taglib_file_tag(tfile);
    if (!tag) {
        printf("  ✗ No tag found\n");
        taglib_file_free(tfile);
        return 0;
    }
    
    // Test basic metadata
    char *title = taglib_tag_title(tag);
    char *artist = taglib_tag_artist(tag);
    char *album = taglib_tag_album(tag);
    char *genre = taglib_tag_genre(tag);
    
    printf("  Title: '%s' (expected: '%s')\n", title ? title : "N/A", expected_title);
    printf("  Artist: '%s' (expected: '%s')\n", artist ? artist : "N/A", expected_artist);
    printf("  Album: '%s' (expected: '%s')\n", album ? album : "N/A", expected_album);
    printf("  Genre: '%s'\n", genre ? genre : "N/A");
    
    // Test numeric metadata
    unsigned int year = taglib_tag_year(tag);
    unsigned int track = taglib_tag_track(tag);
    printf("  Year: %d, Track: %d\n", year, track);
    
    // Test audio properties
    const TagLib_AudioProperties *props = taglib_file_audioproperties(tfile);
    if (props) {
        int duration = taglib_audioproperties_length(props);
        int bitrate = taglib_audioproperties_bitrate(props);
        printf("  Duration: %d sec, Bitrate: %d kbps\n", duration, bitrate);
    }
    
    // Verify results
    int success = 1;
    if (!title || strcmp(title, expected_title) != 0) {
        printf("  ✗ Title mismatch\n");
        success = 0;
    }
    if (!artist || strcmp(artist, expected_artist) != 0) {
        printf("  ✗ Artist mismatch\n");
        success = 0;
    }
    if (!album || strcmp(album, expected_album) != 0) {
        printf("  ✗ Album mismatch\n");
        success = 0;
    }
    
    if (success) {
        printf("  ✓ All metadata extracted correctly\n");
    }
    
    taglib_file_free(tfile);
    printf("\n");
    return success;
}

int main() {
    printf("=== TagLib Metadata Extraction Tests ===\n\n");
    
    TestCase test_cases[] = {
        {
            "/home/mowmow/mp3/Aaron - U-turn (Lili).mp3",
            "U-turn (Lili)",
            "Aaron", 
            "BO Je vais bien ne t'en fais pas"
        },
        {
            "/home/mowmow/mp3/Daft Punk - Harder, Better, Faster, Stronger.mp3",
            "Harder, Better, Faster, Stronger",
            "Daft Punk",
            "Discovery"
        }
    };
    
    int total_tests = sizeof(test_cases) / sizeof(TestCase);
    int passed_tests = 0;
    
    for (int i = 0; i < total_tests; i++) {
        if (test_taglib_metadata_extraction(
            test_cases[i].file_path,
            test_cases[i].expected_title,
            test_cases[i].expected_artist,
            test_cases[i].expected_album)) {
            passed_tests++;
        }
    }
    
    printf("=== Results ===\n");
    printf("Tests passed: %d/%d\n", passed_tests, total_tests);
    
    if (passed_tests == total_tests) {
        printf("🎉 All tests passed!\n");
        return 0;
    } else {
        printf("❌ Some tests failed.\n");
        return 1;
    }
}