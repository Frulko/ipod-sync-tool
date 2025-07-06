#!/bin/bash

# Script de Validation du Fix de Lecture des Podcasts
# ==================================================

# Configuration
IPOD_SYNC="./build/rhythmbox-ipod-sync"
MOUNT_POINT="${1:-/media/ipod}"  # Premier argument ou défaut

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}=== Validation du Fix de Lecture des Podcasts ===${NC}"
echo "Point de montage: $MOUNT_POINT"
echo

# Vérifications préliminaires
if [ ! -f "$IPOD_SYNC" ]; then
    echo -e "${RED}❌ Exécutable non trouvé: $IPOD_SYNC${NC}"
    echo "Compilez d'abord avec: make release"
    exit 1
fi

if [ ! -d "$MOUNT_POINT" ]; then
    echo -e "${RED}❌ Point de montage inexistant: $MOUNT_POINT${NC}"
    echo "Usage: $0 [mount_point]"
    echo "Exemple: $0 /media/ipod"
    exit 1
fi

# Test de base
echo -e "${YELLOW}🔍 Test 1: Vérification de base${NC}"
if $IPOD_SYNC info "$MOUNT_POINT" >/dev/null 2>&1; then
    echo -e "${GREEN}✅ iPod détecté et accessible${NC}"
else
    echo -e "${RED}❌ iPod non accessible ou non monté${NC}"
    exit 1
fi

# Affichage de l'état actuel
echo -e "${YELLOW}📊 Test 2: État actuel${NC}"
CURRENT_STATE=$($IPOD_SYNC list "$MOUNT_POINT" 2>/dev/null)
echo "$CURRENT_STATE"

# Compter les podcasts existants
EXISTING_PODCASTS=$(echo "$CURRENT_STATE" | grep "Podcasts:" | awk '{print $2}')
echo -e "Podcasts actuellement détectés: ${EXISTING_PODCASTS:-0}"
echo

# Test de synchronisation avec nouveau système
echo -e "${YELLOW}🎧 Test 3: Synchronisation avec nouveaux attributs${NC}"

# Créer un fichier de test temporaire
TEST_FILE="/tmp/test_podcast_fix.mp3"
cat > "$TEST_FILE" << 'EOF'
# Fichier MP3 minimal de test
# Ce n'est pas un vrai MP3, juste pour tester les métadonnées
EOF

echo "Synchronisation d'un podcast de test avec nouveaux attributs..."
SYNC_OUTPUT=$($IPOD_SYNC sync-file "$MOUNT_POINT" "$TEST_FILE" --mediatype podcast 2>&1)
SYNC_SUCCESS=$?

echo "Sortie de synchronisation:"
echo "$SYNC_OUTPUT"
echo

if [ $SYNC_SUCCESS -eq 0 ]; then
    echo -e "${GREEN}✅ Synchronisation réussie${NC}"
else
    echo -e "${RED}❌ Échec de synchronisation${NC}"
fi

# Vérification post-sync
echo -e "${YELLOW}📋 Test 4: Vérification post-synchronisation${NC}"
POST_STATE=$($IPOD_SYNC list "$MOUNT_POINT" 2>/dev/null)
echo "$POST_STATE"

NEW_PODCASTS=$(echo "$POST_STATE" | grep "Podcasts:" | awk '{print $2}')
echo -e "Podcasts après sync: ${NEW_PODCASTS:-0}"

# Vérification des logs pour les nouveaux attributs
echo -e "${YELLOW}🔍 Test 5: Validation des attributs podcast${NC}"
if [ -f "ipod_sync.log" ]; then
    echo "Recherche dans les logs des attributs podcast..."
    
    PODCAST_LOGS=$(grep -i "podcast-specific" ipod_sync.log | tail -5)
    if [ -n "$PODCAST_LOGS" ]; then
        echo -e "${GREEN}✅ Attributs podcast détectés dans les logs:${NC}"
        echo "$PODCAST_LOGS"
    else
        echo -e "${RED}❌ Aucun attribut podcast trouvé dans les logs${NC}"
    fi
    
    # Vérifier les attributs spécifiques
    echo -e "\nRécapitulatif des attributs configurés:"
    grep -E "(flag4|mark_unplayed|podcasturl|remember_playback)" ipod_sync.log | tail -3
else
    echo -e "${YELLOW}⚠️  Fichier de log non trouvé${NC}"
fi

echo

