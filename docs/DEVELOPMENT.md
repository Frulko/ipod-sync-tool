# Development Guide - Rhythmbox iPod Sync Modular Project

## Overview

This document provides detailed guidance for developers working on the modular rhythmbox-ipod-sync project.

## Project Architecture

### Module Dependencies

```
main.c
├── rbipod-utils.h (application lifecycle)
├── rbipod-commands.h (command implementations)
├── rbipod-metadata.h (media type parsing)
└── rbipod-logging.h (error reporting)

rbipod-commands.c
├── rbipod-database.h (iPod database operations)
├── rbipod-filesystem.h (mount/unmount)
├── rbipod-sync.h (synchronization logic)
├── rbipod-metadata.h (media type handling)
└── rbipod-utils.h (global context access)

rbipod-sync.c
├── rbipod-files.h (file operations)
├── rbipod-utils.h (cancellation handling)
└── rbipod-logging.h (progress reporting)

rbipod-files.c
├── rbipod-metadata.h (metadata extraction)
├── rbipod-database.h (track addition)
└── rbipod-utils.h (statistics tracking)

... (and so on)
```

### Key Data Flows

1. **Command Processing**: `main.c` → `rbipod-commands.c` → specific modules
2. **Sync Operations**: `rbipod-commands.c` → `rbipod-sync.c` → `rbipod-files.c`
3. **Database Operations**: `rbipod-files.c` → `rbipod-database.c` → libgpod
4. **Logging**: All modules → `rbipod-logging.c` → file/console

## Development Workflow

### Setting Up Development Environment

1. **Clone and Build**:
   ```bash
   cd rhythmbox-ipod-sync-project
   make check-deps  # Verify dependencies
   make debug       # Build with debug symbols
   ```

2. **IDE Configuration**:
   - Add `include/` to include path
   - Configure with pkg-config flags for libgpod, glib-2.0, gio-2.0
   - Enable `-Wall -Wextra` warnings

### Adding New Features

#### 1. New Command Implementation

**Example: Adding a `remove` command**

1. **Add function declaration** in `include/rbipod-commands.h`:
   ```c
   int command_remove_track(const char *mount_point, const char *track_id);
   ```

2. **Implement function** in `src/rbipod-commands.c`:
   ```c
   int command_remove_track(const char *mount_point, const char *track_id) {
       // Implementation here
   }
   ```

3. **Add command parsing** in `src/main.c`:
   ```c
   } else if (strcmp(command, "remove") == 0) {
       if (argc < 4) {
           fprintf(stderr, "Error: remove command requires track ID\n");
           result = 1;
       } else {
           result = command_remove_track(mount_point, argv[3]);
       }
   ```

4. **Update help text** in `src/rbipod-utils.c`.

#### 2. New Module Creation

**Example: Adding playlist management**

1. **Create header** `include/rbipod-playlists.h`:
   ```c
   #ifndef RBIPOD_PLAYLISTS_H
   #define RBIPOD_PLAYLISTS_H
   
   #include "rbipod-types.h"
   
   gboolean create_playlist(RbIpodDb *db, const char *name);
   gboolean add_track_to_playlist(RbIpodDb *db, const char *playlist_name, Itdb_Track *track);
   
   #endif
   ```

2. **Implement module** `src/rbipod-playlists.c`:
   ```c
   #include "../include/rbipod-playlists.h"
   #include "../include/rbipod-logging.h"
   
   gboolean create_playlist(RbIpodDb *db, const char *name) {
       // Implementation
   }
   ```

3. **Update Makefile** - it will automatically pick up new `.c` files in `src/`

#### 3. Extending Existing Modules

**Example: Enhancing metadata extraction**

1. **Add new function** to `include/rbipod-metadata.h`
2. **Implement function** in `src/rbipod-metadata.c`
3. **Use function** in appropriate calling modules

### Testing Strategy

#### 1. Module-Level Testing

Create test files for individual modules:

```bash
# Test compilation of specific module
gcc -c src/rbipod-metadata.c $(pkg-config --cflags libgpod-1.0 glib-2.0)

# Test with debug symbols
make debug
gdb ./build/rhythmbox-ipod-sync
```

