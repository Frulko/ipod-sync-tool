#!/bin/bash

# Script pour créer des fichiers d'artwork de test
# Utilise ffmpeg pour extraire l'artwork depuis les fichiers audio existants

TEST_DIR="/home/mowmow/rhythmbox-ipod-sync-project/tests/fixtures"
MP3_DIR="/home/mowmow/mp3"

echo "Creating test artwork files..."
mkdir -p "$TEST_DIR"

# Extraire artwork depuis Aaron
if [ -f "$MP3_DIR/Aaron - U-turn (Lili).mp3" ]; then
    echo "Extracting artwork from Aaron file..."
    ffmpeg -i "$MP3_DIR/Aaron - U-turn (Lili).mp3" -map 0:v:0 -c:v mjpeg -q:v 2 "$TEST_DIR/aaron_artwork.jpg" -y 2>/dev/null
    if [ $? -eq 0 ]; then
        echo "✓ Created: $TEST_DIR/aaron_artwork.jpg"
        ls -la "$TEST_DIR/aaron_artwork.jpg"
    else
        echo "✗ Failed to extract Aaron artwork"
    fi
fi

# Extraire artwork depuis Daft Punk
if [ -f "$MP3_DIR/Daft Punk - Harder, Better, Faster, Stronger.mp3" ]; then
    echo "Extracting artwork from Daft Punk file..."
    ffmpeg -i "$MP3_DIR/Daft Punk - Harder, Better, Faster, Stronger.mp3" -map 0:v:0 -c:v mjpeg -q:v 2 "$TEST_DIR/daftpunk_artwork.jpg" -y 2>/dev/null
    if [ $? -eq 0 ]; then
        echo "✓ Created: $TEST_DIR/daftpunk_artwork.jpg"
        ls -la "$TEST_DIR/daftpunk_artwork.jpg"
    else
        echo "✗ Failed to extract Daft Punk artwork"
    fi
fi

# Créer un artwork de test générique (carré coloré)
echo "Creating generic test artwork..."
convert -size 300x300 xc:blue -fill white -gravity center -pointsize 24 -annotate +0+0 "TEST\nARTWORK" "$TEST_DIR/generic_artwork.jpg" 2>/dev/null
if [ $? -eq 0 ]; then
    echo "✓ Created: $TEST_DIR/generic_artwork.jpg"
    ls -la "$TEST_DIR/generic_artwork.jpg"
else
    echo "⚠ ImageMagick not available, using solid color"
    # Alternative avec ffmpeg
    ffmpeg -f lavfi -i color=c=blue:size=300x300:d=1 -vframes 1 "$TEST_DIR/generic_artwork.jpg" -y 2>/dev/null
    if [ $? -eq 0 ]; then
        echo "✓ Created: $TEST_DIR/generic_artwork.jpg (solid color)"
        ls -la "$TEST_DIR/generic_artwork.jpg"
    fi
fi

echo ""
echo "Test artwork files created in: $TEST_DIR"
echo "Available files:"
ls -la "$TEST_DIR"/*.jpg 2>/dev/null || echo "No artwork files created"