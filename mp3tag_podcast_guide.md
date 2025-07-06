# 🎙️ Guide mp3tag pour podcasts - Template complet

## 📦 Installation du template

### 1. **Importer l'action dans mp3tag**
1. Ouvrir **mp3tag**
2. Menu **Tools** → **Actions** → **Import**
3. Sélectionner le fichier `mp3tag_podcast_template.mta`
4. L'action "**Podcast Complete Setup**" apparaît dans la liste

### 2. **Configuration des colonnes mp3tag**
Ajouter ces colonnes pour voir toutes les métadonnées :

| **Nom colonne** | **Champ** | **Description** |
|-----------------|-----------|-----------------|
| Episode | TRACK | Numéro d'épisode |
| Season | DISCNUMBER | Numéro de saison |
| Episode ID | GROUPING | Identifiant unique |
| Category | CATEGORY | Catégorie podcast |
| Date | DATE | Date de sortie |
| Network | ALBUMARTIST | Réseau/producteur |

## 🎯 Workflow de tagging recommandé

### **Étape 1 : Métadonnées de base**
Avant d'appliquer l'action, renseigner **manuellement** :

```ini
TITLE = [Titre de l'épisode]
ARTIST = [Nom du podcast]  
ALBUM = [Nom du podcast]
ALBUMARTIST = [Réseau/producteur]
TRACK = [Numéro épisode]
DISCNUMBER = [Numéro saison]
COMMENT = [Description courte]
```

### **Étape 2 : Appliquer l'action**
1. Sélectionner les fichiers podcast
2. Clic droit → **Actions** → **Podcast Complete Setup**
3. Toutes les métadonnées sont automatiquement complétées

### **Étape 3 : Personnaliser URLs (optionnel)**
Modifier les champs générés automatiquement :
- `PODCASTURL` - URL spécifique de l'épisode
- `PODCASTRSS` - URL du flux RSS du podcast

## 📋 Exemples de configuration

### **Podcast Tech français**
```ini
# Métadonnées de base
TITLE = L'IA va-t-elle remplacer les développeurs ?
ARTIST = Tech Podcast FR
ALBUM = Tech Podcast FR  
ALBUMARTIST = Podcast Network France
TRACK = 25
DISCNUMBER = 3
COMMENT = Analyse de l'impact de l'IA sur le développement

# Après action automatique
GENRE = Podcast
GROUPING = TECH PODCAST FR-S3E25
CATEGORY = Technology
DATE = 2024-07-06
PODCAST = Tech Podcast FR
PODCASTNETWORK = Podcast Network France
PODCASTURL = https://podcast.example.com/s3e25
PODCASTRSS = https://podcast.example.com/feed.rss
MEDIATYPE = Podcast
DESCRIPTION = Épisode 25 de la saison 3 - L'IA va-t-elle remplacer les développeurs ?
```

### **Podcast interview**
```ini
# Métadonnées de base
TITLE = Interview avec Linus Torvalds
ARTIST = Dev Stories
ALBUM = Dev Stories
ALBUMARTIST = Independent Media
TRACK = 12
DISCNUMBER = 1
COMMENT = Rencontre exclusive avec le créateur de Linux

# Résultat automatique
GROUPING = DEV STORIES-S1E12
CATEGORY = Technology
# etc...
```

## 🎨 Template de nommage fichier

### **Dans mp3tag : Convert → Filename - Tag**
```
%albumartist% - S%discnumber%E%track% - %title%
```

**Exemples de résultats :**
- `Podcast Network France - S3E25 - L'IA va-t-elle remplacer les développeurs.mp3`
- `Independent Media - S1E12 - Interview avec Linus Torvalds.mp3`

## 🖼️ Gestion artwork

### **Recommandations images :**
- **Format :** JPG ou PNG (sera auto-converti en JPEG par notre système)
- **Taille :** 300x300 minimum, 1400x1400 maximum 
- **Contenu :** Logo du podcast ou artwork spécifique à l'épisode

### **Dans mp3tag :**
1. Sélectionner fichier(s)
2. Clic droit sur zone artwork → **Add cover...**
3. Choisir image → Notre système la convertira automatiquement

## 🔧 Actions avancées personnalisées

### **Action 1 : Cleanup Podcast**
```ini
# Nettoie et normalise les métadonnées
Field: ARTIST → Formatstring: $caps2(%artist%)
Field: ALBUM → Formatstring: $caps2(%album%)
Field: TITLE → Formatstring: $caps2(%title%)
```

### **Action 2 : Numérotation automatique**
```ini
# Auto-incrémente les numéros d'épisode
Field: TRACK → Formatstring: $num(%_counter%,2)
```

### **Action 3 : Date parsing intelligente**
```ini
# Parse la date depuis le nom de fichier si format YYYY-MM-DD
Field: DATE → Formatstring: $regexp(%_filename%,.*(\d{4}-\d{2}-\d{2}).*,$1)
```

## ⚡ Raccourcis clavier utiles

### **Configuration mp3tag recommandée :**
- **F5** : Appliquer "Podcast Complete Setup"
- **F6** : Convert Tag → Filename  
- **F7** : Ajouter artwork
- **Ctrl+S** : Sauvegarder tags

## 🎯 Résultat final dans rhythmbox-ipod-sync

Avec ce template, les fichiers auront **toutes** les métadonnées nécessaires :

```bash
# Synchronisation parfaite
./rhythmbox-ipod-sync sync-folder-filtered /media/ipod/ ~/Podcasts/ podcast
```

**Affichage iPod :**
```
Podcasts
├── Tech Podcast FR (Podcast Network France)
│   └── Saison 3
│       └── 25. L'IA va-t-elle remplacer les développeurs ?
│           📅 06/07/2024
│           📝 Analyse de l'impact de l'IA...
│           🆔 TECH PODCAST FR-S3E25
└── Dev Stories (Independent Media)  
    └── Saison 1
        └── 12. Interview avec Linus Torvalds
            📅 06/07/2024
            📝 Rencontre exclusive...
            🆔 DEV STORIES-S1E12
```

## 🚀 Utilisation en lot

### **Pour traiter 50+ épisodes :**
1. Sélectionner tous les fichiers
2. **Actions** → **Autonumbering Wizard** (pour TRACK/DISCNUMBER)
3. **Actions** → **Podcast Complete Setup**
4. Vérifier et ajuster si nécessaire
5. **Convert** → **Tag - Filename** pour renommer

**Gain de temps : 90%** 🎉

Ce template automatise tout le processus de tagging podcast pour une compatibilité parfaite avec notre système iPod !