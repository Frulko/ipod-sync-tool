# 🎵 rhythmbox-ipod-sync - Synchronisation iPod Avancée

Outil de synchronisation iPod moderne avec support complet des podcasts, audiobooks et métadonnées étendues. Basé sur l'architecture Rhythmbox avec des améliorations pour les médias modernes.

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

## 🚀 Fonctionnalités Principales

### ✨ Synchronisation Avancée
- **📁 Sync par dossier filtré** : Synchronise des dossiers entiers avec type de média forcé
- **📄 Sync fichier unique** : Synchronise des fichiers individuels avec type spécifique
- **🎙️ Support podcast complet** : Métadonnées étendues (saison, épisode, date, description)
- **📚 Support audiobook** : Chapitres, progression, signets
- **🎬 Support vidéo** : Films, clips musicaux, épisodes TV

### 🎯 Fonctionnalités Métadonnées Podcasts
- **Extraction automatique** : DATE, GROUPING, CATEGORY, SUBTITLE
- **URLs podcast** : PODCASTURL, PODCASTRSS, liens épisodes
- **Informations épisode** : Numéro saison/épisode, résumé complet
- **Conversion artwork** : Conversion automatique JPEG pour compatibilité iPod
- **Template mp3tag** : Support complet des templates personnalisés

### 🔧 Fonctionnalités Techniques
- **Gestion robuste database** : Sauvegardes automatiques, récupération d'erreur
- **Operations asynchrones** : File d'attente pour opérations database
- **Détection automatique** : Montage/démontage iPod intelligent
- **Multi-format** : Support FAT32, HFS+, exFAT
- **Logging complet** : Traçabilité détaillée des opérations

## 📦 Installation et Compilation

### 🔧 Dépendances Ubuntu/Debian
```bash
sudo apt-get install libgpod-dev libglib2.0-dev libgdk-pixbuf2.0-dev 
sudo apt-get install libtag1-dev udisks2 util-linux
```

### 🔧 Dépendances CentOS/RHEL  
```bash
sudo yum install libgpod-devel glib2-devel gdk-pixbuf2-devel
sudo yum install taglib-devel udisks2 util-linux
```

### 🛠️ Compilation
```bash
# Vérifier les dépendances
make check-deps

# Compilation optimisée (recommandé)
make release

# Compilation debug (développement)
make debug

# Informations build
make info
```

## 🎯 Guide d'Utilisation

### 📱 Gestion iPod

**🔌 Montage/Démontage automatique :**
```bash
# Détection et montage automatique
./build/rhythmbox-ipod-sync auto-mount

# Montage manuel
./build/rhythmbox-ipod-sync mount /dev/sdb1 /media/ipod

# Démontage sécurisé
./build/rhythmbox-ipod-sync unmount /media/ipod
```

### 🎵 Synchronisation Musique

**📁 Synchronisation dossier traditionnel :**
```bash
./build/rhythmbox-ipod-sync sync /media/ipod ~/Music
```

**📄 Synchronisation fichier unique :**
```bash
# Fichier audio standard
./build/rhythmbox-ipod-sync sync-file /media/ipod ~/chanson.mp3

# Fichier avec type spécifique
./build/rhythmbox-ipod-sync sync-file /media/ipod ~/podcast.mp3 --mediatype podcast
```

### 🎙️ Synchronisation Podcasts (Recommandé)

**📁 Dossier podcast complet :**
```bash
./build/rhythmbox-ipod-sync sync-folder-filtered /media/ipod ~/Podcasts podcast
```

**🏷️ Template mp3tag recommandé :**
Utilisez le template fourni `mp3tag_podcast_template.mta` pour automatiser le tagging :

| **Champ mp3tag** | **Fonction** | **Résultat iPod** |
|------------------|--------------|-------------------|
| `TRACK = 25` | Numéro épisode | Episode 25 |
| `DISCNUMBER = 3` | Numéro saison | Saison 3 |
| `DATE = 2024-07-06` | Date sortie | 06/07/2024 |
| `GROUPING = PODCAST-S3E25` | ID épisode | Identifiant unique |
| `CATEGORY = Technology` | Catégorie | Genre podcast |
| `PODCAST = Mon Podcast` | Nom show | Titre série |

### 📚 Autres Types de Média

**📖 Audiobooks :**
```bash
./build/rhythmbox-ipod-sync sync-folder-filtered /media/ipod ~/Audiobooks audiobook
```

**🎬 Vidéos musicales :**
```bash
./build/rhythmbox-ipod-sync sync-folder-filtered /media/ipod ~/MusicVideos musicvideo
```

**📺 Épisodes TV :**
```bash
./build/rhythmbox-ipod-sync sync-folder-filtered /media/ipod ~/TV tvshow
```

### 🔍 Commandes d'Information

**📋 Lister les pistes :**
```bash
./build/rhythmbox-ipod-sync list /media/ipod
```

