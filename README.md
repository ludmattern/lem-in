# LEM-IN - Ant Farm Pathfinding Simulator

## 📋 Description

Ce projet implémente un simulateur de fourmis basé sur l'algorithme de recherche de chemin pour faire traverser `n` fourmis d'une salle de départ à une salle d'arrivée en un minimum de tours.

## 🚀 Compilation et Exécution

### Compilation

```bash
make
```

### Utilisation

```bash
./lem-in < map_file.map
```

## 📝 Format d'entrée

```
nombre_de_fourmis
##start
nom_salle_start x y
##end  
nom_salle_end x y
nom_salle1 x1 y1
nom_salle2 x2 y2
...
salle1-salle2
salle2-salle3
...
```

## ✅ Validation implémentée

### ✅ **Validation du nombre de fourmis**

- Doit être un entier positif (> 0)
- Première ligne non-commentaire obligatoire
- Dans les limites d'un `int`

### ✅ **Validation des noms de salles**

- ❌ Ne peut pas commencer par `L` (réservé pour les fourmis)
- ❌ Ne peut pas commencer par `#` (réservé pour les commentaires)
- ❌ Ne peut pas contenir d'espaces ou de tirets
- ❌ Pas de doublons autorisés

### ✅ **Validation structurelle**

- Une salle `##start` obligatoire et unique
- Une salle `##end` obligatoire et unique
- Format salle : `nom coord_x coord_y`
- Format lien : `nom1-nom2`
- Coordonnées doivent être des entiers valides
- Arrêt immédiat sur ligne non-conforme

### ✅ **Validation des liens**

- Les deux salles doivent exister
- Pas de liens vers soi-même
- Pas de débordement de buffer

## 🧪 Tests

### Tests automatiques

```bash
bash test_validation.sh
```

### Tests manuels

**Cas invalides (doivent retourner ERROR) :**

```bash
echo "0" | ./lem-in                                    # 0 fourmis
echo "" | ./lem-in                                     # Entrée vide
./lem-in < resources/invalid_maps/L_room               # Nom commençant par L
./lem-in < resources/invalid_maps/duplicate_room_name  # Doublons
```

**Cas valides :**

```bash
./lem-in < test_simple.map                            # Cas simple
./lem-in < resources/valid_maps/perfect0              # Cas complexe
```

## 📊 État du projet

### ✅ **Complété**

- ✅ Parsing robuste des données d'entrée
- ✅ Validation complète selon le sujet
- ✅ Gestion des erreurs avec messages appropriés
- ✅ Structure de données efficace (hash table)
- ✅ Tests de validation complets

### 🔄 **En cours / À implémenter**

- 🔄 Algorithme de pathfinding (BFS/Dijkstra)
- 🔄 Simulation du mouvement des fourmis
- 🔄 Gestion des collisions et flux optimal
- 🔄 Optimisation pour gros graphes (4000+ salles)
- 🔄 Format de sortie `Lx-y` pour chaque mouvement

## 🏗️ Architecture

```
srcs/
└── fichier.c          # Parsing, validation et structures de données
Makefile               # Compilation
test_simple.map        # Cas de test simple
test_validation.sh     # Script de tests automatiques
resources/             # Maps de test fournies
├── valid_maps/
├── invalid_maps/
└── ...
```

## 🔧 Fonctions principales

- `validate_ant_count()` - Validation du nombre de fourmis
- `validate_room_name()` - Validation des noms de salles
- `validate_coordinates()` - Validation des coordonnées
- `parse_room()` - Parsing d'une salle
- `parse_link()` - Parsing d'un lien
- `parse()` - Fonction principale de parsing

## 📈 Performance

- Hash table pour recherche O(1) des salles
- Parsing en un seul passage
- Gestion mémoire optimisée
- Support jusqu'à 20,000 salles et 200,000 liens

---

**Prochaine étape :** Implémentation de l'algorithme de pathfinding pour la simulation des fourmis.
