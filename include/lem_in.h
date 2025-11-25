#ifndef LEM_IN_H
# define LEM_IN_H

# include <stddef.h>
# include <stdint.h>
# include <limits.h>
# include <errno.h>
# include "libft.h"

// ============================================================================
// CONSTANTS AND LIMITS
// ============================================================================

# define MAX_INPUT_SIZE (1 << 20) // 1MB
# define INVALID_ROOM_ID UINT16_MAX
# define HASH_SIZE 32768

# define MAX_ROOMS 20000
# define MAX_LINKS 200000

# define INCREASE 1
# define DECREASE -1

# define SUCCESS 0
# define FAILURE -1

# define FALSE 0
# define TRUE 1

// ============================================================================
// DATA STRUCTURES
// ============================================================================

typedef enum s_room_flags
{
	ROOM_NORMAL = 0,
	ROOM_START = 1,
	ROOM_END = 2,
} room_flags_t;

typedef struct s_room
{
	const char *name;	// points inside input buffer
	int32_t x, y;		// coordinates (can be negative)
	room_flags_t flags; // room type and state
	uint16_t id;		// unique room identifier
} room_t;

typedef struct s_link
{
	uint16_t from; // source room id
	uint16_t to;   // destination room id
} link_t;

typedef struct s_hash_entry
{
	const char *name;
	uint16_t room_id;
} hash_entry_t;



typedef struct s_edge
{
	size_t dest;
	size_t capacity;
	struct s_edge *next;
} t_edge;


typedef struct s_node
{
	t_edge *head;
	int index;
	room_flags_t flags;
	uint8_t bfs_marked;
	uint8_t enqueued;
	uint8_t enqueued_backward;
	char padding[7];
	char *name;
} t_node;

typedef struct s_graph
{
	t_node *nodes;
	size_t ants;
	size_t size;
	size_t start_room_id;
	size_t end_room_id;
	size_t paths_count;
	size_t old_output_lines;
} t_graph;

typedef struct s_bfs
{
	t_list *shortest_path;
	t_edge *neighbours;
	t_edge *neighbours2;
	ssize_t *queue;
	ssize_t *prev;
	size_t queue_front;
	size_t queue_rear;
	size_t queue_size;
	size_t queue_capacity;
	size_t node;
} t_bfs;

typedef struct s_paths
{
	t_list **array;
	size_t *ants_to_paths;
	size_t *n;
	size_t *len;
	size_t output_lines;
	uint8_t *available;
	t_edge *neighbours;
	t_edge *neighbours2;
} t_paths;

typedef struct
{
	
} t_options;

// ============================================================================
// ERROR HANDLING
// ============================================================================

typedef enum
{
	ERR_NONE = 0,
	ERR_MEMORY,
	ERR_INPUT_READ,
	ERR_EMPTY_INPUT,
	ERR_INVALID_ANT_COUNT,
	ERR_ANT_COUNT_ZERO,
	ERR_ANT_COUNT_OVERFLOW,
	ERR_ROOM_NAME_INVALID,
	ERR_ROOM_NAME_STARTS_L,
	ERR_ROOM_NAME_STARTS_HASH,
	ERR_ROOM_NAME_HAS_SPACE,
	ERR_ROOM_NAME_HAS_DASH,
	ERR_ROOM_DUPLICATE,
	ERR_ROOM_COORDINATES,
	ERR_MULTIPLE_START,
	ERR_MULTIPLE_END,
	ERR_LINK_INVALID,
	ERR_LINK_SELF,
	ERR_LINK_ROOM_NOT_FOUND,
	ERR_NO_START,
	ERR_NO_END,
	ERR_NO_ROOMS,
	ERR_INVALID_LINE,
	ERR_TOO_MANY_ROOMS,
	ERR_TOO_MANY_LINKS,
	ERR_NO_PATH
} error_code_t;

// ============================================================================
// MAIN PARSER STRUCTURE
// ============================================================================

typedef struct
{
	char *input_buffer;
	size_t input_size;

	room_t *rooms;
	link_t *links;
	hash_entry_t *hash_table;

	size_t room_count;
	size_t link_count;
	uint16_t start_room_id;
	uint16_t end_room_id;

	int32_t ant_count;
	bool has_start;
	bool has_end;
} lem_in_parser_t;

