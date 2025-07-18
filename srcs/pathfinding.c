#include "lem_in.h"

// Global variables
static int distance[MAX_ROOMS];
static uint16_t parent[MAX_ROOMS];
static bool visited[MAX_ROOMS];
static bool used_rooms[MAX_ROOMS];
static path_t paths[MAX_PATHS];
static size_t path_count = 0;
static ant_t ants[MAX_ANTS];
static uint16_t ants_per_path[MAX_PATHS];
static hash_table_t neighbors[MAX_ROOMS];


// Cache to avoid recalculating paths
static bool path_cache_valid = false;
static uint16_t cached_paths[MAX_PATHS][MAX_ROOMS];
static size_t cached_path_lengths[MAX_PATHS];

// Hash function
static inline uint8_t hash_function(uint16_t value) 
{
    return (uint8_t)(value & 0xFF);
}

// Add neighbor to hash table
static inline void add_neighbor(uint16_t room, uint16_t neighbor) 
{
    uint8_t hash = hash_function(neighbor);
    hash_node_t *node = malloc(sizeof(hash_node_t));
    node->neighbor = neighbor;
    node->next = neighbors[room].buckets[hash];
    neighbors[room].buckets[hash] = node;
    neighbors[room].count++;
}

// Build neighbors hash table
static void build_neighbors_table(const lem_in_parser_t *parser) 
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
        
        add_neighbor(from, to);
        add_neighbor(to, from);
    }
}

