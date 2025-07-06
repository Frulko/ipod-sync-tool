# Rhythmbox iPod Sync Project Makefile
# ====================================

# Project configuration
PROJECT_NAME = rhythmbox-ipod-sync
VERSION = 1.0.0
BUILD_DIR = build
SRC_DIR = src
INCLUDE_DIR = include

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -I$(INCLUDE_DIR)
LDFLAGS = -pthread
DEBUG_FLAGS = -g -DDEBUG
RELEASE_FLAGS = -O2 -DNDEBUG

# Package configuration
PKG_CONFIG = pkg-config
PACKAGES = libgpod-1.0 glib-2.0 gio-2.0

# Get package flags
CFLAGS += $(shell $(PKG_CONFIG) --cflags $(PACKAGES))
LDFLAGS += $(shell $(PKG_CONFIG) --libs $(PACKAGES))

# Source files
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
TARGET = $(BUILD_DIR)/$(PROJECT_NAME)

# Header dependencies
HEADERS = $(wildcard $(INCLUDE_DIR)/*.h)

# Default target
.PHONY: all
all: release

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Release build
.PHONY: release
release: CFLAGS += $(RELEASE_FLAGS)
release: $(TARGET)

# Debug build
.PHONY: debug
debug: CFLAGS += $(DEBUG_FLAGS)
debug: $(TARGET)

# Main target
$(TARGET): $(OBJECTS) | $(BUILD_DIR)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo "Build complete: $@"

# Object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
	@echo "Clean complete"

# Install target
.PHONY: install
install: release
	sudo cp $(TARGET) /usr/local/bin/
	sudo chmod +x /usr/local/bin/$(PROJECT_NAME)
	@echo "Installed to /usr/local/bin/$(PROJECT_NAME)"

# Uninstall target
.PHONY: uninstall
uninstall:
	sudo rm -f /usr/local/bin/$(PROJECT_NAME)
	@echo "Uninstalled $(PROJECT_NAME)"

# Check dependencies
.PHONY: check-deps
check-deps:
	@echo "Checking dependencies..."
	@$(PKG_CONFIG) --exists $(PACKAGES) && echo "✓ All dependencies found" || (echo "✗ Missing dependencies. Please install:" && echo "  Ubuntu/Debian: sudo apt-get install libgpod-dev libglib2.0-dev" && echo "  CentOS/RHEL: sudo yum install libgpod-devel glib2-devel" && exit 1)

# Show build info
.PHONY: info
info:
	@echo "Project: $(PROJECT_NAME) v$(VERSION)"
	@echo "Compiler: $(CC)"
	@echo "Packages: $(PACKAGES)"
	@echo "CFLAGS: $(CFLAGS)"
	@echo "LDFLAGS: $(LDFLAGS)"
	@echo "Sources: $(SOURCES)"
	@echo "Target: $(TARGET)"

# Test compile (without linking)
.PHONY: test-compile
test-compile: $(OBJECTS)
	@echo "Compilation test successful"

# Help target
.PHONY: help
help:
	@echo "Available targets:"
	@echo "  all          - Build release version (default)"
	@echo "  release      - Build optimized release version"
	@echo "  debug        - Build debug version with symbols"
	@echo "  clean        - Remove build artifacts"
	@echo "  install      - Install to /usr/local/bin"
	@echo "  uninstall    - Remove from /usr/local/bin"
	@echo "  check-deps   - Check if dependencies are installed"
	@echo "  info         - Show build configuration"
	@echo "  test-compile - Test compilation without linking"
	@echo "  help         - Show this help message"

# Dependency tracking
-include $(OBJECTS:.o=.d)

$(BUILD_DIR)/%.d: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@$(CC) -M $(CFLAGS) $< | sed 's|^.*:|$(BUILD_DIR)/$*.o:|' > $@