// ============================================================================
// FUNCTION PROTOTYPES
// ============================================================================

// Parser lifecycle
lem_in_parser_t *parser_create(void);
void *parser_destroy(lem_in_parser_t *parser);
bool parse_input(lem_in_parser_t *parser);

// Input handling
bool read_input(lem_in_parser_t *parser);

// Validation functions
bool validate_ant_count(const char *line, int32_t *count, error_code_t *error);
bool validate_room_name(const char *name, error_code_t *error);
bool validate_coordinates(const char *x_str, const char *y_str, error_code_t *error);

// Parsing functions
bool parse_room_line(lem_in_parser_t *parser, char *line, int next_flag);
bool parse_link_line(lem_in_parser_t *parser, char *line);
bool is_room_line(const char *line);

// Hash table
uint32_t hash_string(const char *str);
bool hash_add_room(lem_in_parser_t *parser, const char *name, uint16_t room_id);
int16_t hash_get_room_id(const lem_in_parser_t *parser, const char *name);

// Error handling
bool print_error(error_code_t code, const char *context);
const char *error_to_string(error_code_t code);

// Output
bool display_input(const lem_in_parser_t *parser);

// graph building functions
t_graph *graph_builder(const lem_in_parser_t *parser);
t_graph *create_graph(const lem_in_parser_t *parser);
int8_t create_edge(t_graph *graph, size_t src, size_t dest);

// cleaner functions
void free_graph(t_graph *graph);
void free_bfs(t_bfs *bfs);
void del_content(void *content);
t_paths *free_paths(t_paths *paths, t_graph *graph);

// bfs functions
t_bfs *bfs_initializer(t_graph *graph);
int8_t enqueue(size_t node, size_t neigh, t_graph *graph, t_bfs *bfs);
size_t dequeue(t_bfs *bfs);
int8_t direct_start_end(t_graph *graph);
void reset_marks(t_graph *graph, t_bfs *bfs);
void reset_marks_fail(t_graph *graph, t_bfs *bfs);
void update_capacity(t_graph *graph, t_bfs *bfs, int8_t order);
void capacity_changer(t_graph *graph, t_list *from, t_list *to, int8_t order);
size_t is_on_path(size_t node, t_list *path, t_graph *graph);
size_t find_path_index(t_list **path, t_list *aug_paths, t_graph *graph);
t_list *get_next_path(t_list *path, t_graph *graph);
t_list *add_node_to_paths(size_t *node, t_list **aug_paths);
t_list *rebuild_paths(t_graph *graph);

// paths finder functions
t_bfs *bfs(t_graph *graph, t_list *path);
int8_t is_source_neighbours(size_t node, t_graph *graph);
void skip_node(t_bfs *new_bfs, t_edge *neigh, t_graph *graph, t_list *path);
t_bfs *reconstruct_path(t_bfs *new_bfs, t_graph *graph);
void enqueue_node(t_bfs *new_bfs, t_graph *graph, t_edge *neigh, t_list *path);
size_t find_path_index(t_list **path, t_list *aug_paths, t_graph *graph);
t_list *bfs_and_compare(t_graph *graph, t_list *aug_paths, t_list **path);
t_list *first_bfs(t_graph *graph);
int8_t is_new_solution_better(t_list *aug_paths, t_graph *graph);
t_paths *find_solution(t_graph *graph, t_list *aug_paths);
t_list *find_paths(t_graph *graph);
int8_t is_solution_found(t_paths *paths, t_graph *graph);

// solver functions
int8_t solver(t_graph *graph, t_list *aug_paths);
// int8_t reset_availability(t_graph *graph, t_paths *paths, size_t *ants2paths);
void assign_ants_to_paths(t_graph *graph, t_paths *paths, size_t *tmp);
void display_lines(t_paths *paths, t_graph *graph);


// init functions
t_paths *init_paths(t_graph *graph, t_list *aug_paths);
t_paths *init_output(t_graph *graph, t_list *aug_paths);
void init_lines(t_paths *paths, t_graph *graph);

#endif // LEM_IN_H
