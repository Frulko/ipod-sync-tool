#include <iostream>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <glib.h>

// TagLib headers
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/attachedpictureframe.h>
#include <taglib/flacfile.h>
#include <taglib/flacpicture.h>
#include <taglib/mp4file.h>
#include <taglib/mp4tag.h>
#include <taglib/mp4coverart.h>

/**
 * Test d'extraction d'artwork avec TagLib C++ API
 * 
 * Ce test vérifie que TagLib peut extraire correctement :
 * - Artwork depuis MP3 (ID3v2 APIC frames)
 * - Artwork depuis FLAC (picture blocks)
 * - Artwork depuis MP4/M4A (cover art items)
 * - Détection correcte du format (JPEG/PNG)
 */

struct AudioMetadata {
    guchar *artwork_data;
    gsize artwork_size;
    char *artwork_format;
    
    AudioMetadata() : artwork_data(nullptr), artwork_size(0), artwork_format(nullptr) {}
    
    ~AudioMetadata() {
        if (artwork_data) g_free(artwork_data);
        if (artwork_format) g_free(artwork_format);
    }
};

bool extract_artwork_mp3_test(const char *file_path, AudioMetadata *meta) {
    try {
        TagLib::MPEG::File mpegFile(file_path);
        if (!mpegFile.isValid()) return false;
        
        TagLib::ID3v2::Tag *id3v2tag = mpegFile.ID3v2Tag();
        if (!id3v2tag) return false;
        
        TagLib::ID3v2::FrameList frames = id3v2tag->frameList("APIC");
        if (frames.isEmpty()) return false;
        
        TagLib::ID3v2::AttachedPictureFrame *frame = 
            static_cast<TagLib::ID3v2::AttachedPictureFrame *>(frames.front());
        
        if (!frame) return false;
        
        TagLib::ByteVector imageData = frame->picture();
        if (imageData.isEmpty()) return false;
        
        meta->artwork_size = imageData.size();
        meta->artwork_data = static_cast<guchar*>(g_malloc(meta->artwork_size));
        memcpy(meta->artwork_data, imageData.data(), meta->artwork_size);
        
        TagLib::String mimeType = frame->mimeType();
        std::string mimeStr = mimeType.toCString();
        if (mimeStr.find("jpeg") != std::string::npos || mimeStr.find("jpg") != std::string::npos) {
            meta->artwork_format = g_strdup("jpeg");
        } else if (mimeStr.find("png") != std::string::npos) {
            meta->artwork_format = g_strdup("png");
        } else {
            meta->artwork_format = g_strdup("unknown");
        }
        
        std::cout << "  MIME type: " << mimeType.toCString() << std::endl;
        
        return true;
        
    } catch (const std::exception& e) {
        std::cout << "  Exception: " << e.what() << std::endl;
        return false;
    }
}

bool extract_artwork_flac_test(const char *file_path, AudioMetadata *meta) {
    try {
        TagLib::FLAC::File flacFile(file_path);
        if (!flacFile.isValid()) return false;
        
        TagLib::List<TagLib::FLAC::Picture*> pictures = flacFile.pictureList();
        if (pictures.isEmpty()) return false;
        
        TagLib::FLAC::Picture *picture = pictures.front();
        if (!picture) return false;
        
        TagLib::ByteVector imageData = picture->data();
        if (imageData.isEmpty()) return false;
        
        meta->artwork_size = imageData.size();
        meta->artwork_data = static_cast<guchar*>(g_malloc(meta->artwork_size));
        memcpy(meta->artwork_data, imageData.data(), meta->artwork_size);
        
        TagLib::String mimeType = picture->mimeType();
        std::string mimeStr = mimeType.toCString();
        if (mimeStr.find("jpeg") != std::string::npos || mimeStr.find("jpg") != std::string::npos) {
            meta->artwork_format = g_strdup("jpeg");
        } else if (mimeStr.find("png") != std::string::npos) {
            meta->artwork_format = g_strdup("png");
        } else {
            meta->artwork_format = g_strdup("unknown");
        }
        
        return true;
        
    } catch (const std::exception& e) {
        std::cout << "  Exception: " << e.what() << std::endl;
        return false;
    }
}

