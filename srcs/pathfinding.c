#include "lem_in.h"
#include <stdint.h>


// List of paths found (each path is a sequence of room identifiers)
static path_t paths[MAX_PATHS];
static size_t path_count = 0;

// All the ants, their current position, etc.
static ant_t ants[MAX_ANTS];

// How many ants are sent on each path
static uint16_t ants_per_path[MAX_PATHS];

// Neighbors of each room (fast access structure built from links)
static hash_table_t neighbors[MAX_ROOMS];

// Copy of the paths as room indices, for the simulation
static uint16_t cached_paths[MAX_PATHS][MAX_ROOMS];
static size_t cached_path_lengths[MAX_PATHS];

static int res_head[MAX_NODES];         
// For each node: index of the first outgoing edge (linked list in arrays)
static int res_to[MAX_EDGES];           // For each edge: to which node it goes
static int res_cap[MAX_EDGES];          // For each edge: how many "passages" remain (capacity)
static int res_cost[MAX_EDGES];        
static int res_next_e[MAX_EDGES];       // For each edge: index of the next edge in the list
static int res_rev[MAX_EDGES];          // For each edge: index of its inverse edge (return)
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

		res_add_edge(node_in(r), node_out(r), cap, 1);
	}

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

	while (path_count < (size_t)flow && path_count < MAX_PATHS)
	{
		int front = 0, rear = 0;

		// Initialize : find the minimum cost path in the used flow
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

		// Modified BFS : find the edges where the reverse flow > 0
		while (front != rear && parent_node[end] == -1)
		{
			int current = queue[front];
			front = (front + 1) % MAX_NODES;
			for (int e = res_head[current]; e != -1; e = res_next_e[e])
			{
				int destination = res_to[e];
				int rev = res_rev[e];
				
				// If the reverse flow > 0, it means we pushed flow here
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

		// No more path available
		if (parent_node[end] == -1)
			break;

		// Build the path
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

		// Collect the edges
		int edges_on_path[MAX_NODES];
		int edges_count = 0;
		x = end;

		while (x != start)
		{
			edges_on_path[edges_count++] = parent_edge[x];
			x = parent_node[x];
		}
		
		// Convert the nodes to rooms
		for (int j = nodes_len - 1; j >= 0; j--)
		{
			int node = nodes_path[j];
			uint16_t room = (uint16_t)(node / 2);

			if (p->length == 0 || p->path[p->length - 1] != room)
				p->path[p->length++] = room;
		}

		// Consume 1 unit of flow on this path
		for (int j = 0; j < edges_count; j++)
		{
			int e = edges_on_path[j];
			res_cap[res_rev[e]] -= 1;
			res_cap[e] += 1;
		}

		path_count++;
	}

	return path_count > 0;
}


static void cache_paths_for_simulation(const lem_in_parser_t *parser)
{
	for (size_t i = 0; i < path_count; i++)
	{
		size_t write_idx = 0;
		for (size_t j = 0; j < paths[i].length; j++)
		{
			if (paths[i].path[j] != parser->start_room_id)
			{
				cached_paths[i][write_idx++] = paths[i].path[j];
			}
		}
		cached_path_lengths[i] = write_idx;
	}
}

static void sort_paths_by_length(void)
{
	for (size_t i = 0; i < path_count; i++)
	{
		for (size_t j = 0; j < path_count - i - 1; j++)
		{
			if (cached_path_lengths[j] > cached_path_lengths[j + 1])
			{
				size_t len_j = cached_path_lengths[j];
				size_t len_j1 = cached_path_lengths[j + 1];
				
				size_t temp_len = len_j;
				cached_path_lengths[j] = len_j1;
				cached_path_lengths[j + 1] = temp_len;
				
				path_t temp_path = paths[j];
				paths[j] = paths[j + 1];
				paths[j + 1] = temp_path;
				
				uint16_t temp_cached[MAX_ROOMS];
				
				for (size_t k = 0; k < len_j; k++)
					temp_cached[k] = cached_paths[j][k];
				
				for (size_t k = 0; k < len_j1; k++)
					cached_paths[j][k] = cached_paths[j + 1][k];
				
				for (size_t k = 0; k < len_j; k++)
					cached_paths[j + 1][k] = temp_cached[k];
			}
		}
	}
}

static void test_combination_internal(const lem_in_parser_t *parser, uint32_t test_mask, size_t *best_count_ref, int *best_time_ref)
{
	uint16_t remaining = parser->ant_count;
	static int test_ants[MAX_PATHS];
	
	// Initialize the counters for the used paths
	for (size_t i = 0; i < path_count; i++)
		test_ants[i] = 0;

	// Distribute the ants on the used paths
	while (remaining > 0)
	{
		int best_path = -1;
		int best_t = 2147483647;

		for (size_t i = 0; i < path_count; i++)
		{
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

	// count the total time with this configuration
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

	// Keep the best configuration
	if (max_time < *best_time_ref)
	{
		*best_time_ref = max_time;
		// Count the number of used paths
		size_t count = 0;
		for (size_t i = 0; i < path_count; i++)
		{
			if (test_mask & (1U << i))
				count++;
		}
		*best_count_ref = count;
	}
}

static void optimize_path_selection(const lem_in_parser_t *parser)
{
	if (path_count <= 1)
		return;
	
	sort_paths_by_length();

	size_t best_count = path_count;
	int best_time = 2147483647;

	for (size_t test_count = 1; test_count <= path_count; test_count++)
	{
		uint32_t mask = (1U << test_count) - 1;
		test_combination_internal(parser, mask, &best_count, &best_time);
	}

	// If we have few paths, test all possible combinations
	if (path_count <= 8)
	{
		for (uint32_t mask = 1; mask < (1U << path_count); mask++)
		{
			test_combination_internal(parser, mask, &best_count, &best_time);
		}
	}
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
	
	optimize_path_selection(parser);
	
	return true;
}

static inline uint8_t hash_function(uint16_t value)
{
	return (uint8_t)(value & 0xFF);
}

static inline bool add_neighbor(uint16_t room, uint16_t neighbor)
{
	uint8_t hash = hash_function(neighbor);
	hash_node_t *node = malloc(sizeof(hash_node_t));
	if (!node)
		return false;
	node->neighbor = neighbor;
	node->next = neighbors[room].buckets[hash];
	neighbors[room].buckets[hash] = node;
	neighbors[room].count++;
	return true;
}

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

	ft_printf("# Number of lines: %d\n", total_turns);
}

bool start(lem_in_parser_t *parser)
{
    find_superposition_paths(parser);

	calculate_ants_per_path(parser);

	init_ants(parser);

	simulate_all_turns(parser);

	return true;
}
