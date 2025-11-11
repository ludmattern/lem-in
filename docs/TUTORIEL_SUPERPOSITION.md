# 📚 Tutoriel : Réécrire la Superposition (Max-Flow) Étape par Étape

## 🎯 Objectif
Tu vas réécrire toi-même l'algorithme de superposition pour vraiment le comprendre. Je vais te guider étape par étape, du plus simple au plus complexe.

---

## 📋 Table des matières
1. [Préparation : Les structures de données](#étape-0--préparation)
2. [Étape 1 : Transformer les salles (node splitting)](#étape-1--node-splitting)
3. [Étape 2 : Initialiser le réseau](#étape-2--initialiser-le-réseau)
4. [Étape 3 : Ajouter des arêtes](#étape-3--ajouter-des-arêtes)
5. [Étape 4 : Construire le graphe résiduel](#étape-4--construire-le-graphe-résiduel)
6. [Étape 5 : BFS pour trouver un chemin](#étape-5--bfs-pour-augmenter-le-flux)
7. [Étape 6 : Répéter pour trouver tous les chemins](#étape-6--edmonds-karp-max-flow)
8. [Étape 7 : Extraire les chemins finaux](#étape-7--extraire-les-chemins)
9. [Étape 8 : Fonction principale](#étape-8--fonction-principale)

---

## Étape 0 : Préparation

### 🧠 Concept de base
On va créer un **réseau de capacités** où chaque salle est coupée en deux :
- Un nœud **entrée** (où on arrive)
- Un nœud **sortie** (d'où on repart)
- Entre les deux : une **capacité de 1** (= 1 fourmi à la fois)

### 📝 Variables globales à déclarer

```c
// ========================= Superposition via Max-Flow =========================

#define MAX_RES_NODES (MAX_ROOMS * 2)
#define MAX_RES_EDGES (MAX_LINKS * 4 + MAX_ROOMS * 2)

static int res_head[MAX_RES_NODES];      // Pour chaque nœud : première arête
static int res_to[MAX_RES_EDGES];        // Pour chaque arête : nœud destination
static int res_cap[MAX_RES_EDGES];       // Pour chaque arête : capacité restante
static int res_next_e[MAX_RES_EDGES];    // Pour chaque arête : arête suivante
static int res_rev[MAX_RES_EDGES];       // Pour chaque arête : son arête inverse
static int res_edge_count = 0;           // Nombre d'arêtes créées
```

**✍️ À faire** : Ajoute ces variables globales après la ligne 14 de `pathfinding.c` (après `static hash_table_t neighbors[MAX_ROOMS];`).

---

## Étape 1 : Node Splitting

### 🧠 Concept
Chaque salle `r` devient deux nœuds :
- `node_in(r) = r * 2`
- `node_out(r) = r * 2 + 1`

Exemple : Salle 5
- `node_in(5) = 10`
- `node_out(5) = 11`

### 📝 Code à écrire

```c
// Convertit un ID de salle en ID de nœud "entrée"
static inline int node_in(uint16_t r)
{
    // TODO : retourner r * 2
}

// Convertit un ID de salle en ID de nœud "sortie"
static inline int node_out(uint16_t r)
{
    // TODO : retourner r * 2 + 1
}
```

**✍️ À faire** : Écris ces deux fonctions. Elles sont toutes simples !

**🧪 Test mental** : Si la salle 3 existe, quels sont ses nœuds entrée/sortie ?
<details>
<summary>Réponse</summary>

- `node_in(3) = 6`
- `node_out(3) = 7`
</details>

---

## Étape 2 : Initialiser le réseau

### 🧠 Concept
Avant de commencer, il faut "réinitialiser" toutes les structures :
- Mettre `res_edge_count` à 0
- Mettre tous les `res_head[i]` à -1 (= "pas d'arête")

### 📝 Code à écrire

```c
// Initialise le réseau vide
static void res_init(int node_count)
{
    (void)node_count; // On ne l'utilise pas directement
    
    // TODO : Mettre res_edge_count à 0
    
    // TODO : Boucle for de 0 à MAX_RES_NODES
    //        Pour chaque i, mettre res_head[i] = -1
}
```

**✍️ À faire** : Complète cette fonction.

**💡 Astuce** : C'est comme "effacer le tableau" avant de dessiner un nouveau graphe dessus.

---

## Étape 3 : Ajouter des arêtes

### 🧠 Concept
Pour ajouter une arête de `u` vers `v` avec une capacité `cap`, on doit :
1. Créer l'arête directe `u → v` avec capacité `cap`
2. Créer l'arête inverse `v → u` avec capacité `0` (pour les échanges futurs)
3. Lier ces deux arêtes ensemble avec `res_rev`

**Analogie** : C'est comme créer une route à double sens, mais au début seul un sens est ouvert.

### 📝 Code à écrire

```c
// Ajoute une arête u→v avec capacité cap, et son arête inverse
static void res_add_edge(int u, int v, int cap)
{
    // ===== Arête directe u→v =====
    int e = res_edge_count++;        // TODO : Prendre un nouvel index
    // TODO : res_to[e] = v
    // TODO : res_cap[e] = cap
    // TODO : res_next_e[e] = res_head[u]  (on l'insère en tête de liste)
    // TODO : res_head[u] = e              (elle devient la première)
    
    // ===== Arête inverse v→u =====
    int r = res_edge_count++;        // TODO : Prendre un nouvel index
    // TODO : res_to[r] = u
    // TODO : res_cap[r] = 0               (capacité inverse nulle au départ)
    // TODO : res_next_e[r] = res_head[v]
    // TODO : res_head[v] = r
    
    // ===== Lier les deux arêtes =====
    // TODO : res_rev[e] = r
    // TODO : res_rev[r] = e
}
```

**✍️ À faire** : Complète cette fonction en remplaçant les TODO.

**🧪 Test mental** : Si on appelle `res_add_edge(2, 5, 3)`, combien d'arêtes sont créées ?
<details>
<summary>Réponse</summary>

2 arêtes :
- Arête directe 2→5 (capacité 3)
- Arête inverse 5→2 (capacité 0)
</details>

---

## Étape 4 : Construire le graphe résiduel

### 🧠 Concept
On va créer le réseau complet :
1. Pour chaque salle : créer `entrée → sortie` (capacité 1, sauf start/end)
2. Pour chaque lien A-B : créer `A_sortie → B_entrée` et `B_sortie → A_entrée`

### 📝 Code à écrire

```c
// Construit le réseau de capacités
static void build_residual_graph(const lem_in_parser_t *parser)
{
    // TODO : Appeler res_init() avec parser->room_count * 2
    
    // ===== Partie 1 : Couper les salles en deux =====
    // TODO : Boucle for r de 0 à parser->room_count
    //        {
    //            int cap = 1;  // Par défaut
    //            
    //            // Si c'est start ou end, cap = parser->ant_count
    //            if (r == parser->start_room_id || r == parser->end_room_id)
    //                cap = parser->ant_count;
    //            
    //            // Ajouter arête node_in(r) → node_out(r) avec capacité cap
    //            res_add_edge(node_in(r), node_out(r), cap);
    //        }
    
    // ===== Partie 2 : Créer les couloirs entre salles =====
    // TODO : Boucle for i de 0 à parser->link_count
    //        {
    //            uint16_t a = parser->links[i].from;
    //            uint16_t b = parser->links[i].to;
    //            int cap = parser->ant_count; // Capacité "infinie" pour les couloirs
    //            
    //            // Ajouter node_out(a) → node_in(b)
    //            res_add_edge(node_out(a), node_in(b), cap);
    //            
    //            // Ajouter node_out(b) → node_in(a)
    //            res_add_edge(node_out(b), node_in(a), cap);
    //        }
}
```

**✍️ À faire** : Complète cette fonction en suivant les TODO.

**💡 Pourquoi start/end ont une grande capacité ?**
Parce qu'on veut que plusieurs fourmis puissent y être en même temps (start au début, end à la fin).

---

## Étape 5 : BFS pour augmenter le flux

### 🧠 Concept
On cherche un chemin de `s` (source) vers `t` (destination) dans le réseau où toutes les arêtes ont de la capacité disponible. Si on en trouve un, on "pousse" 1 unité de flux dessus.

**Analogie** : C'est comme trouver un chemin libre sur une carte routière, puis dire "OK, 1 voiture va passer par là".

### 📝 Code à écrire (c'est le plus complexe !)

```c
// Cherche un chemin avec de la capacité et pousse 1 unité de flux
static int bfs_augment(int s, int t)
{
    // Déclarations des tableaux statiques
    static int parent_edge[MAX_RES_NODES];
    static int parent_node[MAX_RES_NODES];
    static int queue[MAX_RES_NODES];
    int qh = 0, qt = 0;
    
    // TODO : Réinitialiser parent_edge et parent_node à -1
    //        Boucle for i de 0 à MAX_RES_NODES
    //            parent_edge[i] = -1;
    //            parent_node[i] = -1;
    
    // TODO : Ajouter s à la queue
    //        queue[qt++] = s;
    //        parent_node[s] = s;  (la source vient d'elle-même)
    
    // ===== BFS =====
    // TODO : while (qh < qt)
    //        {
    //            int u = queue[qh++];  // Prendre le prochain nœud
    //            
    //            // Parcourir toutes les arêtes sortantes de u
    //            for (int e = res_head[u]; e != -1; e = res_next_e[e])
    //            {
    //                int v = res_to[e];  // Nœud destination
    //                
    //                // Si v pas visité ET arête a de la capacité
    //                if (parent_node[v] == -1 && res_cap[e] > 0)
    //                {
    //                    parent_node[v] = u;
    //                    parent_edge[v] = e;
    //                    
    //                    // Si on a atteint t, on pousse le flux !
    //                    if (v == t)
    //                    {
    //                        // Remonter le chemin et ajuster les capacités
    //                        int x = v;
    //                        while (x != s)
    //                        {
    //                            int ed = parent_edge[x];
    //                            res_cap[ed] -= 1;          // Réduire capacité directe
    //                            res_cap[res_rev[ed]] += 1; // Augmenter capacité inverse
    //                            x = parent_node[x];
    //                        }
    //                        return 1;  // On a poussé 1 unité
    //                    }
    //                    
    //                    queue[qt++] = v;  // Ajouter v à la queue
    //                }
    //            }
    //        }
    
    return 0;  // Aucun chemin trouvé
}
```

**✍️ À faire** : C'est la fonction la plus difficile. Prends ton temps, relis l'explication et complète-la petit à petit.

**💡 Astuce** : La partie "remonter le chemin" est cruciale. On part de `t` et on remonte jusqu'à `s` en suivant `parent_node`.

---

## Étape 6 : Edmonds-Karp (Max-Flow)

### 🧠 Concept
On répète `bfs_augment` jusqu'à ce qu'on ne puisse plus trouver de chemins. Le nombre total d'augmentations = nombre de chemins trouvés.

### 📝 Code à écrire

```c
// Trouve le nombre maximum de chemins
static int edmonds_karp_maxflow(const lem_in_parser_t *parser)
{
    // TODO : int s = node_out(parser->start_room_id);
    // TODO : int t = node_in(parser->end_room_id);
    // TODO : int flow = 0;
    
    // TODO : int limit = parser->ant_count;
    // TODO : if (limit > MAX_PATHS)
    //            limit = MAX_PATHS;
    
    // TODO : while (flow < limit)
    //        {
    //            int aug = bfs_augment(s, t);
    //            if (!aug)
    //                break;  // Plus de chemins
    //            flow += aug;
    //        }
    
    // TODO : return flow;
}
```

**✍️ À faire** : Complète cette fonction.

**🧪 Test mental** : Si on a 100 fourmis mais seulement 5 chemins possibles, combien de fois va-t-on appeler `bfs_augment` ?
<details>
<summary>Réponse</summary>

Maximum 5 fois (ou jusqu'à ce que `bfs_augment` retourne 0).
</details>

---

## Étape 7 : Extraire les chemins

### 🧠 Concept
Maintenant qu'on a "poussé" du flux dans le réseau, il faut **lire** les chemins pour les donner au simulateur. On va faire un BFS qui suit les arêtes "utilisées" (celles où `res_cap[res_rev[e]] > 0`).

**Analogie** : C'est comme suivre les traces de voitures sur la neige pour voir quelles routes ont été utilisées.

### 📝 Structure générale

```c
static bool extract_paths_from_flow(const lem_in_parser_t *parser, int flow)
{
    path_count = 0;
    int s = node_out(parser->start_room_id);
    int t = node_in(parser->end_room_id);
    
    // Pour chaque unité de flux (= chaque chemin)
    for (int k = 0; k < flow && path_count < MAX_PATHS; k++)
    {
        // 1. BFS pour trouver un chemin "utilisé"
        // 2. Reconstruire le chemin en salles réelles
        // 3. "Consommer" ce chemin pour ne pas le retrouver
    }
    
    return path_count > 0;
}
```

Cette fonction est très longue. Je te donne la structure en 3 parties :

### 📝 Partie A : BFS pour trouver un chemin utilisé

```c
// (À mettre dans la boucle for k)
static int parent_edge[MAX_RES_NODES];
static int parent_node[MAX_RES_NODES];
static int queue[MAX_RES_NODES];
int qh = 0, qt = 0;

// TODO : Réinitialiser parent_edge et parent_node à -1

// TODO : queue[qt++] = s;
//        parent_node[s] = s;

// BFS qui suit les arêtes "utilisées"
// TODO : while (qh < qt && parent_node[t] == -1)
//        {
//            int u = queue[qh++];
//            for (int e = res_head[u]; e != -1; e = res_next_e[e])
//            {
//                int v = res_to[e];
//                // Condition spéciale : on suit les arêtes où res_cap[res_rev[e]] > 0
//                if (parent_node[v] == -1 && res_cap[res_rev[e]] > 0)
//                {
//                    parent_node[v] = u;
//                    parent_edge[v] = e;
//                    queue[qt++] = v;
//                    if (v == t)
//                        break;
//                }
//            }
//        }

// TODO : if (parent_node[t] == -1)
//            break;  // Plus de chemins à extraire
```

### 📝 Partie B : Reconstruire le chemin

**IMPORTANT** : Cette partie a été corrigée pour éviter le bug des salles manquantes !

```c
path_t *p = &paths[path_count];
p->length = 0;

// Collecter tous les nœuds du chemin de t vers s
int nodes_path[MAX_RES_NODES];
int nodes_len = 0;
int x = t;

// TODO : Remonter de t vers s et collecter TOUS les nœuds
//        while (x != s)
//        {
//            nodes_path[nodes_len++] = x;
//            x = parent_node[x];
//        }
//        nodes_path[nodes_len++] = s;  // Ajouter la source

// Stocker les arêtes du chemin pour le flux
int edges_on_path[MAX_RES_NODES];
int ec = 0;
x = t;

// TODO : Remonter de t vers s pour les arêtes
//        while (x != s)
//        {
//            edges_on_path[ec++] = parent_edge[x];
//            x = parent_node[x];
//        }

// Extraire les salles du chemin (de s vers t)
// TODO : for (int i = nodes_len - 1; i >= 0; i--)
//        {
//            int node = nodes_path[i];
//            uint16_t room = (uint16_t)(node / 2);
//            
//            // Ajouter la salle si ce n'est pas un doublon
//            // (node_in et node_out donnent le même room)
//            if (p->length == 0 || p->path[p->length - 1] != room)
//            {
//                p->path[p->length++] = room;
//            }
//        }
```

**Note** : On collecte d'abord TOUS les nœuds, puis on les convertit en salles en éliminant les doublons. C'est plus robuste que de suivre uniquement les transitions `out→in`.

**🐛 Bug corrigé** : L'ancienne version ne capturait que les transitions `out→in`, ce qui faisait sauter des salles intermédiaires. Exemple : le chemin `Start→X→Y→P→End` devenait `Start→X→P→End` (manquait Y). La nouvelle version capture tous les nœuds du chemin, élimine les doublons (car `node_in(X)` et `node_out(X)` correspondent à la même salle X), et reconstruit correctement le chemin complet.

### 📝 Partie C : Consommer le flux et stocker

```c
// Consommer le flux sur ces arêtes
// TODO : for (int i = 0; i < ec; i++)
//        {
//            int e = edges_on_path[i];
//            res_cap[res_rev[e]] -= 1;
//            res_cap[e] += 1;
//        }

// Vérifier et stocker le chemin
// TODO : if (p->length >= 2 && p->path[p->length - 1] == parser->end_room_id)
//        {
//            cached_path_lengths[path_count] = p->length;
//            for (size_t i = 0; i < p->length; i++)
//                cached_paths[path_count][i] = p->path[i];
//            path_count++;
//        }
//        else
//        {
//            break;
//        }
```

**✍️ À faire** : Assemble les 3 parties dans `extract_paths_from_flow`. C'est long mais logique !

---

## Étape 8 : Fonction principale

### 🧠 Concept
C'est la fonction qui orchestre tout :
1. Construire le réseau
2. Trouver le max-flow
3. Extraire les chemins

### 📝 Code à écrire

```c
// Fonction principale de la superposition
static bool find_paths_with_superposition(const lem_in_parser_t *parser)
{
    // TODO : build_residual_graph(parser);
    
    // TODO : int flow = edmonds_karp_maxflow(parser);
    
    // TODO : if (flow <= 0)
    //            return false;
    
    // TODO : return extract_paths_from_flow(parser, flow);
}
```

**✍️ À faire** : Complète cette fonction.

---

## 🎉 Intégration finale

Une fois toutes les fonctions écrites, modifie `find_paths` pour appeler ta nouvelle fonction :

```c
bool find_paths(lem_in_parser_t *parser)
{
    if (!build_neighbors_table(parser))
        return print_error(ERR_MEMORY, "neighbors table allocation");

    find_paths_with_superposition(parser);  // ← Ta nouvelle fonction !

    calculate_ants_per_path(parser);
    init_ants(parser);
    simulate_all_turns(parser);
    free_neighbors_table();

    return true;
}
```

---

## ✅ Checklist finale

Avant de tester :
- [ ] Toutes les variables globales sont déclarées
- [ ] `node_in` et `node_out` sont implémentés
- [ ] `res_init` est implémenté
- [ ] `res_add_edge` est implémenté
- [ ] `build_residual_graph` est implémenté
- [ ] `bfs_augment` est implémenté
- [ ] `edmonds_karp_maxflow` est implémenté
- [ ] `extract_paths_from_flow` est implémenté (3 parties)
- [ ] `find_paths_with_superposition` est implémenté
- [ ] `find_paths` appelle `find_paths_with_superposition`

---

## 🧪 Test

```bash
make re
./lem-in < resources/slow_maps/new_big-superposition
```

Si tu vois un nombre de lignes beaucoup plus bas qu'avant, **BRAVO !** 🎉

---

## 🆘 En cas de problème

Si ça ne compile pas ou si ça plante :
1. Vérifie les indices de tableaux (pas de dépassement)
2. Vérifie que toutes les boucles sont bien fermées
3. Regarde la sauvegarde dans `docs/superposition_backup.c` pour comparer

---

## 💪 Challenge bonus

Une fois que tout marche, essaie de :
1. Ajouter des `ft_printf` pour voir combien de chemins sont trouvés
2. Afficher la longueur de chaque chemin trouvé
3. Compter combien d'itérations fait `bfs_augment`

---

**Bon courage ! 🚀 Prends ton temps, et n'hésite pas à me demander si tu bloques sur une étape.**

