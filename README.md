# Rhythmbox iPod Sync - Modular Project

A modular reimplementation of the Rhythmbox iPod synchronization tool, split into manageable components for better maintainability and development.

## Project Structure

```
rhythmbox-ipod-sync-project/
├── src/                    # Source files
│   ├── main.c             # Main application entry point
│   ├── rbipod-logging.c   # Logging system implementation
│   ├── rbipod-metadata.c  # Metadata extraction and management
│   ├── rbipod-database.c  # iPod database operations
│   ├── rbipod-actions.c   # Delayed action management
│   ├── rbipod-filesystem.c # Filesystem operations
│   ├── rbipod-files.c     # File operations and track management
│   ├── rbipod-sync.c      # Synchronization logic
│   ├── rbipod-commands.c  # Command implementations
│   └── rbipod-utils.c     # Utility functions
├── include/               # Header files
│   ├── rbipod-config.h    # Configuration constants
│   ├── rbipod-types.h     # Type definitions
│   ├── rbipod-logging.h   # Logging interface
│   ├── rbipod-metadata.h  # Metadata interface
│   ├── rbipod-database.h  # Database interface
│   ├── rbipod-actions.h   # Actions interface
│   ├── rbipod-filesystem.h # Filesystem interface
│   ├── rbipod-files.h     # Files interface
│   ├── rbipod-sync.h      # Sync interface
│   ├── rbipod-commands.h  # Commands interface
│   └── rbipod-utils.h     # Utils interface
├── build/                 # Build artifacts (created during compilation)
├── docs/                  # Documentation
├── examples/              # Usage examples
├── Makefile              # Build system
└── README.md             # This file
```

## Features

### New Functionality
- **Single File Sync**: Sync individual files with specific media types
- **Filtered Folder Sync**: Sync entire folders with forced media type classification
- **Enhanced Media Type Support**: Support for podcasts, audiobooks, videos, etc.
- **🎯 FIXED: Podcast Playlist Management**: Podcasts now appear correctly in iPod's Podcasts menu
- **Smart Playlist Creation**: Automatically creates and manages special playlists for different media types

### Core Features
- Rhythmbox-inspired robust iPod management
- Asynchronous database operations with proper queuing
- Thread-safe read-only/read-write state management
- Comprehensive backup and recovery mechanisms
- Proper libgpod integration patterns
- Robust error handling and validation

## Dependencies

### Ubuntu/Debian
```bash
sudo apt-get install libgpod-dev libglib2.0-dev udisks2 util-linux
```

### CentOS/RHEL
```bash
sudo yum install libgpod-devel glib2-devel udisks2 util-linux
```

## Building

### Check Dependencies
```bash
make check-deps
```

### Build Release Version
```bash
make release
```

### Build Debug Version
```bash
make debug
```

### Show Build Information
```bash
make info
```

## Usage

### Basic Commands

**Mount/Unmount:**
```bash
./build/rhythmbox-ipod-sync auto-mount
./build/rhythmbox-ipod-sync mount /dev/sdb1 /media/ipod
./build/rhythmbox-ipod-sync unmount /media/ipod
```

**Traditional Sync:**
```bash
./build/rhythmbox-ipod-sync sync /media/ipod ~/Music
```

**New Single File Sync:**
```bash
# Sync single file as default audio type
./build/rhythmbox-ipod-sync sync-file /media/ipod ~/podcast.mp3

# Sync single file with specific media type
./build/rhythmbox-ipod-sync sync-file /media/ipod ~/podcast.mp3 --mediatype podcast
```

**New Filtered Folder Sync:**
```bash
# Sync entire folder as podcasts
./build/rhythmbox-ipod-sync sync-folder-filtered /media/ipod ~/Podcasts podcast

# Sync audiobook folder
./build/rhythmbox-ipod-sync sync-folder-filtered /media/ipod ~/Audiobooks audiobook

# Sync music videos
./build/rhythmbox-ipod-sync sync-folder-filtered /media/ipod ~/MusicVideos musicvideo
```

**Information Commands:**
```bash
./build/rhythmbox-ipod-sync list /media/ipod
./build/rhythmbox-ipod-sync info /media/ipod
```

### Supported Media Types

- `audio` - Regular music files (default)
- `movie` / `video` - Video files
- `podcast` - Podcast episodes
- `audiobook` - Audiobook chapters
- `musicvideo` / `music-video` - Music videos
- `tvshow` / `tv-show` - TV show episodes
- `ringtone` - Ringtones
- `rental` - Rental content
- `itunes-extra` / `extra` - iTunes extras
- `memo` - Voice memos
- `itunes-u` / `itunesu` - iTunes U content

## Development

### Module Organization

The project is organized into logical modules:

1. **Core Types** (`rbipod-types.h`) - All data structures and enums
2. **Configuration** (`rbipod-config.h`) - Program constants
3. **Logging** (`rbipod-logging.*`) - Thread-safe logging system
4. **Database** (`rbipod-database.*`) - iPod database management
5. **Actions** (`rbipod-actions.*`) - Delayed action queue management
6. **Metadata** (`rbipod-metadata.*`) - Audio metadata extraction
7. **Filesystem** (`rbipod-filesystem.*`) - Mount/unmount operations
8. **Files** (`rbipod-files.*`) - File operations and track creation
9. **Sync** (`rbipod-sync.*`) - Synchronization logic
10. **Commands** (`rbipod-commands.*`) - CLI command implementations
11. **Utils** (`rbipod-utils.*`) - Application lifecycle and utilities

### Adding New Features

1. **Add function declarations** to appropriate header files
2. **Implement functions** in corresponding source files
3. **Update command parsing** in `main.c` if needed
4. **Add usage information** in `rbipod-utils.c`
5. **Test compilation** with `make test-compile`

### Build Targets

- `make all` / `make release` - Optimized release build
- `make debug` - Debug build with symbols
- `make clean` - Remove build artifacts
- `make install` - Install to `/usr/local/bin`
- `make uninstall` - Remove from system
- `make check-deps` - Verify dependencies
- `make info` - Show build configuration
- `make test-compile` - Test compilation only

## Architecture Benefits

### Compared to Single File

**Advantages:**
- **Modular Development** - Work on specific components independently
- **Better Testing** - Unit test individual modules
- **Reduced Compilation Time** - Only recompile changed modules
- **Code Reusability** - Modules can be reused in other projects
- **Clearer Dependencies** - Header files make interfaces explicit
- **Team Collaboration** - Multiple developers can work on different modules
- **Maintainability** - Easier to locate and fix issues

**Maintained Features:**
- All original functionality preserved
- Same command-line interface
- Identical safety and backup mechanisms
- Compatible with existing workflows

## Current Implementation Status

This modular version currently includes:

✅ **Complete Module Structure** - All modules properly separated
✅ **Build System** - Makefile with dependency checking
✅ **Core Interfaces** - All header files with proper declarations
✅ **New Sync Features** - Single file and filtered folder sync
✅ **Enhanced Media Types** - Support for all iPod media types
✅ **Stub Implementations** - Basic functionality for all modules

🚧 **TODO for Full Implementation:**
- Complete file copying implementation in `rbipod-files.c`
- Implement proper filesystem detection in `rbipod-filesystem.c`
- Add comprehensive backup/restore in `rbipod-database.c`
- Enhance metadata extraction with actual audio tag reading
- Implement device auto-detection
- Add progress reporting improvements

## Contributing

1. Choose a module to work on
2. Implement the TODO items in the corresponding `.c` file
3. Test with `make debug` and verify functionality
4. Update documentation as needed

## License

Based on Rhythmbox's iPod management code architecture.