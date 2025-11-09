// Ce fichier décide des chemins, répartit les fourmis, et imprime leurs mouvements.
// Les commentaires expliquent chaque bloc simplement, comme si on découvrait l'informatique.
#include "lem_in.h"


// Liste des chemins trouvés (chaque chemin est une suite d'identifiants de salles)
static path_t paths[MAX_PATHS];
static size_t path_count = 0;
// Toutes les fourmis, leur position actuelle, etc.
static ant_t ants[MAX_ANTS];
// Combien de fourmis on envoie sur chaque chemin
static uint16_t ants_per_path[MAX_PATHS];
// Voisins de chaque salle (structure d'accès rapide construite depuis les liens)
static hash_table_t neighbors[MAX_ROOMS];

// Cache to avoid recalculating paths
// Copie des chemins sous forme d'indices de salles, pour la simulation
static uint16_t cached_paths[MAX_PATHS][MAX_ROOMS];
static size_t cached_path_lengths[MAX_PATHS];

// Forward declarations
static void free_neighbors_table(void);

// Hash function
static inline uint8_t hash_function(uint16_t value)
{
	return (uint8_t)(value & 0xFF);
}

// Add neighbor to hash table
static inline bool add_neighbor(uint16_t room, uint16_t neighbor)
{
	uint8_t hash = hash_function(neighbor);
	hash_node_t *node = malloc(sizeof(hash_node_t));
	if (!node)
		return false; // Échec d'allocation
	node->neighbor = neighbor;
	node->next = neighbors[room].buckets[hash];
	neighbors[room].buckets[hash] = node;
	neighbors[room].count++;
	return true;
}

// Build neighbors hash table
// Construit la table des voisins (qui touche qui) à partir des liens lus
static bool build_neighbors_table(const lem_in_parser_t *parser)
{
	for (size_t i = 0; i < MAX_ROOMS; i++)
	{
		for (int j = 0; j < 256; j++)
			neighbors[i].buckets[j] = NULL;
		neighbors[i].count = 0;
	}

	for (size_t i = 0; i < parser->link_count; i++)
	{
		uint16_t from = parser->links[i].from;
		uint16_t to = parser->links[i].to;

		if (!add_neighbor(from, to) || !add_neighbor(to, from))
		{
			// En cas d'échec, nettoyer la mémoire déjà allouée
			free_neighbors_table();
			return false;
		}
	}
	return true;
}

// Free hash table memory
// Libère toute la mémoire allouée pour la table des voisins
static void free_neighbors_table(void)
{
	for (size_t i = 0; i < MAX_ROOMS; i++)
	{
		for (int j = 0; j < 256; j++)
		{
			hash_node_t *current = neighbors[i].buckets[j];
			while (current)
			{
				hash_node_t *temp = current;
				current = current->next;
				free(temp);
			}
			neighbors[i].buckets[j] = NULL;
		}
		neighbors[i].count = 0;
	}
}

// BFS with hash table
// Vérifie simplement qu'il existe au moins un chemin du départ à l'arrivée
bool valid_path(const lem_in_parser_t *parser)
{
	if (!build_neighbors_table(parser))
		return print_error(ERR_MEMORY, "neighbors table allocation");

	uint16_t queue[MAX_ROOMS];
	bool visited_bfs[MAX_ROOMS] = {false};
	size_t front = 0, rear = 0;

	queue[rear++] = parser->start_room_id;
	visited_bfs[parser->start_room_id] = true;

	while (front < rear)
	{
		uint16_t current = queue[front++];

		if (current == parser->end_room_id)
		{
			free_neighbors_table();
			return true;
		}

		for (int bucket = 0; bucket < 256; bucket++)
		{
			hash_node_t *node = neighbors[current].buckets[bucket];
			while (node)
			{
				if (!visited_bfs[node->neighbor])
				{
                    visited_bfs[node->neighbor] = true;
                    queue[rear++] = node->neighbor;
                }
                node = node->next;
            }
        }
    }
    
    free_neighbors_table();
    return print_error(ERR_NO_PATH, NULL);
}


// ========================= Superposition via Max-Flow (Edmonds–Karp) ========================= // Titre de la section: on va chercher plusieurs bons chemins en même temps
// Idée: on réorganise les chemins pour en trouver plusieurs en parallèle                                           // Explication: on optimise l'ensemble de chemins
// tout en respectant "une fourmi par salle".                                                                       // Règle importante: jamais deux fourmis dans la même salle au même moment

