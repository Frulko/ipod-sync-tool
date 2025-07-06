#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "include/rbipod-metadata.h"
#include "include/rbipod-logging.h"

/*
 * EXEMPLE D'UTILISATION DES NOUVELLES FONCTIONS PODCAST
 * 
 * Ce fichier montre comment utiliser les nouvelles fonctionnalités pour
 * définir les métadonnées spécifiques aux podcasts comme demandé :
 * - Saison et épisode 
 * - Date de sortie
 * - Nom du podcast
 * - Descriptions étendues
 */

int main() {
    // Initialiser le logging
    init_logging("example_podcast.log");
    
    printf("🎙️  EXEMPLE D'UTILISATION DES MÉTADONNÉES PODCAST\n");
    printf("==============================================\n\n");
    
    // Créer une structure de métadonnées
    AudioMetadata *meta = g_malloc0(sizeof(AudioMetadata));
    if (!meta) {
        printf("❌ Erreur : Impossible d'allouer la mémoire\n");
        return 1;
    }
    
    // Définir les métadonnées de base
    meta->title = g_strdup("L'Intelligence Artificielle en 2024");
    meta->artist = g_strdup("Tech Podcast FR");
    meta->mediatype = ITDB_MEDIATYPE_PODCAST;
    
    printf("1️⃣  MÉTADONNÉES DE BASE :\n");
    printf("   Titre : %s\n", meta->title);
    printf("   Artiste : %s\n", meta->artist);
    printf("   Type : Podcast\n\n");
    
    // Utiliser les nouvelles fonctions pour définir les métadonnées podcast
    printf("2️⃣  DÉFINITION DES MÉTADONNÉES PODCAST :\n");
    
    // Définir les informations de saison/épisode/date
    time_t release_date = time(NULL) - (7 * 24 * 60 * 60); // Il y a 1 semaine
    set_podcast_metadata(meta, "Tech Podcast FR", 2, 15, "TPF-S02E15", release_date);
    
    printf("   ✅ Podcast: %s\n", meta->podcast_name ? meta->podcast_name : "N/A");
    printf("   ✅ Saison: %d\n", meta->season_number);
    printf("   ✅ Épisode: %d\n", meta->episode_number);
    printf("   ✅ ID Épisode: %s\n", meta->episode_id ? meta->episode_id : "N/A");
    printf("   ✅ Date de sortie: %ld\n", meta->time_released);
    
    // Définir les descriptions
    const char *description = "Découvrez les dernières avancées en intelligence artificielle";
    const char *summary = "Dans cet épisode, nous explorons les développements récents en IA, "
                         "les nouveaux modèles de langage, et l'impact sur l'industrie tech.";
    
    set_podcast_description(meta, description, summary);
    
    printf("   ✅ Description: %s\n", meta->description ? "DÉFINIE" : "NULL");
    printf("   ✅ Résumé: %s\n", meta->episode_summary ? "DÉFINI" : "NULL");
    
    // Définir les URLs
    set_podcast_urls(meta, "https://techpodcast.fr/episodes/s02e15", 
                           "https://techpodcast.fr/feed.rss");
    
    printf("   ✅ URL Podcast: %s\n", meta->podcasturl ? "DÉFINIE" : "NULL");
    printf("   ✅ URL RSS: %s\n", meta->podcastrss ? "DÉFINIE" : "NULL");
    
    printf("\n3️⃣  MAPPING LIBGPOD :\n");
    printf("   Les métadonnées podcast sont mappées sur libgpod comme suit :\n");
    printf("   • season_number → track->cd_nr (numéro de disque)\n");
    printf("   • episode_number → track->track_nr (numéro de piste)\n");
    printf("   • podcast_name → track->album (nom de l'album)\n");
    printf("   • episode_id → track->grouping (groupement)\n");
    printf("   • episode_summary → track->description (si description vide)\n");
    printf("   • time_released → track->time_released (date de sortie)\n");
    
    printf("\n4️⃣  UTILISATION DANS LE CODE :\n");
    printf("   // Extraire métadonnées depuis fichier\n");
    printf("   AudioMetadata *meta = extract_metadata_from_file(\"podcast.mp3\");\n");
    printf("   \n");
    printf("   // Définir les infos podcast\n");
    printf("   set_podcast_metadata(meta, \"Mon Podcast\", 1, 5, \"MP-S01E05\", time(NULL));\n");
    printf("   set_podcast_description(meta, \"Description\", \"Résumé détaillé\");\n");
    printf("   set_podcast_urls(meta, \"https://...\", \"https://feed.rss\");\n");
    printf("   \n");
    printf("   // Créer la track libgpod (automatiquement mappée)\n");
    printf("   Itdb_Track *track = create_ipod_track_from_metadata(meta, path, NULL);\n");
    
    printf("\n5️⃣  AVANTAGES :\n");
    printf("   ✅ Affichage correct des saisons/épisodes sur l'iPod\n");
    printf("   ✅ Chronologie respectée avec dates de sortie\n");
    printf("   ✅ Descriptions détaillées pour navigation\n");
    printf("   ✅ URLs pour synchronisation avec services\n");
    printf("   ✅ Compatibilité complète avec l'API libgpod officielle\n");
    
    // Nettoyer
    free_metadata(meta);
    
    printf("\n🎉 EXEMPLE TERMINÉ AVEC SUCCÈS !\n");
    printf("Les nouvelles fonctions sont prêtes à être utilisées.\n");
    
    cleanup_logging();
    return 0;
}