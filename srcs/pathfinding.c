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
static int res_cost[MAX_EDGES];        
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

static void res_add_edge(int from, int to, int cap, int cost)
{
	int e = res_edge_count++;
	res_to[e] = to;
	res_cap[e] = cap;
	res_cost[e] = cost;
	res_next_e[e] = res_head[from];
	res_head[from] = e;

	int r = res_edge_count++;
	res_to[r] = from;
	res_cap[r] = 0;
	res_cost[r] = -cost;
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
		res_add_edge(node_in(r), node_out(r), cap, 1);
	}

	// creer les arêtes entre les salles voisines
	for (uint16_t i = 0; i < parser->link_count; i++)
	{
		uint16_t from = parser->links[i].from;
		uint16_t to = parser->links[i].to;
		int cap = parser->ant_count;

		res_add_edge(node_out(from), node_in(to), cap, 0);
		res_add_edge(node_out(to), node_in(from), cap, 0);
	}
}

static int spfa_augment(int start, int end, int ant_count)
{
	static int parent_edge[MAX_NODES];
	static int parent_node[MAX_NODES];
	static int queue[MAX_NODES];
	static bool in_queue[MAX_NODES];
	static int dist[MAX_NODES];

	
	for (int i = 0; i < MAX_NODES; i++)
	{
		dist[i] = 2147483647;
		parent_edge[i] = -1;
		parent_node[i] = -1;
		in_queue[i] = false;
	}
	
	int front = 0, rear = 0;
	queue[rear] = start;
	rear = (rear + 1) % MAX_NODES;
	dist[start] = 0;
	in_queue[start] = true;

	while (front != rear)
	{
		int current = queue[front];
		front = (front + 1) % MAX_NODES;
		in_queue[current] = false;

		for (int e = res_head[current]; e != -1; e = res_next_e[e])
		{
			int destination = res_to[e];

			if (res_cap[e] > 0 && dist[current] != 2147483647 && dist[current] + res_cost[e] < dist[destination])
			{
				dist[destination] = dist[current] + res_cost[e];
				parent_node[destination] = current;
				parent_edge[destination] = e;
				if (!in_queue[destination])
				{
					int next_rear = (rear + 1) % MAX_NODES;
					if (next_rear == front)
						continue;
					queue[rear] = destination;
					rear = next_rear;
					in_queue[destination] = true;
				}
			}
		}
	}
	if (dist[end] == 2147483647)
		return 0;
	int min_cap = ant_count;
	int x = end;
	while (x != start)
	{
		int e = parent_edge[x];
		if (res_cap[e] < min_cap)
			min_cap = res_cap[e];
		x = parent_node[x];
	}
	x = end;

	while (x != start)
	{
		int e = parent_edge[x];
		res_cap[e] -= min_cap;
		res_cap[res_rev[e]] += min_cap;
		x = parent_node[x];
	}

	return min_cap;
}

static int min_cost_maxflow(const lem_in_parser_t *parser)
{
	int start = node_out(parser->start_room_id);
	int end = node_in(parser->end_room_id);
	int path_found = 0;

	int limit = MAX_PATHS;

	while (path_found < limit)
	{
		int augmented = spfa_augment(start, end, 1);
		if (!augmented)
			break;
		path_found++;
	}
	return path_found;
}

