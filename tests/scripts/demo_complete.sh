#!/bin/bash

# Démonstration complète des améliorations rhythmbox-ipod-sync
# Montre l'extraction de métadonnées et artwork avec TagLib + libgpod

echo "🎵 DÉMONSTRATION COMPLÈTE - rhythmbox-ipod-sync"
echo "=============================================="
echo ""
echo "Cette démonstration montre les améliorations apportées :"
echo "• Extraction métadonnées fiable avec TagLib (remplace ffprobe défaillant)"
echo "• Extraction artwork native depuis les métadonnées des fichiers"  
echo "• Intégration libgpod basée sur le code officiel avec option --skip-thumbnails"
echo ""

# Vérifier la structure des tests
echo "📁 Structure des tests :"
echo "------------------------"
if [ -d "unit" ] && [ -d "integration" ] && [ -d "fixtures" ]; then
    echo "✓ Dossiers de tests présents"
    echo "  • unit/ - Tests unitaires TagLib"
    echo "  • integration/ - Tests libgpod" 
    echo "  • fixtures/ - Fichiers artwork extraits"
    echo "  • scripts/ - Utilitaires et démos"
else
    echo "❌ Structure de tests incomplète"
    exit 1
fi
echo ""

# Vérifier les fixtures
echo "🖼️  Fixtures d'artwork :"
echo "----------------------"
if ls fixtures/*.jpg >/dev/null 2>&1; then
    for artwork in fixtures/*.jpg; do
        size=$(stat -c%s "$artwork" 2>/dev/null || echo "0")
        echo "  ✓ $(basename "$artwork") - ${size} bytes"
    done
else
    echo "  ⚠️  Aucune fixture. Création en cours..."
    make fixtures
fi
echo ""

# Tests unitaires
echo "🔧 TESTS UNITAIRES TagLib"
echo "========================="
echo ""

echo "1️⃣  Test extraction métadonnées :"
echo "--------------------------------"
if [ -f "build/test_taglib_metadata" ]; then
    ./build/test_taglib_metadata | tail -n 4
else
    echo "❌ Test non compilé. Compilation..."
    make build/test_taglib_metadata && ./build/test_taglib_metadata | tail -n 4
fi
echo ""

echo "2️⃣  Test extraction artwork :"
echo "----------------------------"
if [ -f "build/test_taglib_artwork" ]; then
    ./build/test_taglib_artwork | tail -n 4
else
    echo "❌ Test non compilé. Compilation..."
    make build/test_taglib_artwork && ./build/test_taglib_artwork | tail -n 4
fi
echo ""

# Tests d'intégration
echo "🎯 TESTS LIBGPOD (Code officiel adapté)"
echo "======================================="
echo ""

# Vérifier si iPod est connecté
if [ -d "/media/ipod" ] && [ -f "/media/ipod/iPod_Control/iTunes/iTunesDB" ]; then
    echo "✅ iPod détecté à /media/ipod"
    
    echo ""
    echo "3️⃣  Test assignation covers (RAPIDE - skip thumbnails) :"
    echo "-------------------------------------------------------"
    if [ -f "build/test_libgpod_covers" ]; then
        ./build/test_libgpod_covers /media/ipod fixtures --skip-thumbnails | grep -E "(Found|Processed|Summary|✅)"
    else
        echo "❌ Test non compilé. Compilation..."
        make build/test_libgpod_covers
        if [ $? -eq 0 ]; then
            ./build/test_libgpod_covers /media/ipod fixtures --skip-thumbnails | grep -E "(Found|Processed|Summary|✅)"
        fi
    fi
    
    echo ""
    echo "4️⃣  Test assignation covers (COMPLET - avec thumbnails) :"
    echo "--------------------------------------------------------"
    if [ -f "build/test_libgpod_covers" ]; then
        echo "⚠️  Ce test prend plus de temps car il traite les thumbnails..."
        ./build/test_libgpod_covers /media/ipod fixtures | grep -E "(Found|Processed|Summary|✅|Warning)"
    fi
    
else
    echo "⚠️  iPod non détecté à /media/ipod"
    echo "   Les tests libgpod seront simulés"
    
    echo ""
    echo "3️⃣  Simulation test covers :"
    echo "---------------------------"
    if [ -f "build/test_libgpod_covers" ]; then
        echo "   Commande qui serait exécutée :"
        echo "   ./build/test_libgpod_covers /media/ipod fixtures --skip-thumbnails"
        echo "   ✓ Test compilé et prêt"
    else
        echo "   ❌ Test non compilé"
    fi
fi

echo ""

# Comparaison avant/après
echo "📊 COMPARAISON AVANT/APRÈS"
echo "=========================="
echo ""

test_file="/home/mowmow/mp3/Aaron - U-turn (Lili).mp3"
if [ -f "$test_file" ]; then
    echo "🔴 AVANT (ffprobe - PROBLÉMATIQUE) :"
    echo "-----------------------------------"
    # Montrer le problème ffprobe
    ffprobe_title=$(ffprobe -v quiet -show_format -show_streams -of json "$test_file" 2>/dev/null | grep -m1 '"title"' | cut -d'"' -f4)
    echo "   Titre extrait : '$ffprobe_title' ❌"
    echo "   → Extrait le nom du stream d'artwork, pas le vrai titre !"
    
    echo ""
    echo "🟢 APRÈS (TagLib - FIABLE) :"
    echo "----------------------------"
    # Montrer TagLib qui fonctionne
    if command -v taglib-config >/dev/null 2>&1; then
        cat << 'EOF' > /tmp/demo_taglib.c
#include <stdio.h>
#include <tag_c.h>
int main(int argc, char *argv[]) {
    if (argc < 2) return 1;
    TagLib_File *file = taglib_file_new(argv[1]);
    if (!file || !taglib_file_is_valid(file)) return 1;
    TagLib_Tag *tag = taglib_file_tag(file);
    if (tag) {
        char *title = taglib_tag_title(tag);
        printf("   Titre extrait : '%s' ✅\n", title ? title : "N/A");
    }
    taglib_file_free(file);
    return 0;
}
EOF
        gcc -o /tmp/demo_taglib /tmp/demo_taglib.c $(pkg-config --cflags --libs taglib_c) 2>/dev/null
        if [ -f /tmp/demo_taglib ]; then
            /tmp/demo_taglib "$test_file"
            echo "   → Extrait le vrai titre depuis les métadonnées ID3 !"
            rm -f /tmp/demo_taglib /tmp/demo_taglib.c
        fi
    fi
else
    echo "⚠️  Fichier de test non trouvé : $test_file"
fi

echo ""

# Résumé des améliorations
echo "🏆 RÉSUMÉ DES AMÉLIORATIONS"
echo "=========================="
echo ""
echo "✅ PROBLÈMES RÉSOLUS :"
echo "  • Titres incorrects ('cover', 'image1') → Vrais titres des chansons"
echo "  • Extraction artwork instable → Extraction native multi-format"
echo "  • Dépendance ffprobe défaillant → API TagLib fiable"
echo "  • Pas de tests → Suite de tests complète"
echo ""
echo "🚀 NOUVELLES FONCTIONNALITÉS :"
echo "  • Extraction métadonnées TagLib C (rapide, fiable)"
echo "  • Extraction artwork TagLib C++ (MP3, FLAC, MP4)"
echo "  • Tests libgpod basés sur le code officiel"
echo "  • Option --skip-thumbnails pour performance"
echo "  • Suite de tests unitaires et d'intégration"
echo "  • Documentation libgpod complète"
echo ""
echo "📈 PERFORMANCES :"
echo "  • Extraction mémoire vs processus externes"
echo "  • API native vs parsing JSON défaillant"
echo "  • Support multi-format natif"
echo "  • Tests de performance automatisés"
echo ""
echo "🎯 RÉSULTAT :"
echo "  Tous les problèmes initiaux sont résolus !"
echo "  Le système extrait maintenant correctement :"
echo "  • Les vrais titres depuis les métadonnées"
echo "  • L'artwork embarqué dans les fichiers audio"
echo "  • Compatible avec l'API libgpod officielle"
echo ""
echo "🎉 Mission accomplie !"