#define MAX_RES_NODES (MAX_ROOMS * 2)                                                      // Chaque salle est dupliquée en deux noeuds (entrée/sortie) → 2×
#define MAX_RES_EDGES (MAX_LINKS * 4 + MAX_ROOMS * 2) // 2 dirs * 2 (fwd+rev) + split edges (fwd+rev) // Nombre max d'arêtes dans le réseau (marge large)

static int res_head[MAX_RES_NODES];         // Pour chaque noeud: index de la première arête sortante (liste chaînée dans des tableaux)
static int res_to[MAX_RES_EDGES];           // Pour chaque arête: à quel noeud elle va
static int res_cap[MAX_RES_EDGES];          // Pour chaque arête: combien de “passages” il reste (capacité)
static int res_next_e[MAX_RES_EDGES];       // Pour chaque arête: index de l’arête suivante de la liste
static int res_rev[MAX_RES_EDGES];          // Pour chaque arête: index de son arête inverse (retour)
static int res_edge_count = 0;              // Compteur actuel d’arêtes utilisées dans les tableaux

static inline int node_in(uint16_t r) { return (int)r * 2; }         // Convertit l’id de salle en id du noeud “entrée”
static inline int node_out(uint16_t r) { return (int)r * 2 + 1; }    // Convertit l’id de salle en id du noeud “sortie”

// Prépare la structure "réseau" vide (aucune arête)
static void res_init(int node_count)
{
    (void)node_count; // nodes are implicit (2*room_count)                                               // On ignore ce paramètre (info déjà connue)
    res_edge_count = 0;                                                                                  // Aucune arête au départ
    for (int i = 0; i < MAX_RES_NODES; i++)                                                              // On parcourt tous les noeuds possibles
        res_head[i] = -1;                                                                                // -1 signifie “pas d’arête sortante”
}

// Ajoute une "route" de u vers v avec une capacité (combien peuvent passer)
// et sa route inverse (utilisée pour les échanges)
static void res_add_edge(int u, int v, int cap)
{
    int e = res_edge_count++;         // On réserve un nouvel index d’arête pour l’aller
    res_to[e] = v;                    // Cette arête va de u vers v
    res_cap[e] = cap;                 // Capacité disponible (combien peuvent passer)
    res_next_e[e] = res_head[u];      // Elle pointe vers l’ancienne “première arête” de u
    res_head[u] = e;                  // Et devient la nouvelle première arête de u

    int r = res_edge_count++;         // On réserve un index pour l’arête retour (inverse)
    res_to[r] = u;                    // L’inverse va de v vers u
    res_cap[r] = 0;                   // Au début, on ne peut pas “repasser” (capacité 0)
    res_next_e[r] = res_head[v];      // On l’insère en tête de la liste de v
    res_head[v] = r;                  // Elle devient la première arête sortante de v

    res_rev[e] = r;                   // On mémorise que e et r sont inverses l’une de l’autre
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
            cap = parser->ant_count; // suffisant comme "infini"             // on autorise “beaucoup” de passages
        res_add_edge(node_in(r), node_out(r), cap);                         // On relie l’entrée à la sortie de la salle
    }

    // Undirected links -> two directed edges; capacité élevée (seule la capacité des salles limite)
    for (size_t i = 0; i < parser->link_count; i++)                         // Pour chaque lien entre deux salles
    {
        uint16_t a = parser->links[i].from;                                 // La première salle du lien
        uint16_t b = parser->links[i].to;                                   // La seconde salle du lien
        int cap = parser->ant_count; // approx INF pour les arêtes           // On ne limite pas les couloirs (c’est la salle qui limite)
        res_add_edge(node_out(a), node_in(b), cap);                         // Chemin de a vers b (sortie a -> entrée b)
        res_add_edge(node_out(b), node_in(a), cap);                         // Chemin de b vers a (sortie b -> entrée a)
    }
}

