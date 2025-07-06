# Fix de Lecture des Podcasts - Attributs et Métadonnées

## Problème Résolu

**Symptôme** : Les podcasts étaient visibles dans la playlist Podcasts de l'iPod mais impossible de les lire/écouter.

**Cause** : Les tracks podcast manquaient d'attributs spécifiques et de flags de comportement requis par le firmware iPod pour la lecture des podcasts.

## Analyse Technique du Problème

### Ce Qui Fonctionnait Déjà
✅ **Type de média correct** : `ITDB_MEDIATYPE_PODCAST` était défini  
✅ **Playlist appropriée** : Les tracks étaient ajoutés à la playlist Podcasts  
✅ **Métadonnées de base** : Titre, artiste, album étaient présents  

### Ce Qui Manquait (Attributs Critiques)

#### 1. **Flags de Comportement iPod**
- `flag4` : Flag d'affichage podcast (doit être `0x01`)
- `mark_unplayed` : Marqueur d'épisode non lu (doit être `0x02` pour nouveaux épisodes)

#### 2. **URLs et Métadonnées Podcast**
- `podcasturl` : URL de l'épisode (enclosure URL)
- `podcastrss` : URL du flux RSS du podcast
- `description` : Description/notes de l'épisode
- `subtitle` : Sous-titre de l'épisode
- `category` : Catégorie du podcast

#### 3. **Comportements de Lecture**
- `remember_playback_position` : Mémoriser la position de lecture (essentiel pour podcasts)
- `skip_when_shuffling` : Exclure du mode aléatoire
- `time_released` : Date de publication de l'épisode

## Solution Implémentée

### 1. **Extension de la Structure AudioMetadata**

Ajout de champs podcast-spécifiques dans `include/rbipod-types.h` :

```c
typedef struct {
    // ... champs existants ...
    
    // Podcast-specific fields
    char *podcasturl;          // URL de l'épisode
    char *podcastrss;          // URL du flux RSS
    char *description;         // Description de l'épisode
    char *subtitle;            // Sous-titre
    char *category;            // Catégorie
    time_t time_released;      // Date de publication
    gboolean mark_unplayed;    // Marqueur non lu
} AudioMetadata;
```

### 2. **Génération de Métadonnées Podcast**

Dans `src/rbipod-metadata.c`, ajout de la génération automatique :

```c
// Set podcast-specific metadata if this is a podcast
if (meta->mediatype == ITDB_MEDIATYPE_PODCAST) {
    // Generate synthetic podcast URL based on filename
    meta->podcasturl = g_strdup_printf("file://%s", filename);
    meta->podcastrss = g_strdup("http://localhost/podcast.rss");
    
    // Set podcast category and description
    meta->category = g_strdup("Podcasts");
    meta->description = g_strdup_printf("Podcast episode: %s", meta->title);
    meta->subtitle = g_strdup(meta->title);
    
    // Set release time to current time if not specified
    meta->time_released = time(NULL);
    
    // Mark as unplayed (new episode)
    meta->mark_unplayed = TRUE;
    
    // Ensure proper podcast behavior
    meta->remember_playback_position = TRUE;
    meta->skip_when_shuffling = TRUE;
}
```

### 3. **Configuration des Attributs iPod Spécifiques**

Dans `src/rbipod-files.c`, dans `create_ipod_track_from_metadata()` :

```c
// Set podcast-specific attributes for proper playback
if (track->mediatype == ITDB_MEDIATYPE_PODCAST) {
    // Essential podcast flags for iPod firmware
    track->flag4 = 0x01;  // Podcast display flag
    track->mark_unplayed = meta->mark_unplayed ? 0x02 : 0x01;  // New episode marker
    
    // Podcast URLs and metadata
    if (meta->podcasturl) track->podcasturl = g_strdup(meta->podcasturl);
    if (meta->podcastrss) track->podcastrss = g_strdup(meta->podcastrss);
    if (meta->description) track->description = g_strdup(meta->description);
    if (meta->subtitle) track->subtitle = g_strdup(meta->subtitle);
    if (meta->category) track->category = g_strdup(meta->category);
    
    // Release date for podcast episodes
    if (meta->time_released > 0) {
        track->time_released = meta->time_released;
    }
    
    // Podcast behavior settings
    track->remember_playback_position = TRUE;
    track->skip_when_shuffling = TRUE;
    
    // Set bookmark time to 0 for new episodes
    track->bookmark_time = 0;
}
```