static bool extract_paths_from_flow(const lem_in_parser_t *parser, int flow)
{
	path_count = 0;

	int start = node_out(parser->start_room_id);
	int end = node_in(parser->end_room_id);

	static int parent_edge[MAX_NODES];
	static int parent_node[MAX_NODES];
	static int dist[MAX_NODES];
	static int queue[MAX_NODES];

	// On extrait exactement 'flow' chemins
	while (path_count < (size_t)flow && path_count < MAX_PATHS)
	{
		int front = 0, rear = 0;

		// Initialisation : chercher le chemin de coût minimum dans le flux utilisé
		for (int j = 0; j < MAX_NODES; j++)
		{
			parent_edge[j] = -1;
			parent_node[j] = -1;
			dist[j] = 2147483647;
		}

		queue[rear] = start;
		rear = (rear + 1) % MAX_NODES;
		parent_node[start] = start;
		dist[start] = 0;

		// BFS modifié : on cherche les arêtes où le flux inverse > 0
		while (front != rear && parent_node[end] == -1)
		{
			int current = queue[front];
			front = (front + 1) % MAX_NODES;
			for (int e = res_head[current]; e != -1; e = res_next_e[e])
			{
				int destination = res_to[e];
				int rev = res_rev[e];
				
				// Si le flux inverse > 0, ça veut dire qu'on a poussé du flux ici
				if (parent_node[destination] == -1 && res_cap[rev] > 0 && 
				    dist[current] + res_cost[e] < dist[destination])
				{
					parent_node[destination] = current;
					parent_edge[destination] = e;
					dist[destination] = dist[current] + res_cost[e];
					int next_rear = (rear + 1) % MAX_NODES;
					if (next_rear == front)
						continue;
					queue[rear] = destination;
					rear = next_rear;
					if (destination == end)
						break;
				}
			}
		}

		// Plus de chemin disponible
		if (parent_node[end] == -1)
			break;

		// Construire le chemin
		path_t *p = &paths[path_count];
		p->length = 0;
	
		int nodes_path[MAX_NODES];
		int nodes_len = 0;
		int x = end;

		while (x != start)
		{
			nodes_path[nodes_len++] = x;
			x = parent_node[x];
		}
		nodes_path[nodes_len++] = start;

		// Collecter les arêtes
		int edges_on_path[MAX_NODES];
		int edges_count = 0;
		x = end;

		while (x != start)
		{
			edges_on_path[edges_count++] = parent_edge[x];
			x = parent_node[x];
		}
		
		// Convertir les nœuds en salles
		for (int j = nodes_len - 1; j >= 0; j--)
		{
			int node = nodes_path[j];
			uint16_t room = (uint16_t)(node / 2);

			if (p->length == 0 || p->path[p->length - 1] != room)
				p->path[p->length++] = room;
		}

		// Consommer 1 unité de flux sur ce chemin
		for (int j = 0; j < edges_count; j++)
		{
			int e = edges_on_path[j];
			res_cap[res_rev[e]] -= 1;  // Diminuer le flux utilisé
			res_cap[e] += 1;             // Restaurer la capacité directe
		}

		path_count++;
	}

	return path_count > 0;
}

// Copie les chemins trouvés vers cached_paths pour la simulation
// Exclut Start (les fourmis commencent déjà là) mais garde End
static void cache_paths_for_simulation(const lem_in_parser_t *parser)
{
	for (size_t i = 0; i < path_count; i++)
	{
		size_t write_idx = 0;
		for (size_t j = 0; j < paths[i].length; j++)
		{
			// Sauter Start (les fourmis y sont déjà)
			if (paths[i].path[j] != parser->start_room_id)
			{
				cached_paths[i][write_idx++] = paths[i].path[j];
			}
		}
		cached_path_lengths[i] = write_idx;
	}
}

// Trie les chemins par longueur croissante (pour l'optimisation)
static void sort_paths_by_length(void)
{
	// Tri à bulles simple (path_count est généralement petit)
	for (size_t i = 0; i < path_count; i++)
	{
		for (size_t j = 0; j < path_count - i - 1; j++)
		{
			if (cached_path_lengths[j] > cached_path_lengths[j + 1])
			{
				// Sauvegarder les longueurs AVANT l'échange
				size_t len_j = cached_path_lengths[j];
				size_t len_j1 = cached_path_lengths[j + 1];
				
				// Échanger les longueurs
				size_t temp_len = len_j;
				cached_path_lengths[j] = len_j1;
				cached_path_lengths[j + 1] = temp_len;
				
				// Échanger les chemins
				path_t temp_path = paths[j];
				paths[j] = paths[j + 1];
				paths[j + 1] = temp_path;
				
				// Échanger les chemins cachés (utiliser les longueurs sauvegardées)
				uint16_t temp_cached[MAX_ROOMS];
				
				// Sauvegarder j (avec sa longueur originale)
				for (size_t k = 0; k < len_j; k++)
					temp_cached[k] = cached_paths[j][k];
				
				// Copier j+1 vers j (avec sa longueur originale)
				for (size_t k = 0; k < len_j1; k++)
					cached_paths[j][k] = cached_paths[j + 1][k];
				
				// Copier temp vers j+1 (avec la longueur originale de j)
				for (size_t k = 0; k < len_j; k++)
					cached_paths[j + 1][k] = temp_cached[k];
			}
		}
	}
}