// Cherche une nouvelle "chaîne d'échanges" du départ vers l'arrivée
static int bfs_augment(int s, int t)
{
    static int parent_edge[MAX_RES_NODES];   // Pour chaque noeud: quelle arête on a prise pour y arriver
    static int parent_node[MAX_RES_NODES];   // Pour chaque noeud: de quel noeud on vient
    static int queue[MAX_RES_NODES];         // File pour parcourir “en largeur”
    int qh = 0, qt = 0;                      // Indices tête et queue de la file

    for (int i = 0; i < MAX_RES_NODES; i++)  // On réinitialise le marquage
        parent_edge[i] = -1, parent_node[i] = -1; // -1 = pas encore visité

    queue[qt++] = s;                         // On démarre depuis la source s
    parent_node[s] = s;                      // La source “vient d’elle-même”

    while (qh < qt)                          // Tant qu’il reste des noeuds à traiter
    {
        int u = queue[qh++];                 // On prend le prochain noeud de la file
        for (int e = res_head[u]; e != -1; e = res_next_e[e]) // On parcourt toutes ses arêtes sortantes
        {
            int v = res_to[e];               // Voisin atteint par cette arête
            if (parent_node[v] == -1 && res_cap[e] > 0) // On n’a pas encore visité v et l’arête a de la capacité
            {
                parent_node[v] = u;          // On note qu’on vient de u
                parent_edge[v] = e;          // Et via quelle arête
                if (v == t)                  // Si on est arrivé jusqu’à la destination t
                {
                    // Augment by 1 (capacities are integers and small) // On va “pousser” 1 passage le long du chemin trouvé
                    int x = v;               // On part de la destination
                    while (x != s)           // On remonte jusqu’à la source
                    {
                        int ed = parent_edge[x];          // Arête utilisée pour venir ici
                        res_cap[ed] -= 1;                 // On consomme 1 unité de capacité à l’aller
                        res_cap[res_rev[ed]] += 1;        // On ajoute 1 unité sur l’arête inverse (pour de futurs échanges)
                        x = parent_node[x];               // On remonte d’un noeud
                    }
                    return 1;                             // On a augmenté le flux d’une unité
                }
                queue[qt++] = v;                          // Sinon on met v dans la file et on continue
            }
        }
    }
    return 0;                                             // Pas de chemin d’échange trouvé
}