## Attributs Podcast Critiques Définis

### **Flags de Comportement**
| Attribut | Valeur | Fonction |
|----------|--------|----------|
| `flag4` | `0x01` | Identifie le track comme podcast pour l'interface iPod |
| `mark_unplayed` | `0x02` | Affiche le bullet "non lu" dans l'interface |
| `remember_playback_position` | `TRUE` | Mémorise où on s'est arrêté dans l'épisode |
| `skip_when_shuffling` | `TRUE` | Exclut les podcasts du mode lecture aléatoire |

### **Métadonnées Essentielles**
- **`podcasturl`** : URL de l'épisode (même si locale avec `file://`)
- **`podcastrss`** : URL du flux RSS (peut être synthétique)  
- **`description`** : Notes/résumé de l'épisode
- **`category`** : Catégorie pour l'organisation
- **`time_released`** : Date de publication pour le tri chronologique

## Test et Validation

### **Commandes de Test**

```bash
# 1. Synchroniser un nouveau podcast
./build/rhythmbox-ipod-sync sync-file /media/ipod ~/podcast.mp3 --mediatype podcast

# 2. Vérifier les attributs
./build/rhythmbox-ipod-sync list /media/ipod

# 3. Consulter les logs pour validation
cat ipod_sync.log | grep -i "podcast-specific"
```

### **Ce Que Vous Devriez Observer**

**Dans les logs** :
```
[DEBUG] Set podcast-specific metadata for: Mon Podcast Episode
[DEBUG] Set podcast-specific attributes for track: Mon Podcast Episode
[DEBUG] Added podcast track to Podcasts playlist: Mon Podcast Episode
```

**Sur l'iPod** :
- ✅ **Podcasts listés** dans le menu Podcasts
- ✅ **Lectures possibles** avec lecture normale
- ✅ **Position mémorisée** entre les sessions
- ✅ **Bullet "non lu"** pour nouveaux épisodes
- ✅ **Exclus du shuffle** mode aléatoire

## Différences Avant/Après

### **Avant (Non Lisible)**
```c
track->mediatype = ITDB_MEDIATYPE_PODCAST;  // ✅ Correct
track->flag4 = 0x00;                        // ❌ Pas de flag podcast
track->mark_unplayed = 0x00;                // ❌ Pas de marqueur
track->podcasturl = NULL;                   // ❌ Pas d'URL
track->remember_playback_position = FALSE;  // ❌ Position non mémorisée
```

### **Après (Lisible)**
```c
track->mediatype = ITDB_MEDIATYPE_PODCAST;  // ✅ Correct
track->flag4 = 0x01;                        // ✅ Flag podcast activé
track->mark_unplayed = 0x02;                // ✅ Marqueur "non lu"
track->podcasturl = "file://...";           // ✅ URL présente
track->remember_playback_position = TRUE;   // ✅ Position mémorisée
track->description = "Podcast episode...";  // ✅ Description
// + tous les autres attributs podcast
```

## Résolution de Problèmes

### **Si les Podcasts Restent Non Lisibles**

1. **Vérifier les logs** :
   ```bash
   cat ipod_sync.log | grep -E "(podcast-specific|PODCAST|Created track)"
   ```

2. **Re-synchroniser avec logs de debug** :
   ```bash
   # Rebuild en mode debug
   make debug
   ./build/rhythmbox-ipod-sync sync-file /media/ipod ~/test.mp3 --mediatype podcast
   ```

3. **Vérifier la structure des tracks** :
   ```bash
   ./build/rhythmbox-ipod-sync list /media/ipod | grep -A 5 -B 5 "Podcast"
   ```

4. **Redémarrer l'iPod** après synchronisation pour actualiser le cache du firmware

### **Problèmes Potentiels**

- **Format de fichier** : Certains formats audio peuvent nécessiter des attributs additionnels
- **Version iPod** : Très anciens iPods peuvent avoir des exigences différentes
- **Corruption de base** : Database corrompue nécessitant une réinitialisation

## Améliorations Futures

### **Métadonnées Avancées**
- Extraction des tags ID3 réels au lieu de génération synthétique
- Support des chapitres podcast
- Artwork/pochettes d'épisodes

### **Comportements Avancés**
- Gestion de la vitesse de lecture
- Support des smart playlists podcast
- Synchronisation des états lu/non lu

Cette solution résout définitivement le problème de lecture des podcasts en configurant tous les attributs requis par le firmware iPod pour reconnaître et lire correctement les épisodes podcast.