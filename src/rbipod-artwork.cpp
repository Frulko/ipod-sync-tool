#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <time.h>
#include <glib.h>

// TagLib C++ headers
#include <taglib/fileref.h>
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/attachedpictureframe.h>
#include <taglib/textidentificationframe.h>
#include <taglib/flacfile.h>
#include <taglib/flacpicture.h>
#include <taglib/xiphcomment.h>
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
extern "C" gboolean extract_extended_podcast_metadata(const char *file_path, AudioMetadata *meta) {
    if (!file_path || !meta) return FALSE;
    
    try {
        TagLib::FileRef fileRef(file_path);
        if (fileRef.isNull() || !fileRef.tag()) {
            return FALSE;
        }
        
        gboolean found_any = FALSE;
        
        // For MP3 files, check ID3v2 tags for extended metadata
        TagLib::MPEG::File *mpegFile = dynamic_cast<TagLib::MPEG::File*>(fileRef.file());
        if (mpegFile && mpegFile->ID3v2Tag()) {
            TagLib::ID3v2::Tag *id3v2 = mpegFile->ID3v2Tag();
            
            // Extract DATE field (TDRC, TDAT, DATE)
            TagLib::ID3v2::FrameList date_frames = id3v2->frameList("TDRC");
            if (date_frames.isEmpty()) date_frames = id3v2->frameList("TDAT");
            if (date_frames.isEmpty()) date_frames = id3v2->frameList("DATE");
            if (!date_frames.isEmpty()) {
                TagLib::String date_str = date_frames.front()->toString();
                if (!date_str.isEmpty()) {
                    std::string date_std = date_str.to8Bit(true);
                    struct tm tm_date;
                    memset(&tm_date, 0, sizeof(tm_date));
                    
                    if (strptime(date_std.c_str(), "%Y-%m-%d", &tm_date) ||
                        strptime(date_std.c_str(), "%Y/%m/%d", &tm_date) ||
                        strptime(date_std.c_str(), "%Y-%m", &tm_date) ||
                        strptime(date_std.c_str(), "%Y", &tm_date)) {
                        meta->time_released = mktime(&tm_date);
                        found_any = TRUE;
                        g_print("[DEBUG] Extracted DATE: '%s' -> %ld\n", date_std.c_str(), meta->time_released);
                    }
                }
            }
            
            // Extract GROUPING (TIT1)
            TagLib::ID3v2::FrameList grouping_frames = id3v2->frameList("TIT1");
            if (!grouping_frames.isEmpty()) {
                TagLib::String grouping_str = grouping_frames.front()->toString();
                if (!grouping_str.isEmpty()) {
                    if (meta->episode_id) g_free(meta->episode_id);
                    meta->episode_id = g_strdup(grouping_str.to8Bit(true).c_str());
                    found_any = TRUE;
                    g_print("[DEBUG] Extracted GROUPING: '%s'\n", meta->episode_id);
                }
            }
            
            // Extract SUBTITLE (TIT3)
            TagLib::ID3v2::FrameList subtitle_frames = id3v2->frameList("TIT3");
            if (!subtitle_frames.isEmpty()) {
                TagLib::String subtitle_str = subtitle_frames.front()->toString();
                if (!subtitle_str.isEmpty()) {
                    if (meta->subtitle) g_free(meta->subtitle);
                    meta->subtitle = g_strdup(subtitle_str.to8Bit(true).c_str());
                    found_any = TRUE;
                    g_print("[DEBUG] Extracted SUBTITLE: '%s'\n", meta->subtitle);
                }
            }
            
            // Extract CATEGORY (TCAT - custom frame)
            TagLib::ID3v2::FrameList category_frames = id3v2->frameList("TCAT");
            if (category_frames.isEmpty()) {
                // Try TXXX frame with description "CATEGORY"
                TagLib::ID3v2::FrameList txxx_frames = id3v2->frameList("TXXX");
                for (auto it = txxx_frames.begin(); it != txxx_frames.end(); ++it) {
                    TagLib::ID3v2::UserTextIdentificationFrame *txxx = 
                        dynamic_cast<TagLib::ID3v2::UserTextIdentificationFrame*>(*it);
                    if (txxx && txxx->description().upper() == "CATEGORY") {
                        if (!txxx->fieldList().isEmpty()) {
                            if (meta->category) g_free(meta->category);
                            meta->category = g_strdup(txxx->fieldList().back().to8Bit(true).c_str());
                            found_any = TRUE;
                            g_print("[DEBUG] Extracted CATEGORY: '%s'\n", meta->category);
                            break;
                        }
                    }
                }
            }
            
            // Extract PODCAST (show name from TALB if different from TXXX PODCAST)
            TagLib::ID3v2::FrameList podcast_frames = id3v2->frameList("TXXX");
            for (auto it = podcast_frames.begin(); it != podcast_frames.end(); ++it) {
                TagLib::ID3v2::UserTextIdentificationFrame *txxx = 
                    dynamic_cast<TagLib::ID3v2::UserTextIdentificationFrame*>(*it);
                if (txxx && (txxx->description().upper() == "PODCAST" || 
                            txxx->description().upper() == "PODCASTNAME")) {
                    if (!txxx->fieldList().isEmpty()) {
                        if (meta->podcast_name) g_free(meta->podcast_name);
                        meta->podcast_name = g_strdup(txxx->fieldList().back().to8Bit(true).c_str());
                        found_any = TRUE;
                        g_print("[DEBUG] Extracted PODCAST: '%s'\n", meta->podcast_name);
                        break;
                    }
                }
            }
            
            // Extract PODCASTURL
            TagLib::ID3v2::FrameList url_frames = id3v2->frameList("WOAS");
            if (url_frames.isEmpty()) {
                // Try TXXX frame with PODCASTURL
                for (auto it = podcast_frames.begin(); it != podcast_frames.end(); ++it) {
                    TagLib::ID3v2::UserTextIdentificationFrame *txxx = 
                        dynamic_cast<TagLib::ID3v2::UserTextIdentificationFrame*>(*it);
                    if (txxx && txxx->description().upper() == "PODCASTURL") {
                        if (!txxx->fieldList().isEmpty()) {
                            if (meta->podcasturl) g_free(meta->podcasturl);
                            meta->podcasturl = g_strdup(txxx->fieldList().back().to8Bit(true).c_str());
                            found_any = TRUE;
                            g_print("[DEBUG] Extracted PODCASTURL: '%s'\n", meta->podcasturl);
                            break;
                        }
                    }
                }
            }
            
            // Extract PODCASTRSS
            for (auto it = podcast_frames.begin(); it != podcast_frames.end(); ++it) {
                TagLib::ID3v2::UserTextIdentificationFrame *txxx = 
                    dynamic_cast<TagLib::ID3v2::UserTextIdentificationFrame*>(*it);
                if (txxx && txxx->description().upper() == "PODCASTRSS") {
                    if (!txxx->fieldList().isEmpty()) {
                        if (meta->podcastrss) g_free(meta->podcastrss);
                        meta->podcastrss = g_strdup(txxx->fieldList().back().to8Bit(true).c_str());
                        found_any = TRUE;
                        g_print("[DEBUG] Extracted PODCASTRSS: '%s'\n", meta->podcastrss);
                        break;
                    }
                }
            }
            
            // Extract DESCRIPTION (longer description)
            for (auto it = podcast_frames.begin(); it != podcast_frames.end(); ++it) {
                TagLib::ID3v2::UserTextIdentificationFrame *txxx = 
                    dynamic_cast<TagLib::ID3v2::UserTextIdentificationFrame*>(*it);
                if (txxx && (txxx->description().upper() == "DESCRIPTION" ||
                            txxx->description().upper() == "EPISODESUMMARY")) {
                    if (!txxx->fieldList().isEmpty()) {
                        if (meta->episode_summary) g_free(meta->episode_summary);
                        meta->episode_summary = g_strdup(txxx->fieldList().back().to8Bit(true).c_str());
                        found_any = TRUE;
                        g_print("[DEBUG] Extracted DESCRIPTION: '%s'\n", meta->episode_summary);
                        break;
                    }
                }
            }
        }
        
        return found_any;
        
    } catch (const std::exception& e) {
        g_print("[WARNING] Exception in date extraction: %s\n", e.what());
    }
    
    return FALSE;
}

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