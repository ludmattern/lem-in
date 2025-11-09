## Guide simple de srcs/pathfinding.c

Objectif: comprendre le fichier sans jargon, en lisant une explication claire de chaque partie (variables, fonctions, et ce qui se passe).

Note: je regroupe “ligne par ligne” par petits blocs logiques pour éviter de surcharger; rien d’important n’est laissé de côté.


### En-tête et variables globales
- `#include "lem_in.h"`: on importe les types/fonctions communes du projet.
- `static path_t paths[MAX_PATHS];` et `static size_t path_count`: stockage des chemins trouvés et leur nombre.
- `static ant_t ants[MAX_ANTS];`: toutes les fourmis (id, position, etc.).
- `static uint16_t ants_per_path[MAX_PATHS];`: combien de fourmis sont affectées à chaque chemin.
- `static hash_table_t neighbors[MAX_ROOMS];`: structure d’adjacence (les voisins de chaque salle) pour des parcours rapides.
- `static uint16_t cached_paths[MAX_PATHS][MAX_ROOMS];` et `static size_t cached_path_lengths[MAX_PATHS];`: copie “cache” des chemins (séquence d’ids de salles) pour la simulation.

Idée: on garde tout ce qu’il faut pour manipuler les chemins et simuler le déplacement des fourmis.


### Fonctions d’adjacence (voisinage)
- `hash_function`: calcule une petite clé (0..255) à partir d’un id de salle pour ranger le voisin dans l’un des 256 seaux.
- `add_neighbor(room, neighbor)`: ajoute un voisin à la liste en tête (chaînage) pour la salle donnée; retourne false si la mémoire manque.
- `build_neighbors_table(parser)`: réinitialise toutes les listes de voisins puis, pour chaque lien, ajoute l’aller et le retour (graphe non orienté). Si une allocation échoue, on nettoie tout et on arrête.
- `free_neighbors_table()`: libère tous les maillons/chaînes de toutes les salles, remet à zéro.

Ces fonctions construisent/détruisent la structure “qui est voisin de qui” à partir des liens parsés.


### Vérifier qu’il existe au moins un chemin (BFS)
- `valid_path(parser)`: fait un parcours en largeur (BFS) depuis la salle de départ. On met la salle de départ en file, on visite les voisins non visités, etc. Si on atteint la salle d’arrivée, on dit “ok, au moins un chemin existe”. Sinon, on renvoie une erreur “pas de chemin”.

C’est un garde‑fou: ne pas aller plus loin si aucun chemin n’est possible.


### Superposition (max‑flow, idée simple)
But: trouver “le meilleur lot” de chemins en permettant de réorganiser les chemins entre eux, tout en respectant la règle “une seule fourmi par salle à la fois”.

Comment on encode ça (sans entrer dans la théorie):
- Chaque salle est vue comme deux moitiés (entrée/sortie) reliées entre elles par une “capacité 1” (sauf départ/arrivée, plus grandes). Ça force “au plus une fourmi” par salle.
- Chaque lien entre salles devient des passages possibles entre sorties/entrées de salles voisines, avec des capacités suffisantes.
- On cherche des “chemins d’augmentation” (des échanges de réservations) jusqu’à ce qu’il n’y en ait plus, puis on lit les chemins finaux.

Dans le code (principales fonctions):
- `res_*` (residual): tableaux internes pour stocker ce “réseau” avec capacités (ajout d’arêtes et leurs inverses).
- `build_residual_graph(parser)`: construit ce réseau depuis les salles et les liens (capacité 1 dans les salles, grande sur les arêtes entre salles).
- `bfs_augment(s, t)`: essaye de trouver un nouveau chemin à ajouter; si trouvé, met à jour les capacités (réservations échangées).
- `edmonds_karp_maxflow(parser)`: répète la recherche de nouveaux chemins jusqu’à blocage (ou limite utile atteinte).
- `extract_paths_from_flow(parser, flow)`: reconstruit les chemins finaux utiles depuis ce “flux” trouvé (on suit les passages utilisés), et remplit `paths` + `cached_paths`.
- `find_paths_with_superposition(parser)`: enchaîne les étapes ci‑dessus (construit, lance, extrait) et renvoie true si on a au moins un chemin.

À retenir: c’est là que la “magie” de la superposition s’opère. On en ressort avec plusieurs bons chemins en parallèle si le plan s’y prête.


### Répartition des fourmis par chemin
- `calculate_ants_per_path(parser)`: calcule combien de fourmis seront envoyées sur chaque chemin. Version simple: partage équitable (et le reste sur les premiers). (Une version plus “intelligente” mettrait davantage de fourmis sur les chemins les plus courts.)

But: préparer la simulation pour savoir combien de fourmis suivent chaque chemin.


### Préparer les fourmis
- `init_ants(parser)`: crée les fourmis (1..N), pose chaque fourmi au départ, indique qu’elles n’ont pas encore commencé à avancer (`path_index = 0`) et qu’elles ne sont pas “arrivées”.

Idée: tout le monde est prêt, au départ, au tour 0.


### Simuler un tour de mouvements
- `simulate_turn(parser, turn)`: fait bouger les fourmis d’une étape si possible, en respectant les règles:
  - Une seule fourmi dans une salle à un instant donné (on autorise plusieurs au départ / à l’arrivée).
  - On avance dans l’ordre des fourmis; pour chacune, on tente de passer à la prochaine salle de son chemin si elle est libre.
  - À chaque mouvement réussi, on imprime `L<id>-<nomSalle>`; on sépare plusieurs mouvements par des espaces; s’il y a eu au moins un mouvement, on ajoute un retour à la ligne à la fin du tour.

Résultat: une ligne par tour, avec les déplacements effectués à ce tour.


### Calculer combien de tours on va simuler
- `calculate_total_turns(parser)`: calcule une borne simple du nombre de tours nécessaires, basée sur la longueur des chemins et le nombre de fourmis par chemin (formule classique du sujet). On prend le maximum sur tous les chemins.

Idée: ça donne combien de tours on va faire pour afficher la simulation.


### Lancer toute la simulation
- `simulate_all_turns(parser)`: appelle `calculate_total_turns`, puis exécute chaque tour (1 à total), en appelant `simulate_turn`. À la fin, on imprime `# Nombre de lignes: X` (récapitulatif du nombre de tours).


### Point d’entrée de ce module (chercher des chemins, répartir, simuler)
- `find_paths(parser)`:
  1) Construit les voisins (`build_neighbors_table`), utilisés par la simulation et certains parcours.
  2) Tente la superposition (`find_paths_with_superposition`) pour obtenir le meilleur lot de chemins (si l’architecture de la map le permet).
  3) Calcule la répartition des fourmis (`calculate_ants_per_path`).
  4) Initialise les fourmis (`init_ants`).
  5) Lance la simulation et imprime le résultat (`simulate_all_turns`).
  6) Nettoie la table de voisins (`free_neighbors_table`).

En une phrase: on fabrique de bons chemins, on décide combien de fourmis les empruntent, puis on joue le film tour par tour et on imprime les déplacements.


### Résumé mental rapide
- Construire les voisins (qui touche qui).
- Chercher de bons chemins (superposition = réarranger pour maximiser le parallèle).
- Choisir combien de fourmis par chemin.
- Simuler les déplacements un tour à la fois, en respectant “une salle = une fourmi” (sauf départ/arrivée).
- Récapitulatif du nombre de lignes (tours).


Si tu veux, je peux ajouter des commentaires courts directement dans `srcs/pathfinding.c` au‑dessus de chaque bloc/fonction (sans polluer chaque ligne), pour que l’explication “vive” dans le code.*** End Patch