// Répète la recherche d'échanges pour ajouter un maximum de chemins
static int edmonds_karp_maxflow(const lem_in_parser_t *parser)
{
    int s = node_out(parser->start_room_id);  // Noeud source (sortie de la salle de départ)
    int t = node_in(parser->end_room_id);     // Noeud destination (entrée de la salle d’arrivée)
    int flow = 0;                              // Nombre d’unités (chemins) trouvées jusqu’ici
    int limit = parser->ant_count;            // On ne cherche pas plus de chemins que de fourmis
    if (limit > MAX_PATHS)                    // Et on se limite à MAX_PATHS utiles
        limit = MAX_PATHS; // pas nécessaire d'excéder le nombre de chemins utiles

    while (flow < limit)                      // Tant qu’on peut encore ajouter des chemins
    {
        int aug = bfs_augment(s, t);          // On cherche une nouvelle “chaîne d’échanges”
        if (!aug)                             // Si aucune trouvée,
            break;                            // on s’arrête
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

    for (int k = 0; k < flow && path_count < MAX_PATHS; k++) // On va extraire “flow” chemins, un par un
    {
        static int parent_edge[MAX_RES_NODES];             // Pour mémoriser le parcours (arête utilisée)
        static int parent_node[MAX_RES_NODES];             // Pour mémoriser le parcours (noeud précédent)
        static int queue[MAX_RES_NODES];                   // File pour le BFS
        int qh = 0, qt = 0;                                // Indices tête/queue

        for (int i = 0; i < MAX_RES_NODES; i++)            // On réinitialise les marquages
            parent_edge[i] = -1, parent_node[i] = -1;      // -1 = pas encore visité

        queue[qt++] = s;                                   // On démarre de la source
        parent_node[s] = s;                                // La source vient d’elle-même

        // BFS dans le graphe des arêtes "utilisées" (res_cap[rev] > 0)
        while (qh < qt && parent_node[t] == -1)            // Tant qu’on n’a pas atteint la destination
        {
            int u = queue[qh++];                           // On prend le prochain noeud
            for (int e = res_head[u]; e != -1; e = res_next_e[e]) // On parcourt ses arêtes
            {
                int v = res_to[e];                         // Noeud voisin atteint
                if (parent_node[v] == -1 && res_cap[res_rev[e]] > 0) // On suit uniquement les arêtes “utilisées” (cap inverse > 0)
                {
                    parent_node[v] = u;                    // On note d’où on vient
                    parent_edge[v] = e;                    // On note par quelle arête
                    queue[qt++] = v;                       // On ajoute à la file
                    if (v == t)                            // Si c’est la destination,
                        break;                              // on arrête ici
                }
            }
        }

        if (parent_node[t] == -1)                          // Si on n’a pas pu atteindre la destination
            break; // plus de chemin                        // il n’y a plus de chemin à extraire

        // Reconstruit chemin et consomme 1 unité de flux sur ses arêtes
        path_t *p = &paths[path_count];                    // On va remplir ce chemin
        p->length = 0;                                     // Il est vide pour l’instant
        p->path[p->length++] = parser->start_room_id;      // On commence par la salle de départ

        int x = t;                                         // On part de la destination
        // Stocke les arêtes le long du chemin pour décrémenter à la fin (on va inverser ensuite)
        int edges_on_path[MAX_RES_NODES];                  // Tableau temporaire des arêtes du chemin
        int ec = 0;                                        // Nombre d’arêtes sur ce chemin
        while (x != s)                                     // On remonte jusqu’à la source
        {
            int e = parent_edge[x];                        // Arête utilisée pour venir à x
            edges_on_path[ec++] = e;                       // On l’ajoute à la liste
            x = parent_node[x];                            // On recule d’un noeud
        }
        // parcourir dans le bon sens (de s à t)
        int cur = s;                                       // On repart de la source
        for (int i = ec - 1; i >= 0; i--)                  // Et on rejoue les arêtes dans l’ordre
        {
            int e = edges_on_path[i];                      // Arête suivante
            int v = res_to[e];                             // Noeud atteint
            // si on traverse out->in, on ajoute la salle v/2
            if ((cur % 2) == 1 && (v % 2) == 0)            // On passe d’une “sortie de salle” à une “entrée de salle”
            {
                uint16_t room_v = (uint16_t)(v / 2);       // On convertit le noeud “entrée” en id de salle
                if (p->length < MAX_ROOMS)                 // Sécurité: ne pas dépasser le tableau
                    p->path[p->length++] = room_v;         // On ajoute la salle au chemin
            }
            cur = v;                                       // On avance
        }

        // consomme le flux sur ces arêtes (réduit rev, augmente fwd)
        for (int i = 0; i < ec; i++)                       // On “retire” 1 unité à ce chemin pour éviter de le réutiliser tel quel
        {
            int e = edges_on_path[i];                      // Arête sur le chemin
            res_cap[res_rev[e]] -= 1;                      // On consomme 1 unité sur l’arête inverse (utilisée)
            res_cap[e] += 1;                               // Et on redonne 1 à l’aller (pour cohérence résiduelle)
        }

        if (p->length >= 2 && p->path[p->length - 1] == parser->end_room_id) // Chemin bien formé: finit sur la salle d’arrivée
        {
            cached_path_lengths[path_count] = p->length;   // On mémorise sa longueur
            for (size_t i = 0; i < p->length; i++)         // On copie la suite des salles
                cached_paths[path_count][i] = p->path[i];  // Dans le cache utilisable pour la simulation
            path_count++;                                  // On a un chemin de plus
        }
        else
        {
            break;                                         // Sécurité: si on n’a pas un chemin propre, on arrête l’extraction
        }
    }
    return path_count > 0;                                 // On indique si on a au moins un chemin
}

// Enchaîne: construit le réseau, ajoute des chemins, puis les lit
static bool find_paths_with_superposition(const lem_in_parser_t *parser)
{
    build_residual_graph(parser);                // 1) On construit le réseau de capacités
    int flow = edmonds_karp_maxflow(parser);     // 2) On ajoute autant de chemins que possible (échanges)
    if (flow <= 0)                               // Si aucun chemin n’a été trouvé,
        return false;                            // on échoue
    return extract_paths_from_flow(parser, flow); // 3) On extrait la liste de chemins finaux
}

// Calculate ant distribution
// Décide combien de fourmis iront sur chaque chemin (partage simple)
static void calculate_ants_per_path(const lem_in_parser_t *parser)
{
	for (size_t i = 0; i < path_count; i++)
		ants_per_path[i] = 0;

	if (path_count == 0)
		return;

	uint16_t remaining_ants = parser->ant_count;

	uint16_t ants_per_path_base = remaining_ants / path_count;
	uint16_t extra_ants = remaining_ants % path_count;

	for (size_t i = 0; i < path_count; i++)
	{
		ants_per_path[i] = ants_per_path_base;
		if (i < extra_ants)
			ants_per_path[i]++;
	}
}

// Initialize ants
// Crée les fourmis et les place au départ
static void init_ants(const lem_in_parser_t *parser)
{
	uint16_t ant_id = 1;

	for (size_t path_idx = 0; path_idx < path_count; path_idx++)
	{
		for (uint16_t ant_count = 0; ant_count < ants_per_path[path_idx]; ant_count++)
		{
			if (ant_id > parser->ant_count)
				break;

			ants[ant_id - 1].id = ant_id;
			ants[ant_id - 1].current_room = parser->start_room_id;
			ants[ant_id - 1].path_index = 0;
			ants[ant_id - 1].finished = false;

			ant_id++;
		}
	}
}

// Simulate one turn
// Fait bouger les fourmis d'une étape si possible et imprime la ligne du tour
static void simulate_turn(const lem_in_parser_t *parser, int turn) // Réalise un tour d'animation
{
	(void)turn; // Paramètre non utilisé (conservé pour cohérence)
	bool first_move = true; // Pour savoir si on met un espace avant le prochain mouvement

	for (uint16_t i = 0; i < parser->ant_count; i++)
	{
		if (ants[i].finished) // Déjà arrivée à la fin ?
			continue; // On ignore cette fourmi

		uint16_t path_id = 0; // Quel chemin suit cette fourmi
		uint16_t ant_offset = 0; // Cumul pour retrouver sa tranche d'IDs
		for (size_t p = 0; p < path_count; p++)
		{
			if (ants[i].id <= ant_offset + ants_per_path[p])
			{
				path_id = p; // On a trouvé le chemin
				break; // On arrête la recherche
			}
			ant_offset += ants_per_path[p]; // On avance au prochain chemin
		}

		if (ants[i].path_index < cached_path_lengths[path_id])
		{
			uint16_t next_room = cached_paths[path_id][ants[i].path_index]; // Prochaine salle ciblée

			bool room_available = true; // On suppose la salle libre
			if (next_room != parser->start_room_id && next_room != parser->end_room_id) // Start/End ne bloquent pas
			{
				for (uint16_t j = 0; j < parser->ant_count; j++)
				{
					if (j != i && !ants[j].finished && ants[j].current_room == next_room)
					{
						room_available = false; // Salle occupée par une autre fourmi
						break; // On ne peut pas avancer
					}
				}
			}

			if (room_available)
			{
				if (first_move)
				{
					ft_printf("L%d-%s", ants[i].id, parser->rooms[next_room].name); // Premier mouvement de la ligne
					first_move = false; // Les suivants seront précédés d'un espace
				}
				else
					ft_printf(" L%d-%s", ants[i].id, parser->rooms[next_room].name); // Mouvement suivant

				ants[i].current_room = next_room; // Mise à jour de la position
				ants[i].path_index++; // On passe à l'étape suivante du chemin

				if (next_room == parser->end_room_id)
				{
					ants[i].finished = true; // Arrivée atteinte
				}
			}
		}
	}

	if (!first_move) // S'il y a eu au moins un mouvement ce tour
		ft_printf("\n"); // On termine la ligne
}

// Calculate total turns
// Calcule le nombre total de tours nécessaires (formule du sujet)
static int calculate_total_turns(const lem_in_parser_t *parser) // Calcule le nombre total de tours
{
	(void)parser; // Non utilisé ici
	int total_turns = 0; // On prendra le maximum sur tous les chemins

	for (size_t i = 0; i < path_count; i++) // On parcourt chaque chemin
	{
		if (ants_per_path[i] > 0) // Chemin utilisé par au moins une fourmi ?
		{
			int time_for_path = paths[i].length + ants_per_path[i] - 1; // Formule du sujet
			if (time_for_path > total_turns) // Mise à jour du pire cas
			{
				total_turns = time_for_path; // Nouveau maximum
			}
		}
	}

	return total_turns; // Nombre de lignes de mouvements
}

// Simulate all turns
// Lance tous les tours de la simulation et imprime un récapitulatif
static void simulate_all_turns(const lem_in_parser_t *parser) // Joue tous les tours et affiche le bilan
{
	int total_turns = calculate_total_turns(parser); // Nombre de tours à exécuter

	for (int turn = 1; turn <= total_turns; turn++) // Pour chaque tour
	{
		simulate_turn(parser, turn); // On fait avancer et on imprime ce tour
	}

	ft_printf("# Nombre de lignes: %d\n", total_turns); // Récapitulatif final
}

// Main function
// Point d'entrée: on prépare, on cherche des chemins, on répartit, on simule
bool find_paths(lem_in_parser_t *parser) // Orchestration: prépare, cherche, répartit, simule
{
	if (!build_neighbors_table(parser)) // Construit la table d'adjacence
		return print_error(ERR_MEMORY, "neighbors table allocation"); // Erreur si mémoire manquante

    find_paths_with_superposition(parser); // Cherche un lot de chemins via superposition

	calculate_ants_per_path(parser); // Répartit les fourmis sur les chemins

	init_ants(parser); // Place les fourmis au départ

	simulate_all_turns(parser); // Exécute et imprime la simulation

	free_neighbors_table(); // Nettoie la structure d'adjacence

	return true; // Succès
}