// Fonction helper pour tester une combinaison de chemins
// test_mask: bitmask indiquant quels chemins utiliser (bit i = 1 si chemin i est utilisé)
static void test_combination_internal(const lem_in_parser_t *parser, uint32_t test_mask, 
                                      size_t *best_count_ref, int *best_time_ref)
{
	uint16_t remaining = parser->ant_count;
	static int test_ants[MAX_PATHS];
	
	// Initialiser les compteurs pour les chemins utilisés
	for (size_t i = 0; i < path_count; i++)
		test_ants[i] = 0;

	// Distribuer les fourmis sur les chemins utilisés
	while (remaining > 0)
	{
		int best_path = -1;
		int best_t = 2147483647;

		for (size_t i = 0; i < path_count; i++)
		{
			// Vérifier si ce chemin est utilisé (bit i du mask)
			if (!(test_mask & (1U << i)))
				continue;
			
			int t = (int)cached_path_lengths[i] + test_ants[i];
			if (t < best_t)
			{
				best_t = t;
				best_path = i;
			}
		}
		
		if (best_path == -1)
			break;
			
		test_ants[best_path]++;
		remaining--;
	}

	// Calculer le temps total avec cette configuration
	int max_time = 0;
	for (size_t i = 0; i < path_count; i++)
	{
		if (test_ants[i] > 0)
		{
			int time = (int)cached_path_lengths[i] + test_ants[i] - 1;
			if (time > max_time)
				max_time = time;
		}
	}

	// Garder la meilleure configuration
	if (max_time < *best_time_ref)
	{
		*best_time_ref = max_time;
		// Compter le nombre de chemins utilisés
		size_t count = 0;
		for (size_t i = 0; i < path_count; i++)
		{
			if (test_mask & (1U << i))
				count++;
		}
		*best_count_ref = count;
	}
}

// Filtre les chemins pour ne garder que ceux qui améliorent la solution
// Un chemin trop long peut ralentir l'ensemble : on le supprime si nécessaire
static void optimize_path_selection(const lem_in_parser_t *parser)
{
	if (path_count <= 1)
		return;
	
	// Trier les chemins par longueur croissante (important pour l'optimisation)
	sort_paths_by_length();

	// Tester chaque combinaison de chemins pour trouver l'optimale
	size_t best_count = path_count;
	int best_time = 2147483647;

	// Stratégie 1: Tester les combinaisons consécutives (1, 2, 3, ... chemins)
	for (size_t test_count = 1; test_count <= path_count; test_count++)
	{
		// Mask pour les test_count premiers chemins
		uint32_t mask = (1U << test_count) - 1;
		test_combination_internal(parser, mask, &best_count, &best_time);
	}

	// Stratégie 2: Si on a peu de chemins, tester toutes les combinaisons possibles
	// Cela permet de trouver la meilleure combinaison même si elle n'est pas consécutive
	if (path_count <= 8) // 2^8 = 256 combinaisons max (raisonnable)
	{
		// Tester toutes les combinaisons non-vides
		for (uint32_t mask = 1; mask < (1U << path_count); mask++)
		{
			test_combination_internal(parser, mask, &best_count, &best_time);
		}
	}

	// Ajuster path_count au nombre optimal
	// Note: on garde les best_count premiers chemins car ils sont déjà triés par longueur
	path_count = best_count;
}

