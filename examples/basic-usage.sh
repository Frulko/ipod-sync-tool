#!/bin/bash

# Basic Usage Examples for Rhythmbox iPod Sync - Modular Project
# ==============================================================

# Set the executable path
IPOD_SYNC="./build/rhythmbox-ipod-sync"

echo "=== Rhythmbox iPod Sync - Usage Examples ==="
echo

# Example 1: Show help and version
echo "1. Basic Information:"
$IPOD_SYNC version
echo
$IPOD_SYNC help | head -20
echo "... (truncated for brevity)"
echo

# Example 2: Mount operations (commented out - requires actual device)
echo "2. Mount Operations (example commands):"
echo "# Auto-detect and mount iPod:"
echo "$IPOD_SYNC auto-mount"
echo
echo "# Mount specific device:"
echo "$IPOD_SYNC mount /dev/sdb1 /media/ipod"
echo
echo "# Unmount iPod:"
echo "$IPOD_SYNC unmount /media/ipod"
echo

# Example 3: Sync operations (commented out - requires mounted iPod)
echo "3. Sync Operations (example commands):"
echo "# Traditional directory sync:"
echo "$IPOD_SYNC sync /media/ipod ~/Music"
echo
echo "# NEW: Single file sync with media type:"
echo "$IPOD_SYNC sync-file /media/ipod ~/Downloads/podcast.mp3 --mediatype podcast"
echo
echo "# NEW: Filtered folder sync for specific media types:"
echo "$IPOD_SYNC sync-folder-filtered /media/ipod ~/Podcasts podcast"
echo "$IPOD_SYNC sync-folder-filtered /media/ipod ~/Audiobooks audiobook"
echo "$IPOD_SYNC sync-folder-filtered /media/ipod ~/MusicVideos musicvideo"
echo

# Example 4: Information commands (commented out - requires mounted iPod)
echo "4. Information Commands (example commands):"
echo "# List tracks on iPod:"
echo "$IPOD_SYNC list /media/ipod"
echo
echo "# Show iPod information:"
echo "$IPOD_SYNC info /media/ipod"
echo

echo "=== Supported Media Types ==="
echo "audio, movie, podcast, audiobook, musicvideo, tvshow,"
echo "ringtone, rental, itunes-extra, memo, itunes-u"
echo

echo "=== Build Information ==="
echo "To build the project:"
echo "  make check-deps  # Check dependencies"
echo "  make release     # Build optimized version"
echo "  make debug       # Build with debug symbols"
echo "  make clean       # Clean build artifacts"
echo

echo "=== Project Structure Benefits ==="
echo "✓ Modular development - work on specific components"
echo "✓ Better testing - unit test individual modules"
echo "✓ Reduced compilation time - only recompile changed files"
echo "✓ Code reusability - modules can be reused"
echo "✓ Team collaboration - multiple developers can work on different modules"
echo "✓ Maintainability - easier to locate and fix issues"