# Guide libgpod pour l'artwork

Ce document résume les fonctionnalités libgpod pour la gestion des covers d'après la documentation officielle.

## Fonctions principales pour l'artwork

### itdb_track_set_thumbnails()
```c
gboolean itdb_track_set_thumbnails(Itdb_Track *track, const char *filename);
```
- **Usage** : Ajouter une cover à une track depuis un fichier image
- **Formats supportés** : JPEG, PNG (selon le modèle d'iPod)
- **Retour** : TRUE si succès, FALSE sinon

### itdb_artwork_get_pixbuf()
```c
GdkPixbuf* itdb_artwork_get_pixbuf(Itdb_Device *device, Itdb_Artwork *artwork, 
                                   int width, int height);
```
- **Usage** : Récupérer l'artwork sous forme de GdkPixbuf
- **Paramètres** : device, artwork, dimensions souhaitées
- **Retour** : GdkPixbuf ou NULL

### itdb_track_remove_thumbnails()
```c
void itdb_track_remove_thumbnails(Itdb_Track *track);
```
- **Usage** : Supprimer toutes les covers d'une track

## Structure de données

### Itdb_Track
```c
struct Itdb_Track {
    // ... autres champs ...
    Itdb_Artwork *artwork;  // Pointeur vers l'artwork associé
    // ...
};
```

### Itdb_Artwork
```c
struct Itdb_Artwork {
    GList *thumbnails;      // Liste des thumbnails
    guint32 id;             // ID unique de l'artwork
    // ... autres champs ...
};
```

## Workflow recommandé

### 1. Ajouter l'artwork à une track
```c
// Créer ou obtenir une track
Itdb_Track *track = itdb_track_new();

// Configurer les métadonnées de base
track->title = g_strdup("Mon Titre");
track->artist = g_strdup("Mon Artiste");
// ... autres métadonnées ...

// Ajouter l'artwork depuis un fichier
gboolean success = itdb_track_set_thumbnails(track, "/path/to/cover.jpg");
if (success) {
    g_print("Artwork ajouté avec succès\n");
} else {
    g_print("Échec de l'ajout de l'artwork\n");
}

// Ajouter la track à la base de données
itdb_track_add(itdb, track, -1);
```

### 2. Vérifier la présence d'artwork
```c
if (track->artwork && track->artwork->thumbnails) {
    int thumb_count = g_list_length(track->artwork->thumbnails);
    g_print("Track a %d thumbnail(s)\n", thumb_count);
} else {
    g_print("Aucun artwork trouvé\n");
}
```

### 3. Récupérer et afficher l'artwork
```c
if (track->artwork) {
    GdkPixbuf *pixbuf = itdb_artwork_get_pixbuf(itdb->device, track->artwork, 
                                                300, 300);
    if (pixbuf) {
        // Utiliser le pixbuf (affichage, sauvegarde, etc.)
        g_object_unref(pixbuf);
    }
}
```

## Notes importantes de la documentation

### Limitations iTunes
> "Please note that iTunes additionally stores the artwork as tags in the
> original music file. That's also from where the data is read when
> artwork is displayed in iTunes, and there can be more than one piece
> of artwork. libgpod does not store the artwork as tags in the original
> music file. As a consequence, if iTunes attempts to access the
> artwork, it will find none, and remove libgpod's artwork."

**Solution recommandée** : Maintenir une liste des fichiers d'artwork originaux et les restaurer silencieusement si perdus.

### Support par modèle d'iPod
- **iPod classiques** : Support complet de l'artwork
- **iPod Nano 1-4G** : Support complet
- **iPod Nano 5G+** : Support partiel (nécessite une base iTunes existante)
- **iPhone/iPod Touch** : Support partiel
- **iPod Shuffle** : Pas de support artwork

### Formats d'image
- **JPEG** : Format principal, bien supporté
- **PNG** : Supporté sur la plupart des modèles
- **Autres formats** : Conversion recommandée en JPEG

## Bonnes pratiques

### 1. Gestion des erreurs
```c
gboolean add_artwork_safely(Itdb_Track *track, const char *artwork_path) {
    // Vérifier l'existence du fichier
    if (!g_file_test(artwork_path, G_FILE_TEST_EXISTS)) {
        return FALSE;
    }
    
    // Tentative d'ajout
    gboolean result = itdb_track_set_thumbnails(track, artwork_path);
    
    if (!result) {
        g_warning("Failed to add artwork: %s", artwork_path);
    }
    
    return result;
}
```

### 2. Préparation des images
```c
// Utiliser ffmpeg pour extraire/convertir en JPEG
char command[1024];
snprintf(command, sizeof(command), 
         "ffmpeg -i \"%s\" -map 0:v:0 -c:v mjpeg -q:v 2 \"%s\" -y 2>/dev/null",
         source_file, temp_jpeg);
         
if (system(command) == 0) {
    itdb_track_set_thumbnails(track, temp_jpeg);
    unlink(temp_jpeg); // Nettoyer le fichier temporaire
}
```

### 3. Validation de l'artwork
```c
gboolean validate_artwork(Itdb_Track *track) {
    if (!track->artwork) {
        return FALSE;
    }
    
    if (!track->artwork->thumbnails) {
        return FALSE;
    }
    
    // Vérifier la taille des thumbnails
    GList *thumbs = track->artwork->thumbnails;
    while (thumbs) {
        Itdb_Thumb *thumb = (Itdb_Thumb*)thumbs->data;
        if (thumb->width > 0 && thumb->height > 0) {
            return TRUE; // Au moins un thumbnail valide
        }
        thumbs = g_list_next(thumbs);
    }
    
    return FALSE;
}
```

## Intégration avec TagLib

### Workflow combiné TagLib + libgpod
```c
// 1. Extraire l'artwork avec TagLib (depuis notre implementation)
AudioMetadata meta = {0};
if (extract_artwork_taglib_native(file_path, &meta)) {
    
    // 2. Créer un fichier temporaire
    char temp_file[256];
    snprintf(temp_file, sizeof(temp_file), "/tmp/artwork_%d.jpg", getpid());
    
    FILE *fp = fopen(temp_file, "wb");
    if (fp) {
        fwrite(meta.artwork_data, 1, meta.artwork_size, fp);
        fclose(fp);
        
        // 3. Utiliser libgpod pour l'ajouter à la track
        gboolean success = itdb_track_set_thumbnails(track, temp_file);
        
        // 4. Nettoyer
        unlink(temp_file);
        
        if (success) {
            g_print("Artwork transféré avec succès\n");
        }
    }
    
    // Libérer les données TagLib
    g_free(meta.artwork_data);
    g_free(meta.artwork_format);
}
```

Ce workflow combine la puissance d'extraction de TagLib avec la capacité de stockage iPod de libgpod.