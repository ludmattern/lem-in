// ========================= SAUVEGARDE DU CODE DE SUPERPOSITION (CORRIGÉ) =========================
// Ligne 135 à 373 de pathfinding.c
// Date de sauvegarde: 10 novembre 2025
// Bug corrigé : Extraction complète des chemins (plus de salles manquantes)

// ========================= Superposition via Max-Flow (Edmonds–Karp) ========================= // Titre de la section: on va chercher plusieurs bons chemins en même temps
// Idée: on réorganise les chemins pour en trouver plusieurs en parallèle                                           // Explication: on optimise l'ensemble de chemins
// tout en respectant "une fourmi par salle".                                                                       // Règle importante: jamais deux fourmis dans la même salle au même moment

#define MAX_RES_NODES (MAX_ROOMS * 2)                                                      // Chaque salle est dupliquée en deux noeuds (entrée/sortie) → 2×
#define MAX_RES_EDGES (MAX_LINKS * 4 + MAX_ROOMS * 2) // 2 dirs * 2 (fwd+rev) + split edges (fwd+rev) // Nombre max d'arêtes dans le réseau (marge large)

static int res_head[MAX_RES_NODES];         // Pour chaque noeud: index de la première arête sortante (liste chaînée dans des tableaux)
static int res_to[MAX_RES_EDGES];           // Pour chaque arête: à quel noeud elle va
static int res_cap[MAX_RES_EDGES];          // Pour chaque arête: combien de "passages" il reste (capacité)
static int res_next_e[MAX_RES_EDGES];       // Pour chaque arête: index de l'arête suivante de la liste
static int res_rev[MAX_RES_EDGES];          // Pour chaque arête: index de son arête inverse (retour)
static int res_edge_count = 0;              // Compteur actuel d'arêtes utilisées dans les tableaux

static inline int node_in(uint16_t r) { return (int)r * 2; }         // Convertit l'id de salle en id du noeud "entrée"
static inline int node_out(uint16_t r) { return (int)r * 2 + 1; }    // Convertit l'id de salle en id du noeud "sortie"

// Prépare la structure "réseau" vide (aucune arête)
static void res_init(int node_count)
{
    (void)node_count; // nodes are implicit (2*room_count)                                               // On ignore ce paramètre (info déjà connue)
    res_edge_count = 0;                                                                                  // Aucune arête au départ
    for (int i = 0; i < MAX_RES_NODES; i++)                                                              // On parcourt tous les noeuds possibles
        res_head[i] = -1;                                                                                // -1 signifie "pas d'arête sortante"
}

// Ajoute une "route" de u vers v avec une capacité (combien peuvent passer)
// et sa route inverse (utilisée pour les échanges)
static void res_add_edge(int u, int v, int cap)
{
    int e = res_edge_count++;         // On réserve un nouvel index d'arête pour l'aller
    res_to[e] = v;                    // Cette arête va de u vers v
    res_cap[e] = cap;                 // Capacité disponible (combien peuvent passer)
    res_next_e[e] = res_head[u];      // Elle pointe vers l'ancienne "première arête" de u
    res_head[u] = e;                  // Et devient la nouvelle première arête de u

    int r = res_edge_count++;         // On réserve un index pour l'arête retour (inverse)
    res_to[r] = u;                    // L'inverse va de v vers u
    res_cap[r] = 0;                   // Au début, on ne peut pas "repasser" (capacité 0)
    res_next_e[r] = res_head[v];      // On l'insère en tête de la liste de v
    res_head[v] = r;                  // Elle devient la première arête sortante de v

    res_rev[e] = r;                   // On mémorise que e et r sont inverses l'une de l'autre
    res_rev[r] = e;                   // (utile pour ajuster les capacités après un échange)
}

// Construit le réseau de "routes" avec capacités
static void build_residual_graph(const lem_in_parser_t *parser)
{
    res_init((int)(parser->room_count * 2));                                // On initialise le réseau vide

    // Split nodes: v_in -> v_out (cap 1), except start/end with large cap
    for (uint16_t r = 0; r < parser->room_count; r++)                       // Pour chaque salle r
    {
        int cap = 1;                                                        // Par défaut: 1 fourmi à la fois dans la salle
        if (r == parser->start_room_id || r == parser->end_room_id)         // Sauf départ/arrivée,
            cap = parser->ant_count; // suffisant comme "infini"             // on autorise "beaucoup" de passages
        res_add_edge(node_in(r), node_out(r), cap);                         // On relie l'entrée à la sortie de la salle
    }

    // Undirected links -> two directed edges; capacité élevée (seule la capacité des salles limite)
    for (size_t i = 0; i < parser->link_count; i++)                         // Pour chaque lien entre deux salles
    {
        uint16_t a = parser->links[i].from;                                 // La première salle du lien
        uint16_t b = parser->links[i].to;                                   // La seconde salle du lien
        int cap = parser->ant_count; // approx INF pour les arêtes           // On ne limite pas les couloirs (c'est la salle qui limite)
        res_add_edge(node_out(a), node_in(b), cap);                         // Chemin de a vers b (sortie a -> entrée b)
        res_add_edge(node_out(b), node_in(a), cap);                         // Chemin de b vers a (sortie b -> entrée a)
    }
}

