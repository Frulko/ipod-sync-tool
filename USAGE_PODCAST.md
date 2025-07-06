# 🎙️ Guide d'utilisation des métadonnées podcast

## Nouvelles fonctionnalités implémentées

Suite aux pistes trouvées dans les projets Strawberry Music Player, les améliorations suivantes ont été implémentées :

### ✅ 1. Conversion automatique d'artwork en JPEG

```c
// Le système convertit automatiquement tous les formats d'image vers JPEG
// pour une meilleure compatibilité iPod
if (meta->artwork_format && strcmp(meta->artwork_format, "jpeg") != 0) {
    // Conversion automatique PNG/WEBP/BMP → JPEG avec GdkPixbuf
    // Qualité : 90% pour équilibre taille/qualité
}
```

**Avantages :**
- ✅ Compatibilité maximale avec tous les modèles d'iPod
- ✅ Conversion native en mémoire (pas de fichiers temporaires)
- ✅ Fallback sur méthode fichier si conversion échoue
- ✅ Support de tous formats d'image courants (PNG, WEBP, BMP, etc.)

### ✅ 2. Métadonnées podcast étendues

Nouveaux champs ajoutés dans `AudioMetadata` :

```c
typedef struct {
    // ... champs existants ...
    
    // Nouvelles métadonnées podcast
    int season_number;     // Numéro de saison
    int episode_number;    // Numéro d'épisode  
    char *episode_id;      // Identifiant unique de l'épisode
    char *podcast_name;    // Nom du podcast/émission
    char *episode_summary; // Résumé détaillé de l'épisode
} AudioMetadata;
```

### ✅ 3. Fonctions utilitaires pour podcasts

```c
// Définir les métadonnées principales du podcast
void set_podcast_metadata(AudioMetadata *meta, 
                         const char *podcast_name,    // "Mon Podcast" 
                         int season,                  // 2
                         int episode,                 // 15
                         const char *episode_id,      // "MP-S02E15"
                         time_t release_date);        // 1704067200

// Définir les descriptions
void set_podcast_description(AudioMetadata *meta,
                            const char *description,  // Description courte
                            const char *summary);     // Résumé détaillé

// Définir les URLs
void set_podcast_urls(AudioMetadata *meta,
                     const char *podcast_url,       // URL de l'épisode
                     const char *rss_url);          // URL du flux RSS
```

## 📋 Exemple d'utilisation complète

```c
#include "rbipod-metadata.h"

int main() {
    // 1. Extraire métadonnées depuis fichier
    AudioMetadata *meta = extract_metadata_from_file("podcast.mp3");
    
    // 2. Définir le type comme podcast
    meta->mediatype = ITDB_MEDIATYPE_PODCAST;
    
    // 3. Configurer les informations de l'épisode
    set_podcast_metadata(meta, 
                        "Tech Podcast FR",           // Nom du podcast
                        2,                           // Saison 2
                        15,                          // Épisode 15
                        "TPF-S02E15",               // ID unique
                        1704067200);                // 1er janvier 2024
    
    // 4. Ajouter descriptions détaillées
    set_podcast_description(meta,
                           "Intelligence artificielle en 2024",
                           "Exploration des dernières avancées en IA, "
                           "nouveaux modèles et impact industrie.");
    
    // 5. Configurer les URLs
    set_podcast_urls(meta,
                    "https://techpodcast.fr/s02e15",
                    "https://techpodcast.fr/feed.rss");
    
    // 6. Créer la track libgpod (mapping automatique)
    Itdb_Track *track = create_ipod_track_from_metadata(meta, ipod_path, NULL);
    
    // 7. La track est maintenant prête avec toutes les métadonnées
    
    free_metadata(meta);
    return 0;
}
```

## 🎯 Mapping libgpod automatique

Le système mappe automatiquement les nouvelles métadonnées vers les champs libgpod :

| Métadonnée podcast | Champ libgpod | Description |
|-------------------|---------------|-------------|
| `season_number` | `track->cd_nr` | Numéro de disque (saison) |
| `episode_number` | `track->track_nr` | Numéro de piste (épisode) |
| `podcast_name` | `track->album` | Album (nom du podcast) |
| `episode_id` | `track->grouping` | Groupement (ID épisode) |
| `episode_summary` | `track->description` | Description (si vide) |
| `time_released` | `track->time_released` | Date de sortie |

## 📱 Affichage sur iPod

Avec ces métadonnées, l'iPod affichera :

```
Podcasts
└── Tech Podcast FR (podcast_name)
    └── Saison 2 (season_number)
        └── 15. Intelligence artificielle... (episode_number + title)
            📅 01/01/2024 (time_released)
            📝 Exploration des dernières... (episode_summary)
```

## 🔧 Utilisation pratique

### Synchroniser un dossier de podcasts :

```bash
# Synchroniser comme podcasts avec métadonnées automatiques
./rhythmbox-ipod-sync sync-folder-filtered /media/ipod/ ~/Podcasts/ podcast
```

### Synchroniser un fichier avec métadonnées personnalisées :

```bash
# Le fichier sera traité avec les nouvelles fonctions métadonnées
./rhythmbox-ipod-sync sync-file /media/ipod/ ~/podcast.mp3 --mediatype podcast
```

## 🎨 Conversion artwork

Le système gère automatiquement :

1. **Détection du format** : PNG, WEBP, BMP, etc.
2. **Conversion JPEG** : Qualité 90% optimale pour iPod
3. **Intégration mémoire** : Via `itdb_track_set_thumbnails_from_data`
4. **Fallback fichier** : Si conversion mémoire échoue

**Log d'exemple :**
```
[DEBUG] Converting artwork from png to JPEG for better iPod compatibility
[DEBUG] Successfully converted artwork to JPEG: 45231 -> 38542 bytes  
[DEBUG] Successfully added artwork to track: Episode 15 (38542 bytes, format: jpeg (converted))
```

## 🏆 Résultats

**✅ Tous les problèmes initiaux résolus :**

1. **Titres corrects** : Plus jamais "cover" ou "image1"
2. **Artwork universel** : Conversion automatique vers format compatible
3. **Métadonnées podcast** : Saison, épisode, dates, descriptions complètes
4. **API officielle** : Respect total de libgpod et ses bonnes pratiques
5. **Performance** : Approche mémoire vs fichiers temporaires

Le système est maintenant **production-ready** avec support complet des podcasts ! 🎉