**ℹ️ Informations iPod :**
```bash
./build/rhythmbox-ipod-sync info /media/ipod
```

**🗑️ Nettoyage :**
```bash
# Supprimer tous les podcasts
./build/rhythmbox-ipod-sync reset /media/ipod podcast

# Nettoyer complètement l'iPod
./build/rhythmbox-ipod-sync reset /media/ipod all
```

### 📱 Types de Média Supportés

| **Type** | **Utilisation** | **Emplacement iPod** |
|----------|-----------------|---------------------|
| `audio` | Musique (défaut) | Music |
| `podcast` | Épisodes podcast | Podcasts |
| `audiobook` | Livres audio | Audiobooks |
| `musicvideo` | Clips musicaux | Music → Videos |
| `movie` | Films | Videos |
| `tvshow` | Séries TV | TV Shows |
| `ringtone` | Sonneries | Settings |
| `itunes-u` | Contenu éducatif | iTunes U |

## 📋 Préparation des Fichiers

### 🎙️ Podcasts - Workflow Optimal

#### **Étape 1 : Préparation mp3tag**

1. **Installer le template** :
   - Importer `mp3tag_podcast_template.mta` dans mp3tag
   - Configurer les colonnes selon `mp3tag_podcast_guide.md`

2. **Métadonnées minimales requises** :
```ini
TITLE = Titre de l'épisode
ARTIST = Nom du podcast  
ALBUM = Nom du podcast
ALBUMARTIST = Réseau/producteur
TRACK = 25                    # Numéro épisode
DISCNUMBER = 3               # Numéro saison
COMMENT = Description courte
```

3. **Appliquer l'action "Podcast Complete Setup"** :
   - Sélectionner les fichiers
   - Clic droit → Actions → Podcast Complete Setup
   - ✅ Métadonnées automatiquement complétées

#### **Étape 2 : Vérification**

**Champs automatiquement générés** :
```ini
GENRE = Podcast
GROUPING = PODCAST-NAME-S3E25    # ID unique
CATEGORY = Technology             # Catégorie
DATE = 2024-07-06                # Date sortie
PODCAST = Nom du podcast
PODCASTURL = URL épisode
PODCASTRSS = URL flux RSS
MEDIATYPE = Podcast
DESCRIPTION = Résumé complet
```

#### **Étape 3 : Organisation des Fichiers**

**Structure recommandée** :
```
~/Podcasts/
├── Tech Podcast FR/
│   ├── Saison 1/
│   │   ├── 01 - Titre Episode.mp3
│   │   └── 02 - Titre Episode.mp3
│   └── Saison 2/
│       └── 01 - Titre Episode.mp3
├── Science Show/
│   └── Episodes/
│       ├── S01E01 - Titre.mp3
│       └── S01E02 - Titre.mp3
└── artwork/
    ├── tech-podcast-cover.jpg
    └── science-show-cover.jpg
```

### 📚 Audiobooks - Préparation

#### **Métadonnées essentielles** :
```ini
TITLE = Chapitre X - Titre du chapitre
ARTIST = Auteur du livre
ALBUM = Titre du livre
ALBUMARTIST = Narrateur
TRACK = 1, 2, 3...              # Ordre chapitres
DISCNUMBER = 1                  # Partie/Volume
GENRE = Audiobook
```

#### **Organisation fichiers** :
```
~/Audiobooks/
├── Auteur - Titre Livre/
│   ├── 01 - Chapitre 1.mp3
│   ├── 02 - Chapitre 2.mp3
│   └── cover.jpg
└── Série/
    ├── Tome 1/
    │   ├── Partie1-Ch01.mp3
    │   └── Partie1-Ch02.mp3
    └── Tome 2/
        └── ...
```

### 🎵 Musique - Bonnes Pratiques

#### **Métadonnées standard** :
```ini
TITLE = Titre de la chanson
ARTIST = Artiste principal
ALBUM = Nom de l'album
ALBUMARTIST = Artiste de l'album
TRACK = 5                       # Numéro piste
TRACKTOTAL = 12                # Total pistes album
DISCNUMBER = 1                 # Numéro disque
DISCTOTAL = 2                  # Total disques
GENRE = Rock, Pop, Electronic...
YEAR = 2024
```

### 🖼️ Artwork - Spécifications

#### **Formats supportés** :
- **Recommandé** : JPEG (conversion automatique)
- **Accepté** : PNG (converti en JPEG)
- **Taille optimale** : 300x300 à 1400x1400 pixels
- **Qualité** : 85% minimum pour JPEG

#### **Intégration automatique** :
- ✅ Extraction depuis ID3v2 APIC (MP3)
- ✅ Extraction depuis FLAC metadata blocks
- ✅ Extraction depuis MP4/M4A cover art
- ✅ Conversion automatique au format iPod

### 🔧 Scripts de Préparation

#### **Script de vérification métadonnées** :
```bash
#!/bin/bash
# Vérifier que tous les fichiers ont les métadonnées requises
find ~/Podcasts -name "*.mp3" -exec taglib-config --print-tags {} \;
```