bool extract_artwork_mp4_test(const char *file_path, AudioMetadata *meta) {
    try {
        TagLib::MP4::File mp4File(file_path);
        if (!mp4File.isValid()) return false;
        
        TagLib::MP4::Tag *tag = mp4File.tag();
        if (!tag) return false;
        
        TagLib::MP4::ItemMap itemMap = tag->itemMap();
        if (!itemMap.contains("covr")) return false;
        
        TagLib::MP4::Item coverItem = itemMap["covr"];
        TagLib::MP4::CoverArtList coverList = coverItem.toCoverArtList();
        if (coverList.isEmpty()) return false;
        
        TagLib::MP4::CoverArt coverArt = coverList.front();
        TagLib::ByteVector imageData = coverArt.data();
        if (imageData.isEmpty()) return false;
        
        meta->artwork_size = imageData.size();
        meta->artwork_data = static_cast<guchar*>(g_malloc(meta->artwork_size));
        memcpy(meta->artwork_data, imageData.data(), meta->artwork_size);
        
        TagLib::MP4::CoverArt::Format format = coverArt.format();
        switch (format) {
            case TagLib::MP4::CoverArt::JPEG:
                meta->artwork_format = g_strdup("jpeg");
                break;
            case TagLib::MP4::CoverArt::PNG:
                meta->artwork_format = g_strdup("png");
                break;
            default:
                meta->artwork_format = g_strdup("unknown");
                break;
        }
        
        return true;
        
    } catch (const std::exception& e) {
        std::cout << "  Exception: " << e.what() << std::endl;
        return false;
    }
}

struct TestCase {
    const char *file_path;
    const char *format_type;
    size_t min_size;
    bool (*extract_func)(const char*, AudioMetadata*);
};

int main() {
    std::cout << "=== TagLib Artwork Extraction Tests ===" << std::endl << std::endl;
    
    std::vector<TestCase> test_cases = {
        {
            "/home/mowmow/mp3/Aaron - U-turn (Lili).mp3",
            "MP3",
            5000,
            extract_artwork_mp3_test
        },
        {
            "/home/mowmow/mp3/Daft Punk - Harder, Better, Faster, Stronger.mp3",
            "MP3",
            10000,
            extract_artwork_mp3_test
        }
        // Add FLAC and MP4 test cases when available
        // {
        //     "/path/to/test.flac",
        //     "FLAC",
        //     5000,
        //     extract_artwork_flac_test
        // },
        // {
        //     "/path/to/test.m4a",
        //     "MP4",
        //     5000,
        //     extract_artwork_mp4_test
        // }
    };
    
    int total_tests = test_cases.size();
    int passed_tests = 0;
    
    for (const auto& test_case : test_cases) {
        std::cout << "Testing " << test_case.format_type << ": " << test_case.file_path << std::endl;
        
        AudioMetadata meta;
        
        if (test_case.extract_func(test_case.file_path, &meta)) {
            std::cout << "  ✓ Artwork extracted: " << meta.artwork_size 
                      << " bytes, format: " << (meta.artwork_format ? meta.artwork_format : "unknown") << std::endl;
            
            if (meta.artwork_size >= test_case.min_size) {
                std::cout << "  ✓ Size check passed" << std::endl;
                passed_tests++;
            } else {
                std::cout << "  ✗ Size too small (expected >= " << test_case.min_size << ")" << std::endl;
            }
        } else {
            std::cout << "  ✗ No artwork found or extraction failed" << std::endl;
        }
        
        std::cout << std::endl;
    }
    
    std::cout << "=== Results ===" << std::endl;
    std::cout << "Tests passed: " << passed_tests << "/" << total_tests << std::endl;
    
    if (passed_tests == total_tests) {
        std::cout << "🎉 All tests passed!" << std::endl;
        return 0;
    } else {
        std::cout << "❌ Some tests failed." << std::endl;
        return 1;
    }
}