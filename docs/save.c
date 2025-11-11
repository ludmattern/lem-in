#include "lem_in.h"
#include <stdint.h>


// Liste des chemins trouvés (chaque chemin est une suite d'identifiants de salles)
static path_t paths[MAX_PATHS];
static size_t path_count = 0;

// Toutes les fourmis, leur position actuelle, etc.
static ant_t ants[MAX_ANTS];

// Combien de fourmis on envoie sur chaque chemin
static uint16_t ants_per_path[MAX_PATHS];

// Voisins de chaque salle (structure d'accès rapide construite depuis les liens)
static hash_table_t neighbors[MAX_ROOMS];

// Copie des chemins sous forme d'indices de salles, pour la simulation
static uint16_t cached_paths[MAX_PATHS][MAX_ROOMS];
static size_t cached_path_lengths[MAX_PATHS];

static int res_head[MAX_NODES];         
// Pour chaque noeud: index de la première arête sortante (liste chaînée dans des tableaux)
static int res_to[MAX_EDGES];           // Pour chaque arête: à quel noeud elle va
static int res_cap[MAX_EDGES];          // Pour chaque arête: combien de "passages" il reste (capacité)
static int res_next_e[MAX_EDGES];       // Pour chaque arête: index de l'arête suivante de la liste
static int res_rev[MAX_EDGES];          // Pour chaque arête: index de son arête inverse (retour)
static int res_edge_count = 0;

static void free_neighbors_table(void);

static inline int node_in(uint16_t r) { return (int)r * 2; } 
static inline int node_out(uint16_t r) { return (int)r * 2 + 1; }

static void res_init(int node_count)
{
	(void)node_count;
	res_edge_count = 0;

	for (int i = 0; i < MAX_NODES; i++)
		res_head[i] = -1;
}

static void res_add_edge(int from, int to, int cap)
{
	int e = res_edge_count++;
	res_to[e] = to;
	res_cap[e] = cap;
	res_next_e[e] = res_head[from];
	res_head[from] = e;

	int r = res_edge_count++;
	res_to[r] = from;
	res_cap[r] = 0;
	res_next_e[r] = res_head[to];
	res_head[to] = r;

	res_rev[e] = r;
	res_rev[r] = e;
}

static void residual_graph_builder(const lem_in_parser_t *parser)
{
	res_init((int)parser->room_count * 2);

	// split rooms into in and out nodes
	for (uint16_t r = 0; r < parser->room_count; r++)
	{
		int cap = 1;

		if (r == parser->start_room_id || r == parser->end_room_id)
			cap = parser->ant_count;

		// Créer l'arête entrée → sortie de cette salle
		res_add_edge(node_in(r), node_out(r), cap);
	}

	// creer les arêtes entre les salles voisines
	for (uint16_t i = 0; i < parser->link_count; i++)
	{
		uint16_t from = parser->links[i].from;
		uint16_t to = parser->links[i].to;
		int cap = parser->ant_count;

		res_add_edge(node_out(from), node_in(to), cap);
		res_add_edge(node_out(to), node_in(from), cap);
	}
}

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


// ========================= Superposition via Max-Flow (Edmonds–Karp) =========================
// TODO : À réécrire en suivant le tutoriel dans docs/TUTORIEL_SUPERPOSITION.md
// Sauvegarde du code original : docs/superposition_backup.c

// TODO Étape 0 : Déclarer les variables globales (MAX_RES_NODES, MAX_RES_EDGES, tableaux statiques)

// TODO Étape 1 : Écrire node_in() et node_out()

// TODO Étape 2 : Écrire res_init()

// TODO Étape 3 : Écrire res_add_edge()

// TODO Étape 4 : Écrire build_residual_graph()

// TODO Étape 5 : Écrire bfs_augment()

// TODO Étape 6 : Écrire edmonds_karp_maxflow()

// TODO Étape 7 : Écrire extract_paths_from_flow()

// TODO Étape 8 : Écrire find_paths_with_superposition()

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
