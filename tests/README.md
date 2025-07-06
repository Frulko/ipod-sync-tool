# Tests pour rhythmbox-ipod-sync

Ce dossier contient les tests unitaires et d'intégration pour le projet rhythmbox-ipod-sync.

## Organisation

- `unit/` - Tests unitaires pour les fonctions individuelles
- `integration/` - Tests d'intégration pour les workflows complets
- `fixtures/` - Fichiers de test (échantillons audio avec métadonnées)
- `scripts/` - Scripts utilitaires pour les tests

## Tests disponibles

### Tests de métadonnées (TagLib)
- `test_taglib_metadata.c` - Test d'extraction de métadonnées avec TagLib
- `test_taglib_artwork.cpp` - Test d'extraction d'artwork avec TagLib C++

### Tests d'intégration
- `test_ipod_sync.c` - Test complet de synchronisation iPod

## Compilation des tests

```bash
# Depuis le répertoire racine du projet
make tests
```

## Exécution

```bash
# Tests individuels
./tests/unit/test_taglib_metadata
./tests/unit/test_taglib_artwork

# Tests d'intégration (nécessite un iPod connecté)
./tests/integration/test_ipod_sync
```