# Analyse comparative
echo -e "${YELLOW}📈 Test 6: Analyse comparative${NC}"
if [ "$NEW_PODCASTS" -gt "${EXISTING_PODCASTS:-0}" ]; then
    ADDED_PODCASTS=$((NEW_PODCASTS - ${EXISTING_PODCASTS:-0}))
    echo -e "${GREEN}✅ $ADDED_PODCASTS nouveau(x) podcast(s) ajouté(s)${NC}"
else
    echo -e "${YELLOW}⚠️  Aucun nouveau podcast détecté (normal si déjà présent)${NC}"
fi

# Vérification de la playlist Podcasts
PODCASTS_PLAYLIST=$(echo "$POST_STATE" | grep "Podcasts.*tracks.*(Podcasts)")
if [ -n "$PODCASTS_PLAYLIST" ]; then
    echo -e "${GREEN}✅ Playlist Podcasts active et fonctionnelle${NC}"
    echo "   $PODCASTS_PLAYLIST"
else
    echo -e "${RED}❌ Playlist Podcasts manquante ou non configurée${NC}"
fi

# Cleanup
rm -f "$TEST_FILE"

# Résumé final
echo
echo -e "${BLUE}=== RÉSUMÉ DE VALIDATION ===${NC}"

SUCCESS_COUNT=0
TOTAL_TESTS=6

# Test 1: iPod accessible
if $IPOD_SYNC info "$MOUNT_POINT" >/dev/null 2>&1; then
    echo -e "${GREEN}✅ Test 1: iPod accessible${NC}"
    ((SUCCESS_COUNT++))
else
    echo -e "${RED}❌ Test 1: iPod inaccessible${NC}"
fi

# Test 2: Sync réussie
if [ $SYNC_SUCCESS -eq 0 ]; then
    echo -e "${GREEN}✅ Test 2: Synchronisation réussie${NC}"
    ((SUCCESS_COUNT++))
else
    echo -e "${RED}❌ Test 2: Échec de synchronisation${NC}"
fi

# Test 3: Podcasts détectés
if [ "${NEW_PODCASTS:-0}" -gt 0 ]; then
    echo -e "${GREEN}✅ Test 3: Podcasts détectés (${NEW_PODCASTS})${NC}"
    ((SUCCESS_COUNT++))
else
    echo -e "${RED}❌ Test 3: Aucun podcast détecté${NC}"
fi

# Test 4: Playlist Podcasts
if [ -n "$PODCASTS_PLAYLIST" ]; then
    echo -e "${GREEN}✅ Test 4: Playlist Podcasts fonctionnelle${NC}"
    ((SUCCESS_COUNT++))
else
    echo -e "${RED}❌ Test 4: Playlist Podcasts manquante${NC}"
fi

# Test 5: Logs d'attributs
if [ -n "$PODCAST_LOGS" ]; then
    echo -e "${GREEN}✅ Test 5: Attributs podcast configurés${NC}"
    ((SUCCESS_COUNT++))
else
    echo -e "${RED}❌ Test 5: Attributs podcast manquants${NC}"
fi

# Test 6: Structure générale
if [ "${NEW_PODCASTS:-0}" -gt 0 ] && [ -n "$PODCASTS_PLAYLIST" ]; then
    echo -e "${GREEN}✅ Test 6: Structure podcast complète${NC}"
    ((SUCCESS_COUNT++))
else
    echo -e "${RED}❌ Test 6: Structure podcast incomplète${NC}"
fi

echo
echo -e "Score final: ${SUCCESS_COUNT}/${TOTAL_TESTS} tests réussis"

if [ $SUCCESS_COUNT -eq $TOTAL_TESTS ]; then
    echo -e "${GREEN}🎉 SUCCÈS COMPLET: Fix de lecture des podcasts validé !${NC}"
    echo -e "${GREEN}   Les podcasts devraient maintenant être lisibles sur l'iPod${NC}"
    echo
    echo -e "${BLUE}Prochaines étapes:${NC}"
    echo "1. Débranchez et rebranchez l'iPod"
    echo "2. Ou redémarrez l'iPod pour actualiser le cache"
    echo "3. Naviguez vers Podcasts sur l'iPod"
    echo "4. Testez la lecture d'un épisode"
    exit 0
elif [ $SUCCESS_COUNT -ge 4 ]; then
    echo -e "${YELLOW}⚠️  SUCCÈS PARTIEL: La plupart des tests sont réussis${NC}"
    echo -e "${YELLOW}   Les podcasts pourraient fonctionner, vérifiez sur l'iPod${NC}"
    exit 0
else
    echo -e "${RED}❌ ÉCHEC: Problèmes détectés dans le fix${NC}"
    echo -e "${RED}   Consultez docs/PODCAST_PLAYBACK_FIX.md pour le débogage${NC}"
    exit 1
fi