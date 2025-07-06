# Tests Makefile for rhythmbox-ipod-sync
# =====================================

# Configuration
CC = gcc
CXX = g++
CFLAGS = -Wall -Wextra -std=c99 -I../include
CXXFLAGS = -Wall -Wextra -std=c++11 -I../include
LDFLAGS = -pthread

# Package configuration
PKG_CONFIG = pkg-config
PACKAGES = libgpod-1.0 glib-2.0 taglib_c taglib
PKG_CFLAGS = $(shell $(PKG_CONFIG) --cflags $(PACKAGES))
PKG_LDFLAGS = $(shell $(PKG_CONFIG) --libs $(PACKAGES))

CFLAGS += $(PKG_CFLAGS)
CXXFLAGS += $(PKG_CFLAGS)
LDFLAGS += $(PKG_LDFLAGS)

# Directories
UNIT_DIR = unit
INTEGRATION_DIR = integration
BUILD_DIR = build

# Test targets
UNIT_TESTS = $(BUILD_DIR)/test_taglib_metadata $(BUILD_DIR)/test_taglib_artwork
INTEGRATION_TESTS = $(BUILD_DIR)/test_libgpod_artwork $(BUILD_DIR)/test_libgpod_covers $(BUILD_DIR)/test_artwork_performance

ALL_TESTS = $(UNIT_TESTS) $(INTEGRATION_TESTS)

# Default target
.PHONY: all
all: $(ALL_TESTS)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Unit tests
$(BUILD_DIR)/test_taglib_metadata: $(UNIT_DIR)/test_taglib_metadata.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

$(BUILD_DIR)/test_taglib_artwork: $(UNIT_DIR)/test_taglib_artwork.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

# Integration tests
$(BUILD_DIR)/test_libgpod_artwork: $(INTEGRATION_DIR)/test_libgpod_artwork.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

$(BUILD_DIR)/test_libgpod_covers: $(INTEGRATION_DIR)/test_libgpod_covers.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

$(BUILD_DIR)/test_artwork_performance: $(INTEGRATION_DIR)/test_artwork_performance.c ../build/rbipod-artwork.o | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $< ../build/rbipod-artwork.o -o $@ $(LDFLAGS)

# Run individual tests
.PHONY: test-metadata
test-metadata: $(BUILD_DIR)/test_taglib_metadata
	@echo "=== Running TagLib Metadata Tests ==="
	@./$(BUILD_DIR)/test_taglib_metadata

.PHONY: test-artwork
test-artwork: $(BUILD_DIR)/test_taglib_artwork
	@echo "=== Running TagLib Artwork Tests ==="
	@./$(BUILD_DIR)/test_taglib_artwork

.PHONY: test-libgpod
test-libgpod: $(BUILD_DIR)/test_libgpod_artwork fixtures
	@echo "=== Running libgpod Integration Tests ==="
	@if [ -d "/media/ipod" ] && [ -f "fixtures/aaron_artwork.jpg" ]; then \
		./$(BUILD_DIR)/test_libgpod_artwork /media/ipod fixtures/aaron_artwork.jpg; \
	else \
		echo "Skipping libgpod test: iPod not mounted or test fixtures missing"; \
		echo "Run 'make fixtures' first and ensure iPod is mounted at /media/ipod"; \
	fi

.PHONY: test-covers
test-covers: $(BUILD_DIR)/test_libgpod_covers fixtures
	@echo "=== Running libgpod Cover Assignment Tests ==="
	@if [ -d "/media/ipod" ] && [ -d "fixtures" ]; then \
		echo "Testing with thumbnail processing:"; \
		./$(BUILD_DIR)/test_libgpod_covers /media/ipod fixtures; \
		echo ""; \
		echo "Testing without thumbnail processing (faster):"; \
		./$(BUILD_DIR)/test_libgpod_covers /media/ipod fixtures --skip-thumbnails; \
	else \
		echo "Skipping covers test: iPod not mounted or fixtures missing"; \
	fi

