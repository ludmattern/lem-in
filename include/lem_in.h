#ifndef LEM_IN_H
# define LEM_IN_H

# include <stdint.h>
# include <limits.h>
# include <errno.h>
# include "libft.h"
# include "get_next_line.h"
# include "ft_printf.h"

// ============================================================================
// CONSTANTS AND LIMITS
// ============================================================================

# define MAX_ROOMS 20000
# define MAX_LINKS 200000
# define HASH_SIZE 32768
# define MAX_ANTS 10000
# define MAX_PATHS 15
# define MAX_INPUT_SIZE (1 << 20) // 1MB
# define INVALID_ROOM_ID UINT16_MAX
# define MAX_NODES (MAX_ROOMS * 2 + 8)
# define MAX_EDGES (MAX_LINKS * 4 + MAX_ROOMS * 2)

// ============================================================================
// DATA STRUCTURES
// ============================================================================

typedef enum
{
	ROOM_NORMAL = 0,
	ROOM_START = 1,
	ROOM_END = 2,
	ROOM_VISITED = 4
} room_flags_t;

typedef struct
{
	const char *name;	// points inside input buffer
	int32_t x, y;		// coordinates (can be negative)
	room_flags_t flags; // room type and state
	uint16_t id;		// unique room identifier
} room_t;

typedef struct
{
	uint16_t from; // source room id
	uint16_t to;   // destination room id
} link_t;

typedef struct
{
	const char *name;
	uint16_t room_id;
} hash_entry_t;

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

// Table de hachage pour les voisins (accès O(1) garanti)
typedef struct hash_node
{
	uint16_t neighbor;
	struct hash_node *next;
} hash_node_t;

typedef struct {
	uint16_t ant_id;
	uint16_t room_id;
} move_t;

typedef struct
{
	hash_node_t *buckets[256]; // 256 buckets pour une distribution optimale
	size_t count;
} hash_table_t;

// Structures optimisées
typedef struct
{
	uint16_t path[MAX_ROOMS];
	size_t length;
	int cost;
} path_t;

typedef struct
{
	uint16_t id;
    uint16_t path_id;
	uint16_t current_room;
	uint16_t path_index;
	uint16_t assigned_path;
	bool finished;
} ant_t;

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

// Pathfinding functions
bool is_valid_path(const lem_in_parser_t *parser);
bool start(lem_in_parser_t *parser);

#endif // LEM_IN_H