#### 2. Integration Testing

```bash
# Test basic functionality
./build/rhythmbox-ipod-sync version
./build/rhythmbox-ipod-sync help

# Test error handling
./build/rhythmbox-ipod-sync invalid-command
./build/rhythmbox-ipod-sync sync # Missing arguments
```

#### 3. Memory Testing

```bash
# Build with debug symbols
make debug

# Run with valgrind (if available)
valgrind --leak-check=full ./build/rhythmbox-ipod-sync help
```

### Code Standards

#### 1. Naming Conventions

- **Functions**: `module_function_name()` (e.g., `rb_ipod_db_new()`)
- **Types**: `CamelCase` (e.g., `RbIpodDb`, `AudioMetadata`)
- **Constants**: `UPPER_SNAKE_CASE` (e.g., `MAX_PATH_LEN`)
- **Variables**: `snake_case` (e.g., `file_path`, `track_count`)

#### 2. Header Guards

All headers must use include guards:
```c
#ifndef RBIPOD_MODULE_H
#define RBIPOD_MODULE_H
// content
#endif
```

#### 3. Error Handling

- Use `gboolean` return values for success/failure
- Log errors with appropriate levels
- Clean up resources on failure paths
- Validate input parameters

#### 4. Memory Management

- Use GLib memory functions (`g_malloc()`, `g_free()`)
- Always check allocation success
- Free allocated memory in reverse order
- Set pointers to NULL after freeing

### Common Development Tasks

#### 1. Adding New Media Type Support

1. **Update** `parse_media_type_string()` in `rbipod-metadata.c`
2. **Update** `get_media_type_name()` in `rbipod-metadata.c`
3. **Test** with sync commands using `--mediatype`

#### 2. Enhancing File Operations

1. **Check** `rbipod-files.c` for file handling functions
2. **Add** new file type support in `is_supported_audio_file()`
3. **Enhance** metadata extraction as needed

#### 3. Improving Database Operations

1. **Review** `rbipod-database.c` for database functions
2. **Add** backup/restore improvements
3. **Enhance** error recovery mechanisms

## Debugging Tips

### 1. Enable Debug Logging

Build with debug mode and check log output:
```bash
make debug
./build/rhythmbox-ipod-sync sync /media/ipod ~/test
cat ipod_sync.log
```

### 2. Common Issues

- **Missing includes**: Add required system headers
- **Link errors**: Check Makefile LDFLAGS
- **Segmentation faults**: Check pointer validity before dereferencing
- **Memory leaks**: Ensure all allocations have corresponding frees

### 3. Module Isolation

Test modules independently by temporarily stubbing dependencies.

## Build System Details

### Makefile Targets

- `make release` - Optimized build (`-O2`)
- `make debug` - Debug build (`-g -DDEBUG`)
- `make clean` - Remove build artifacts
- `make info` - Show build configuration
- `make check-deps` - Verify dependencies

### Adding New Dependencies

1. **Update** `PKG_CONFIG` packages in Makefile
2. **Test** with `make check-deps`
3. **Document** in README.md

## Contributing Guidelines

1. **Test** changes with `make test-compile`
2. **Verify** no new warnings introduced
3. **Update** documentation as needed
4. **Follow** existing code style
5. **Add** appropriate logging statements
6. **Check** memory management

## Future Enhancements

### Priority Items

1. **Complete file copying** implementation
2. **Implement device auto-detection**
3. **Add comprehensive backup/restore**
4. **Enhance metadata extraction** with actual tag reading
5. **Add progress reporting** improvements

### Long-term Goals

1. **GUI interface** using the modular backend
2. **Plugin system** for extended functionality
3. **Network sync** capabilities
4. **Multiple device** support
5. **Advanced playlist** management

## Resources

- **libgpod Documentation**: Available in system docs
- **GLib Reference**: https://docs.gtk.org/glib/
- **Original Rhythmbox Code**: For reference patterns
- **iPod Technical Info**: For understanding device specifics