// Cherche une nouvelle "chaîne d'échanges" du départ vers l'arrivée
static int bfs_augment(int s, int t)
{
    static int parent_edge[MAX_RES_NODES];   // Pour chaque noeud: quelle arête on a prise pour y arriver
    static int parent_node[MAX_RES_NODES];   // Pour chaque noeud: de quel noeud on vient
    static int queue[MAX_RES_NODES];         // File pour parcourir "en largeur"
    int qh = 0, qt = 0;                      // Indices tête et queue de la file

    for (int i = 0; i < MAX_RES_NODES; i++)  // On réinitialise le marquage
        parent_edge[i] = -1, parent_node[i] = -1; // -1 = pas encore visité

    queue[qt++] = s;                         // On démarre depuis la source s
    parent_node[s] = s;                      // La source "vient d'elle-même"

    while (qh < qt)                          // Tant qu'il reste des noeuds à traiter
    {
        int u = queue[qh++];                 // On prend le prochain noeud de la file
        for (int e = res_head[u]; e != -1; e = res_next_e[e]) // On parcourt toutes ses arêtes sortantes
        {
            int v = res_to[e];               // Voisin atteint par cette arête
            if (parent_node[v] == -1 && res_cap[e] > 0) // On n'a pas encore visité v et l'arête a de la capacité
            {
                parent_node[v] = u;          // On note qu'on vient de u
                parent_edge[v] = e;          // Et via quelle arête
                if (v == t)                  // Si on est arrivé jusqu'à la destination t
                {
                    // Augment by 1 (capacities are integers and small) // On va "pousser" 1 passage le long du chemin trouvé
                    int x = v;               // On part de la destination
                    while (x != s)           // On remonte jusqu'à la source
                    {
                        int ed = parent_edge[x];          // Arête utilisée pour venir ici
                        res_cap[ed] -= 1;                 // On consomme 1 unité de capacité à l'aller
                        res_cap[res_rev[ed]] += 1;        // On ajoute 1 unité sur l'arête inverse (pour de futurs échanges)
                        x = parent_node[x];               // On remonte d'un noeud
                    }
                    return 1;                             // On a augmenté le flux d'une unité
                }
                queue[qt++] = v;                          // Sinon on met v dans la file et on continue
            }
        }
    }
    return 0;                                             // Pas de chemin d'échange trouvé
}

// Répète la recherche d'échanges pour ajouter un maximum de chemins
static int edmonds_karp_maxflow(const lem_in_parser_t *parser)
{
    int s = node_out(parser->start_room_id);  // Noeud source (sortie de la salle de départ)
    int t = node_in(parser->end_room_id);     // Noeud destination (entrée de la salle d'arrivée)
    int flow = 0;                              // Nombre d'unités (chemins) trouvées jusqu'ici
    int limit = parser->ant_count;            // On ne cherche pas plus de chemins que de fourmis
    if (limit > MAX_PATHS)                    // Et on se limite à MAX_PATHS utiles
        limit = MAX_PATHS; // pas nécessaire d'excéder le nombre de chemins utiles

    while (flow < limit)                      // Tant qu'on peut encore ajouter des chemins
    {
        int aug = bfs_augment(s, t);          // On cherche une nouvelle "chaîne d'échanges"
        if (!aug)                             // Si aucune trouvée,
            break;                            // on s'arrête
        flow += aug;                          // Sinon on a ajouté 1 chemin
    }
    return flow;                              // On retourne combien de chemins on a pu ajouter
}