.PHONY: test-performance
test-performance: $(BUILD_DIR)/test_artwork_performance fixtures
	@echo "=== Running Artwork Performance Tests ==="
	@if [ -f "/home/mowmow/mp3/Aaron - U-turn (Lili).mp3" ]; then \
		if [ -d "/media/ipod" ] && [ -f "fixtures/aaron_artwork.jpg" ]; then \
			echo "Full performance test (with libgpod):"; \
			./$(BUILD_DIR)/test_artwork_performance "/home/mowmow/mp3/Aaron - U-turn (Lili).mp3" /media/ipod fixtures/aaron_artwork.jpg 5; \
		else \
			echo "Basic performance test (extraction only):"; \
			./$(BUILD_DIR)/test_artwork_performance "/home/mowmow/mp3/Aaron - U-turn (Lili).mp3"; \
		fi \
	else \
		echo "Skipping performance test: test audio file not found"; \
	fi

# Run all unit tests
.PHONY: test-unit
test-unit: test-metadata test-artwork
	@echo "=== All Unit Tests Completed ==="

# Run all tests
.PHONY: test
test: test-unit test-libgpod test-covers
	@echo "=== All Tests Completed ==="

.PHONY: test-full
test-full: test-unit test-libgpod test-covers test-performance
	@echo "=== Full Test Suite Completed ===" 

# Create test fixtures
.PHONY: fixtures
fixtures:
	@echo "=== Creating Test Fixtures ==="
	@./scripts/create_test_artwork.sh

# Clean test builds
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
	@echo "Test build artifacts cleaned"

# Clean fixtures
.PHONY: clean-fixtures
clean-fixtures:
	rm -rf fixtures/*.jpg
	@echo "Test fixtures cleaned"

# Clean everything
.PHONY: clean-all
clean-all: clean clean-fixtures
	@echo "All test artifacts cleaned"

# Help target
.PHONY: help
help:
	@echo "Available test targets:"
	@echo "  all              - Build all tests"
	@echo "  test             - Run all tests"
	@echo "  test-unit        - Run unit tests only"
	@echo "  test-metadata    - Test TagLib metadata extraction"
	@echo "  test-artwork     - Test TagLib artwork extraction"
	@echo "  test-libgpod     - Test libgpod artwork integration"
	@echo "  test-covers      - Test libgpod cover assignment (with --skip-thumbnails)"
	@echo "  test-performance - Test artwork extraction and assignment performance"
	@echo "  test-full        - Run complete test suite including performance"
	@echo "  fixtures         - Create test fixtures"
	@echo "  clean            - Clean build artifacts"
	@echo "  clean-fixtures   - Clean test fixtures"
	@echo "  clean-all        - Clean everything"
	@echo ""
	@echo "Requirements for libgpod tests:"
	@echo "  - iPod mounted at /media/ipod"
	@echo "  - Test fixtures created (run 'make fixtures')"

# Show test environment info
.PHONY: info
info:
	@echo "Test Environment Information:"
	@echo "  CC: $(CC)"
	@echo "  CXX: $(CXX)"
	@echo "  CFLAGS: $(CFLAGS)"
	@echo "  CXXFLAGS: $(CXXFLAGS)"
	@echo "  LDFLAGS: $(LDFLAGS)"
	@echo "  PKG_CONFIG packages: $(PACKAGES)"
	@echo ""
	@echo "iPod mount status:"
	@if [ -d "/media/ipod" ]; then \
		echo "  ✓ /media/ipod exists"; \
		if [ -f "/media/ipod/iPod_Control/iTunes/iTunesDB" ]; then \
			echo "  ✓ iTunesDB found"; \
		else \
			echo "  ✗ iTunesDB not found"; \
		fi \
	else \
		echo "  ✗ /media/ipod not found"; \
	fi
	@echo ""
	@echo "Test fixtures:"
	@if [ -d "fixtures" ]; then \
		ls -la fixtures/*.jpg 2>/dev/null || echo "  No artwork fixtures found"; \
	else \
		echo "  fixtures/ directory not found"; \
	fi