static bool find_superposition_paths(const lem_in_parser_t *parser)
{
	residual_graph_builder(parser);
	int flow = min_cost_maxflow(parser);
	if (flow <= 0)
		return false;
	if (!extract_paths_from_flow(parser, flow))
		return false;
	cache_paths_for_simulation(parser);
	
	// Optimiser la sélection des chemins (enlever les chemins trop longs)
	optimize_path_selection(parser);
	
	return true;
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

bool valid_path(const lem_in_parser_t *parser)
{
	if (!build_neighbors_table(parser))
		return print_error(ERR_MEMORY, "neighbors table allocation");

	uint16_t queue[MAX_ROOMS];
	bool visited_bfs[MAX_ROOMS] = {false};
	size_t front = 0, rear = 0;

	queue[rear] = parser->start_room_id;
	rear = (rear + 1) % MAX_ROOMS;
	visited_bfs[parser->start_room_id] = true;

	while (front != rear)
	{
		uint16_t current = queue[front];
		front = (front + 1) % MAX_ROOMS;

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
					size_t next_rear = (rear + 1) % MAX_ROOMS;
					if (next_rear == front)
						continue;
                    queue[rear] = node->neighbor;
					rear = next_rear;
                }
                node = node->next;
            }
        }
    }
    
    free_neighbors_table();
    return print_error(ERR_NO_PATH, NULL);
}


static void calculate_ants_per_path(const lem_in_parser_t *parser)
{
	for (size_t i = 0; i < path_count; i++)
		ants_per_path[i] = 0;

	if (path_count == 0)
		return;

	uint16_t remaining_ants = parser->ant_count;

	while (remaining_ants > 0)
	{
		int best_path = 0;
		int best_time = (int)cached_path_lengths[0] + ants_per_path[0];

		for (size_t i = 1; i < path_count; i++)
		{
			int time_if_added = (int)cached_path_lengths[i] + ants_per_path[i];
			if (time_if_added < best_time)
			{
				best_time = time_if_added;
				best_path = i;
			}
		}

		ants_per_path[best_path]++;
		remaining_ants--;
	}
}

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

static void simulate_turn(const lem_in_parser_t *parser, int turn)
{
	(void)turn;
	bool first_move = true;

	for (uint16_t i = 0; i < parser->ant_count; i++)
	{
		if (ants[i].finished)
			continue; //ignore if already finished

		uint16_t path_id = 0;
		uint16_t ant_offset = 0; 
		for (size_t p = 0; p < path_count; p++)
		{
			//find the path for the ant
			if (ants[i].id <= ant_offset + ants_per_path[p])
			{
				path_id = p;
				break;
			}
			ant_offset += ants_per_path[p];
		}

		if (ants[i].path_index < cached_path_lengths[path_id])
		{
			uint16_t next_room = cached_paths[path_id][ants[i].path_index]; // Next room to move to

			bool room_available = true;
			if (next_room != parser->start_room_id && next_room != parser->end_room_id) 
			{
				for (uint16_t j = 0; j < parser->ant_count; j++)
				{
					if (j != i && !ants[j].finished && ants[j].current_room == next_room)
					{
						room_available = false;
						break;
					}
				}
			}

			if (room_available)
			{
				if (first_move)
				{
					ft_printf("L%d-%s", ants[i].id, parser->rooms[next_room].name); 
					first_move = false;
				}
				else
					ft_printf(" L%d-%s", ants[i].id, parser->rooms[next_room].name);

				ants[i].current_room = next_room;
				ants[i].path_index++;

				if (ants[i].current_room == parser->end_room_id)
				{
					ants[i].finished = true;
				}
			}
		}
	}

	if (!first_move)
		ft_printf("\n");
}

static int calculate_total_turns(const lem_in_parser_t *parser)
{
	(void)parser;
	int total_turns = 0;

	for (size_t i = 0; i < path_count; i++)
	{
		if (ants_per_path[i] > 0)
		{
			int time_for_path = cached_path_lengths[i] + ants_per_path[i] - 1;
			if (time_for_path > total_turns)
			{
				total_turns = time_for_path;
			}
		}
	}

	return total_turns;
}

static void simulate_all_turns(const lem_in_parser_t *parser)
{
	int total_turns = calculate_total_turns(parser);

	for (int turn = 1; turn <= total_turns; turn++)
	{
		simulate_turn(parser, turn);
	}

	ft_printf("# Nombre de lignes: %d\n", total_turns); // Récapitulatif final
}

bool start(lem_in_parser_t *parser)
{
    find_superposition_paths(parser); // Cherche un lot de chemins via superposition

	calculate_ants_per_path(parser);

	init_ants(parser);

	simulate_all_turns(parser);

	return true;
}