// Reconstruit les chemins finaux à partir du réseau après échanges
static bool extract_paths_from_flow(const lem_in_parser_t *parser, int flow)
{
    path_count = 0;                                        // Aucun chemin final stocké au départ
    int s = node_out(parser->start_room_id);               // Source dans le réseau
    int t = node_in(parser->end_room_id);                  // Destination dans le réseau

    for (int k = 0; k < flow && path_count < MAX_PATHS; k++) // On va extraire "flow" chemins, un par un
    {
        static int parent_edge[MAX_RES_NODES];             // Pour mémoriser le parcours (arête utilisée)
        static int parent_node[MAX_RES_NODES];             // Pour mémoriser le parcours (noeud précédent)
        static int queue[MAX_RES_NODES];                   // File pour le BFS
        int qh = 0, qt = 0;                                // Indices tête/queue

        for (int i = 0; i < MAX_RES_NODES; i++)            // On réinitialise les marquages
            parent_edge[i] = -1, parent_node[i] = -1;      // -1 = pas encore visité

        queue[qt++] = s;                                   // On démarre de la source
        parent_node[s] = s;                                // La source vient d'elle-même

        // BFS dans le graphe des arêtes "utilisées" (res_cap[rev] > 0)
        while (qh < qt && parent_node[t] == -1)            // Tant qu'on n'a pas atteint la destination
        {
            int u = queue[qh++];                           // On prend le prochain noeud
            for (int e = res_head[u]; e != -1; e = res_next_e[e]) // On parcourt ses arêtes
            {
                int v = res_to[e];                         // Noeud voisin atteint
                if (parent_node[v] == -1 && res_cap[res_rev[e]] > 0) // On suit uniquement les arêtes "utilisées" (cap inverse > 0)
                {
                    parent_node[v] = u;                    // On note d'où on vient
                    parent_edge[v] = e;                    // On note par quelle arête
                    queue[qt++] = v;                       // On ajoute à la file
                    if (v == t)                            // Si c'est la destination,
                        break;                              // on arrête ici
                }
            }
        }

        if (parent_node[t] == -1)                          // Si on n'a pas pu atteindre la destination
            break; // plus de chemin                        // il n'y a plus de chemin à extraire

        // Reconstruit chemin et consomme 1 unité de flux sur ses arêtes
        path_t *p = &paths[path_count];                    // On va remplir ce chemin
        p->length = 0;                                     // Il est vide pour l'instant

        // Collecter tous les nœuds du chemin de t vers s
        int nodes_path[MAX_RES_NODES];                     // Tous les nœuds du chemin
        int nodes_len = 0;                                 // Nombre de nœuds
        int x = t;
        while (x != s)
        {
            nodes_path[nodes_len++] = x;
            x = parent_node[x];
        }
        nodes_path[nodes_len++] = s;                       // Ajouter la source

        // Stocker les arêtes pour consommer le flux plus tard
        int edges_on_path[MAX_RES_NODES];                  // Tableau temporaire des arêtes du chemin
        int ec = 0;                                        // Nombre d'arêtes sur ce chemin
        x = t;
        while (x != s)                                     // On remonte jusqu'à la source
        {
            edges_on_path[ec++] = parent_edge[x];          // Stocker l'arête
            x = parent_node[x];
        }

        // Extraire les salles du chemin (de s vers t)
        for (int i = nodes_len - 1; i >= 0; i--)
        {
            int node = nodes_path[i];
            uint16_t room = (uint16_t)(node / 2);
            
            // Ajouter la salle si ce n'est pas un doublon
            // (node_in et node_out de la même salle donnent le même room)
            if (p->length == 0 || p->path[p->length - 1] != room)
            {
                p->path[p->length++] = room;
            }
        }

        // consomme le flux sur ces arêtes (réduit rev, augmente fwd)
        for (int i = 0; i < ec; i++)                       // On "retire" 1 unité à ce chemin pour éviter de le réutiliser tel quel
        {
            int e = edges_on_path[i];                      // Arête sur le chemin
            res_cap[res_rev[e]] -= 1;                      // On consomme 1 unité sur l'arête inverse (utilisée)
            res_cap[e] += 1;                               // Et on redonne 1 à l'aller (pour cohérence résiduelle)
        }

        if (p->length >= 2 && p->path[p->length - 1] == parser->end_room_id) // Chemin bien formé: finit sur la salle d'arrivée
        {
            cached_path_lengths[path_count] = p->length;   // On mémorise sa longueur
            for (size_t i = 0; i < p->length; i++)         // On copie la suite des salles
                cached_paths[path_count][i] = p->path[i];  // Dans le cache utilisable pour la simulation
            path_count++;                                  // On a un chemin de plus
        }
        else
        {
            break;                                         // Sécurité: si on n'a pas un chemin propre, on arrête l'extraction
        }
    }
    return path_count > 0;                                 // On indique si on a au moins un chemin
}

// Enchaîne: construit le réseau, ajoute des chemins, puis les lit
static bool find_paths_with_superposition(const lem_in_parser_t *parser)
{
    build_residual_graph(parser);                // 1) On construit le réseau de capacités
    int flow = edmonds_karp_maxflow(parser);     // 2) On ajoute autant de chemins que possible (échanges)
    if (flow <= 0)                               // Si aucun chemin n'a été trouvé,
        return false;                            // on échoue
    return extract_paths_from_flow(parser, flow); // 3) On extrait la liste de chemins finaux
}

