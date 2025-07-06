#!/bin/bash

# Test Script for Podcast Synchronization Fix
# ==========================================

# Configuration
IPOD_SYNC="./build/rhythmbox-ipod-sync"
MOUNT_POINT="/media/ipod"  # Adjust as needed

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}=== Test de Synchronisation des Podcasts ===${NC}"
echo

# Check if mount point exists
if [ ! -d "$MOUNT_POINT" ]; then
    echo -e "${RED}❌ Point de montage $MOUNT_POINT n'existe pas${NC}"
    echo "Veuillez :"
    echo "1. Connecter votre iPod"
    echo "2. Le monter avec: $IPOD_SYNC mount /dev/sdX $MOUNT_POINT"
    echo "3. Ajuster MOUNT_POINT dans ce script si nécessaire"
    exit 1
fi

# Check if iPod is mounted
if ! $IPOD_SYNC info "$MOUNT_POINT" >/dev/null 2>&1; then
    echo -e "${RED}❌ iPod non détecté au point de montage $MOUNT_POINT${NC}"
    echo "Veuillez monter votre iPod d'abord"
    exit 1
fi

echo -e "${GREEN}✅ iPod détecté au point de montage $MOUNT_POINT${NC}"
echo

# Show current state
echo -e "${YELLOW}📊 État actuel de l'iPod :${NC}"
$IPOD_SYNC list "$MOUNT_POINT"
echo

# Test 1: Sync a single podcast file (if available)
echo -e "${YELLOW}🎧 Test 1: Synchronisation d'un fichier podcast${NC}"

# Create a test podcast file if none exists
TEST_FILE="/tmp/test_podcast.mp3"
if [ ! -f "$TEST_FILE" ]; then
    echo "Création d'un fichier de test..."
    # Create a minimal valid MP3 file (silent)
    cat > "$TEST_FILE.tmp" << 'EOF'
ID3    TCON     PodcastTIT2     Test PodcastTPE1     Test ArtistTALB     Test AlbumTRCK     1
EOF
    # Note: This creates a minimal file, not a real MP3. 
    # For real testing, use an actual podcast file.
    mv "$TEST_FILE.tmp" "$TEST_FILE"
fi

# Sync the podcast file
echo "Synchronisation du podcast de test..."
if $IPOD_SYNC sync-file "$MOUNT_POINT" "$TEST_FILE" --mediatype podcast; then
    echo -e "${GREEN}✅ Synchronisation réussie${NC}"
else
    echo -e "${RED}❌ Échec de la synchronisation${NC}"
fi
echo

# Test 2: Check playlists after sync
echo -e "${YELLOW}📋 Test 2: Vérification des playlists${NC}"
echo "Recherche de la playlist Podcasts..."

PLAYLIST_OUTPUT=$($IPOD_SYNC list "$MOUNT_POINT")
echo "$PLAYLIST_OUTPUT"

if echo "$PLAYLIST_OUTPUT" | grep -q "Podcasts.*tracks.*(Podcasts)"; then
    echo -e "${GREEN}✅ Playlist Podcasts détectée avec des tracks${NC}"
else
    echo -e "${RED}❌ Playlist Podcasts non trouvée ou vide${NC}"
    echo "Cela indique que le fix des playlists ne fonctionne pas correctement"
fi
echo

# Test 3: Check media type distribution
echo -e "${YELLOW}🎵 Test 3: Vérification des types de média${NC}"
MEDIA_TYPES=$(echo "$PLAYLIST_OUTPUT" | grep -A 10 "TRACKS BY MEDIA TYPE")
echo "$MEDIA_TYPES"

PODCAST_COUNT=$(echo "$MEDIA_TYPES" | grep "Podcasts:" | awk '{print $2}')
if [ "$PODCAST_COUNT" -gt 0 ] 2>/dev/null; then
    echo -e "${GREEN}✅ $PODCAST_COUNT podcast(s) détecté(s) dans la base${NC}"
else
    echo -e "${RED}❌ Aucun podcast détecté dans la base${NC}"
fi
echo

# Test 4: Test folder sync
echo -e "${YELLOW}📁 Test 4: Synchronisation d'un dossier de podcasts${NC}"

# Create test folder structure
TEST_DIR="/tmp/test_podcasts"
mkdir -p "$TEST_DIR"

# Create multiple test files
for i in {1..3}; do
    cp "$TEST_FILE" "$TEST_DIR/podcast_episode_$i.mp3"
done

echo "Synchronisation du dossier de test ($TEST_DIR)..."
if $IPOD_SYNC sync-folder-filtered "$MOUNT_POINT" "$TEST_DIR" podcast; then
    echo -e "${GREEN}✅ Synchronisation du dossier réussie${NC}"
else
    echo -e "${RED}❌ Échec de la synchronisation du dossier${NC}"
fi
echo

# Final state check
echo -e "${YELLOW}📊 État final de l'iPod :${NC}"
$IPOD_SYNC list "$MOUNT_POINT"
echo

# Summary
echo -e "${BLUE}=== Résumé du Test ===${NC}"
FINAL_OUTPUT=$($IPOD_SYNC list "$MOUNT_POINT")
FINAL_PODCAST_COUNT=$(echo "$FINAL_OUTPUT" | grep "Podcasts:" | awk '{print $2}')

if [ "$FINAL_PODCAST_COUNT" -gt 0 ] 2>/dev/null; then
    echo -e "${GREEN}🎉 SUCCÈS: $FINAL_PODCAST_COUNT podcast(s) synchronisé(s) et correctement catégorisé(s)${NC}"
    echo -e "${GREEN}   Les podcasts devraient maintenant apparaître dans le menu Podcasts de l'iPod${NC}"
else
    echo -e "${RED}❌ ÉCHEC: Aucun podcast détecté après synchronisation${NC}"
    echo -e "${RED}   Le problème de playlist n'est pas résolu${NC}"
fi

# Cleanup
rm -rf "$TEST_DIR" "$TEST_FILE"

echo
echo -e "${BLUE}Pour diagnostiquer les problèmes :${NC}"
echo "1. Vérifiez les logs : cat ipod_sync.log | grep -i podcast"
echo "2. Consultez docs/PODCAST_FIX.md pour plus d'informations"
echo "3. Redémarrez votre iPod pour rafraîchir la base de données"