// Free hash table memory
static void free_neighbors_table(void) 
{
    for (size_t i = 0; i < MAX_ROOMS; i++) 
	{
        for (int j = 0; j < 256; j++) {
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
bool valid_path(const lem_in_parser_t *parser) 
{
    build_neighbors_table(parser);
    
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
    return false;
}

static uint16_t find_min_distance_room(void) 
{
    uint16_t min_room = INVALID_ROOM_ID;
    int min_distance = INT_MAX;
    
    for (uint16_t i = 0; i < MAX_ROOMS; i++) {
        if (!visited[i] && distance[i] < min_distance) {
            min_distance = distance[i];
            min_room = i;
        }
    }
    
    return min_room;
}

static bool dijkstra_simplified(const lem_in_parser_t *parser) 
{
    for (size_t i = 0; i < MAX_ROOMS; i++) {
        distance[i] = INT_MAX;
        parent[i] = INVALID_ROOM_ID;
        visited[i] = false;
    }
    distance[parser->start_room_id] = 0;
    
    while (true) {
        uint16_t current = find_min_distance_room();
        
        if (current == INVALID_ROOM_ID) break;
        if (current == parser->end_room_id) break;
        
        visited[current] = true;
        
        for (int bucket = 0; bucket < 256; bucket++) {
            hash_node_t *node = neighbors[current].buckets[bucket];
            while (node) {
                uint16_t neighbor = node->neighbor;
                
                if (!visited[neighbor] && !used_rooms[neighbor]) {
                    int new_distance = distance[current] + 1;
                    if (new_distance < distance[neighbor]) {
                        distance[neighbor] = new_distance;
                        parent[neighbor] = current;
                    }
                }
                node = node->next;
            }
        }
    }
    
    return distance[parser->end_room_id] != INT_MAX;
}

// Save path
static void save_path(const lem_in_parser_t *parser) {
    if (path_count >= MAX_PATHS) return;
    
    path_t *path = &paths[path_count];
    path->length = 0;
        
    uint16_t current = parser->end_room_id;
    while (current != INVALID_ROOM_ID) {
        path->path[path->length++] = current;
        if (current != parser->start_room_id && current != parser->end_room_id)
            used_rooms[current] = true;
        current = parent[current];
    }
    
    for (size_t i = 0; i < path->length / 2; i++) {
        uint16_t temp = path->path[i];
        path->path[i] = path->path[path->length - 1 - i];
        path->path[path->length - 1 - i] = temp;
    }
    
    cached_path_lengths[path_count] = path->length;
    for (size_t i = 0; i < path->length; i++) {
        cached_paths[path_count][i] = path->path[i];
    }
    
    path_count++;
}

// Find disjoint paths with strict limit
static bool find_disjoint_paths(const lem_in_parser_t *parser) {
    path_count = 0;
    path_cache_valid = false;
    
    for (size_t i = 0; i < MAX_ROOMS; i++) {
        used_rooms[i] = false;
    }
    used_rooms[parser->start_room_id] = true;
    used_rooms[parser->end_room_id] = false;
        
    size_t max_paths = 5;
    
    while (path_count < max_paths) {
        
        if (!dijkstra_simplified(parser)) {
            break;
        }
        
        save_path(parser);
        
        if (path_count >= 5) break;
    }
        
    return path_count > 0;
}

// Calculate ant distribution
static void calculate_ants_per_path(const lem_in_parser_t *parser) {
    (void)parser;
    
    for (size_t i = 0; i < path_count; i++) {
        ants_per_path[i] = 0;
    }
    
    if (path_count == 0) return;
        
    uint16_t remaining_ants = parser->ant_count;
    
    uint16_t ants_per_path_base = remaining_ants / path_count;
    uint16_t extra_ants = remaining_ants % path_count;
    
    for (size_t i = 0; i < path_count; i++) {
        ants_per_path[i] = ants_per_path_base;
        if (i < extra_ants) {
            ants_per_path[i]++;
        }
    }
}

// Initialize ants
static void init_ants(const lem_in_parser_t *parser) {
    uint16_t ant_id = 1;
    
    for (size_t path_idx = 0; path_idx < path_count; path_idx++) {
        for (uint16_t ant_count = 0; ant_count < ants_per_path[path_idx]; ant_count++) {
            if (ant_id > parser->ant_count) break;
            
            ants[ant_id - 1].id = ant_id;
            ants[ant_id - 1].current_room = parser->start_room_id;
            ants[ant_id - 1].path_index = 0;
            ants[ant_id - 1].finished = false;
            
            ant_id++;
        }
    }
}

// Simulate one turn
static void simulate_turn(const lem_in_parser_t *parser, int turn) {
    (void)turn;
    bool first_move = true;
    
    for (uint16_t i = 0; i < parser->ant_count; i++) {
        if (ants[i].finished) continue;
        
        uint16_t path_id = 0;
        uint16_t ant_offset = 0;
        for (size_t p = 0; p < path_count; p++) {
            if (ants[i].id <= ant_offset + ants_per_path[p]) {
                path_id = p;
                break;
            }
            ant_offset += ants_per_path[p];
        }
        
        if (ants[i].path_index < cached_path_lengths[path_id]) {
            uint16_t next_room = cached_paths[path_id][ants[i].path_index];
            
            bool room_available = true;
            if (next_room != parser->start_room_id && next_room != parser->end_room_id) {
                for (uint16_t j = 0; j < parser->ant_count; j++) {
                    if (j != i && !ants[j].finished && ants[j].current_room == next_room) {
                        room_available = false;
                        break;
                    }
                }
            }
            
            if (room_available) {
                if (first_move) {
                    printf("L%d-%s", ants[i].id, parser->rooms[next_room].name);
                    first_move = false;
                } else {
                    printf(" L%d-%s", ants[i].id, parser->rooms[next_room].name);
                }
                
                ants[i].current_room = next_room;
                ants[i].path_index++;
                
                if (next_room == parser->end_room_id) {
                    ants[i].finished = true;
                }
            }
        }
    }
    
    if (!first_move) {
        printf("\n");
    }
}

// Calculate total turns
static int calculate_total_turns(const lem_in_parser_t *parser) {
    (void)parser;
    int total_turns = 0;
    
    for (size_t i = 0; i < path_count; i++) {
        if (ants_per_path[i] > 0) {
            int time_for_path = paths[i].length + ants_per_path[i] - 1;
            if (time_for_path > total_turns) {
                total_turns = time_for_path;
            }
        }
    }
    
    return total_turns;
}

// Simulate all turns
static void simulate_all_turns(const lem_in_parser_t *parser) {
    int total_turns = calculate_total_turns(parser);
    
    for (int turn = 1; turn <= total_turns; turn++) {
        simulate_turn(parser, turn);
    }
}

// Main function
bool find_paths_optimized(lem_in_parser_t *parser) {
    build_neighbors_table(parser);
    
    if (!find_disjoint_paths(parser)) {
        free_neighbors_table();
		return false;
    }
    
    calculate_ants_per_path(parser);
    
    init_ants(parser);
    
    simulate_all_turns(parser);
    
    free_neighbors_table();

	return true;
}

// Compatibility function
bool find_paths(lem_in_parser_t *parser) {
    return find_paths_optimized(parser);
}