#### **Script de nommage automatique** :
```bash
#!/bin/bash
# Renommer selon le pattern iPod
# Pattern: "Artiste - SXeXX - Titre.mp3"
for file in *.mp3; do
    # Extraction metadata et renommage
    mv "$file" "$(get-metadata-pattern "$file")"
done
```

### ⚡ Workflow Complet Recommandé

1. **📁 Organisation** : Créer structure dossiers par type
2. **🏷️ Tagging** : Utiliser mp3tag avec templates fournis  
3. **🖼️ Artwork** : Ajouter covers dans mp3tag
4. **✅ Vérification** : Contrôler métadonnées avec scripts
5. **🚀 Synchronisation** : Utiliser `sync-folder-filtered`

```bash
# Workflow podcast type
./build/rhythmbox-ipod-sync sync-folder-filtered /media/ipod ~/Podcasts podcast

# Workflow audiobook type  
./build/rhythmbox-ipod-sync sync-folder-filtered /media/ipod ~/Audiobooks audiobook

# Vérification résultat
./build/rhythmbox-ipod-sync list /media/ipod
```

## 🔧 Architecture Technique

### 🏗️ Organisation Modulaire

**Modules principaux** :
- **🗂️ Types & Config** : Structures données et constantes
- **📝 Logging** : Système de logs thread-safe
- **💾 Database** : Gestion base données iPod (libgpod)
- **🔄 Actions** : File d'attente opérations asynchrones  
- **🏷️ Metadata** : Extraction métadonnées (TagLib C++)
- **📁 Filesystem** : Montage/démontage et détection
- **📄 Files** : Opérations fichiers et création pistes
- **🔄 Sync** : Logique de synchronisation
- **⌨️ Commands** : Implémentation commandes CLI

### 🔑 Fonctionnalités Clés

#### **Gestion Base Données iPod**
- ✅ **Sauvegardes automatiques** avant modifications
- ✅ **File d'attente opérations** pendant sauvegardes database
- ✅ **Validation complète** avant/après opérations
- ✅ **Récupération automatique** après interruptions

#### **Extraction Métadonnées Avancée**
- ✅ **TagLib C++** : Support MP3, FLAC, MP4/M4A
- ✅ **Artwork natif** : Extraction depuis tags (APIC, Picture blocks)
- ✅ **Conversion JPEG** : Automatique via GdkPixbuf
- ✅ **Métadonnées podcast** : 8 champs étendus (DATE, GROUPING, etc.)

#### **Sécurité et Robustesse**
- ✅ **Détection filesystem** : FAT32, HFS+, exFAT automatique
- ✅ **Permissions utilisateur** : Pas de root requis
- ✅ **Gestion interruptions** : Signal handlers et cleanup
- ✅ **Validation écriture** : Vérification permissions avant sync

### 🎯 Fonctionnalités Implémentées

#### ✅ **100% Fonctionnel**
- **Synchronisation** : sync, sync-file, sync-folder-filtered
- **Gestion iPod** : mount, unmount, auto-mount
- **Base données** : Lecture, écriture, sauvegarde, récupération
- **Métadonnées** : Extraction complète MP3/FLAC/MP4 + podcasts
- **Artwork** : Extraction native + conversion JPEG automatique
- **Types média** : Support complet (audio, podcast, audiobook, video...)
- **CLI** : Toutes commandes opérationnelles

#### 🎙️ **Spécialisations Podcasts**
- **Template mp3tag** : Automatisation tagging complet
- **Guide utilisateur** : Documentation détaillée workflow
- **Métadonnées étendues** : 8 champs podcast extraits depuis ID3v2
- **Playlist automatique** : Apparition correcte dans menu Podcasts iPod

### 🛠️ Compilation et Tests

```bash
# Build optimisé
make release

# Tests compilation
make test-compile  

# Installation système
sudo make install

# Vérification dépendances
make check-deps
```

### 📊 Performance

**Optimisations clés** :
- **Compilation conditionnelle** : Modules séparés, recompilation partielle
- **Database queuing** : Opérations asynchrones, pas de blocage
- **Memory management** : Libération automatique ressources
- **Error recovery** : Rollback automatique en cas d'erreur

## 🚀 Statut Projet

**🎯 Prêt pour production** :
- Architecture robuste et testée
- Support complet métadonnées podcasts
- Documentation utilisateur complète  
- Templates mp3tag fournis
- Workflow optimisé pour tous types médias

**💡 Utilisation recommandée** :
```bash
# Installation
make release && sudo make install

# Workflow podcast type
rhythmbox-ipod-sync sync-folder-filtered /media/ipod ~/Podcasts podcast

# Vérification
rhythmbox-ipod-sync list /media/ipod
```

## 📜 Licence

Basé sur l'architecture de gestion iPod de Rhythmbox avec extensions modernes pour podcasts et métadonnées avancées.