#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <glib.h>

// TagLib C++ headers
#include <taglib/fileref.h>
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/attachedpictureframe.h>
#include <taglib/flacfile.h>
#include <taglib/flacpicture.h>
#include <taglib/mp4file.h>
#include <taglib/mp4tag.h>
#include <taglib/mp4coverart.h>

extern "C" {
#include "../include/rbipod-types.h"
#include "../include/rbipod-logging.h"
}

// Extract artwork from MP3 files using ID3v2 APIC frames
extern "C" gboolean extract_artwork_mp3_id3v2(const char *file_path, AudioMetadata *meta) {
    if (!file_path || !meta) return FALSE;
    
    try {
        TagLib::MPEG::File mpegFile(file_path);
        if (!mpegFile.isValid()) return FALSE;
        
        TagLib::ID3v2::Tag *id3v2tag = mpegFile.ID3v2Tag();
        if (!id3v2tag) return FALSE;
        
        TagLib::ID3v2::FrameList frames = id3v2tag->frameList("APIC");
        if (frames.isEmpty()) return FALSE;
        
        // Get the first cover art frame (usually front cover)
        TagLib::ID3v2::AttachedPictureFrame *frame = 
            static_cast<TagLib::ID3v2::AttachedPictureFrame *>(frames.front());
        
        if (!frame) return FALSE;
        
        TagLib::ByteVector imageData = frame->picture();
        if (imageData.isEmpty()) return FALSE;
        
        // Copy artwork data to metadata structure
        meta->artwork_size = imageData.size();
        meta->artwork_data = static_cast<guchar*>(g_malloc(meta->artwork_size));
        memcpy(meta->artwork_data, imageData.data(), meta->artwork_size);
        
        // Determine format from MIME type
        TagLib::String mimeType = frame->mimeType();
        std::string mimeStr = mimeType.toCString();
        if (mimeStr.find("jpeg") != std::string::npos || mimeStr.find("jpg") != std::string::npos) {
            meta->artwork_format = g_strdup("jpeg");
        } else if (mimeStr.find("png") != std::string::npos) {
            meta->artwork_format = g_strdup("png");
        } else {
            meta->artwork_format = g_strdup("unknown");
        }
        
        log_message(LOG_DEBUG, "Extracted MP3 artwork: %zu bytes, MIME: %s", 
                   meta->artwork_size, mimeType.toCString());
        
        return TRUE;
        
    } catch (const std::exception& e) {
        log_message(LOG_WARNING, "TagLib exception during MP3 artwork extraction: %s", e.what());
        return FALSE;
    }
}

// Extract artwork from FLAC files using picture metadata blocks
extern "C" gboolean extract_artwork_flac(const char *file_path, AudioMetadata *meta) {
    if (!file_path || !meta) return FALSE;
    
    try {
        TagLib::FLAC::File flacFile(file_path);
        if (!flacFile.isValid()) return FALSE;
        
        TagLib::List<TagLib::FLAC::Picture*> pictures = flacFile.pictureList();
        if (pictures.isEmpty()) return FALSE;
        
        // Get the first picture (usually front cover)
        TagLib::FLAC::Picture *picture = pictures.front();
        if (!picture) return FALSE;
        
        TagLib::ByteVector imageData = picture->data();
        if (imageData.isEmpty()) return FALSE;
        
        // Copy artwork data to metadata structure
        meta->artwork_size = imageData.size();
        meta->artwork_data = static_cast<guchar*>(g_malloc(meta->artwork_size));
        memcpy(meta->artwork_data, imageData.data(), meta->artwork_size);
        
        // Determine format from MIME type
        TagLib::String mimeType = picture->mimeType();
        std::string mimeStr = mimeType.toCString();
        if (mimeStr.find("jpeg") != std::string::npos || mimeStr.find("jpg") != std::string::npos) {
            meta->artwork_format = g_strdup("jpeg");
        } else if (mimeStr.find("png") != std::string::npos) {
            meta->artwork_format = g_strdup("png");
        } else {
            meta->artwork_format = g_strdup("unknown");
        }
        
        log_message(LOG_DEBUG, "Extracted FLAC artwork: %zu bytes, MIME: %s", 
                   meta->artwork_size, mimeType.toCString());
        
        return TRUE;
        
    } catch (const std::exception& e) {
        log_message(LOG_WARNING, "TagLib exception during FLAC artwork extraction: %s", e.what());
        return FALSE;
    }
}

// Extract artwork from MP4/M4A files using cover art items
extern "C" gboolean extract_artwork_mp4(const char *file_path, AudioMetadata *meta) {
    if (!file_path || !meta) return FALSE;
    
    try {
        TagLib::MP4::File mp4File(file_path);
        if (!mp4File.isValid()) return FALSE;
        
        TagLib::MP4::Tag *tag = mp4File.tag();
        if (!tag) return FALSE;
        
        TagLib::MP4::ItemMap itemMap = tag->itemMap();
        if (!itemMap.contains("covr")) return FALSE;
        
        TagLib::MP4::Item coverItem = itemMap["covr"];
        TagLib::MP4::CoverArtList coverList = coverItem.toCoverArtList();
        if (coverList.isEmpty()) return FALSE;
        
        // Get the first cover art
        TagLib::MP4::CoverArt coverArt = coverList.front();
        TagLib::ByteVector imageData = coverArt.data();
        if (imageData.isEmpty()) return FALSE;
        
        // Copy artwork data to metadata structure
        meta->artwork_size = imageData.size();
        meta->artwork_data = static_cast<guchar*>(g_malloc(meta->artwork_size));
        memcpy(meta->artwork_data, imageData.data(), meta->artwork_size);
        
        // Determine format from TagLib format
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
        
        log_message(LOG_DEBUG, "Extracted MP4 artwork: %zu bytes, format: %s", 
                   meta->artwork_size, meta->artwork_format);
        
        return TRUE;
        
    } catch (const std::exception& e) {
        log_message(LOG_WARNING, "TagLib exception during MP4 artwork extraction: %s", e.what());
        return FALSE;
    }
}

// Main TagLib artwork extraction function
extern "C" gboolean extract_artwork_taglib_native(const char *file_path, AudioMetadata *meta) {
    if (!file_path || !meta) return FALSE;
    
    // Determine file type from extension
    const char *ext = strrchr(file_path, '.');
    if (!ext) return FALSE;
    
    // Convert to lowercase for comparison
    gchar *ext_lower = g_ascii_strdown(ext, -1);
    
    gboolean success = FALSE;
    
    if (g_str_equal(ext_lower, ".mp3")) {
        success = extract_artwork_mp3_id3v2(file_path, meta);
    } else if (g_str_equal(ext_lower, ".flac")) {
        success = extract_artwork_flac(file_path, meta);
    } else if (g_str_equal(ext_lower, ".m4a") || g_str_equal(ext_lower, ".mp4")) {
        success = extract_artwork_mp4(file_path, meta);
    }
    
    g_free(ext_lower);
    
    if (success) {
        log_message(LOG_DEBUG, "TagLib native artwork extraction successful: %s (%zu bytes)", 
                   file_path, meta->artwork_size);
    } else {
        log_message(LOG_DEBUG, "TagLib native artwork extraction failed or unsupported format: %s", 
                   file_path);
    }
    
    return success;
}