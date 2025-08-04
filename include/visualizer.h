#ifndef VISUALIZER_H
#define VISUALIZER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include "libft.h"
#include "get_next_line.h"
#include "ft_printf.h"

#define MAX_NAME_LENGTH 100
#define MAX_CONNECTIONS 512
#define MAX_ACTIONS_PER_TURN 1000

typedef struct
{
	char name[MAX_NAME_LENGTH];
	int x, y;
	int is_start;
	int is_end;
} Room;

typedef struct
{
	int from_room;
	int to_room;
} Connection;

typedef struct
{
	int ant_id;
	int current_room;
	int target_room;
	float progress;
	int is_moving;
} Ant;

typedef struct
{
	Room rooms[20000]; // Use same limit as lem_in.h
	Connection connections[MAX_CONNECTIONS];
	Ant ants[10000]; // Use same limit as lem_in.h
	int room_count;
	int connection_count;
	int ant_count;
} Map;

// Visualizer context structure to replace global variables
typedef struct
{
	Map map;
	SDL_Surface *screen;
	TTF_Font *font;
	int current_turn;
	int max_turns;
	char *turn_lines[1000];
	int turn_line_count;
	int next_start;
	int next_end;
	int is_first_line;
} visualizer_context_t;

// Forward declaration - lem_in_parser_t will be defined when lem_in.h is included
struct lem_in_parser_s;

// Function prototypes - now take context as parameter
int convert_parser_to_visualizer(const struct lem_in_parser_s *parser, visualizer_context_t *ctx);
int init_sdl(visualizer_context_t *ctx);
void init_map(visualizer_context_t *ctx);
void draw_rooms(visualizer_context_t *ctx);
void draw_connections(visualizer_context_t *ctx);
void draw_ants(visualizer_context_t *ctx);
void update_ant_animation(visualizer_context_t *ctx);
void process_turn_movements(visualizer_context_t *ctx, int turn);
void reset_ants_to_start(visualizer_context_t *ctx);
int all_ants_stopped(visualizer_context_t *ctx);
int display_map(visualizer_context_t *ctx);
int get_map_info(void);

// Parser functions
void set_visualizer_context(visualizer_context_t *context);
void error_checker(void);
int add_room(char *name, int x, int y);
int get_room(char *token);
int get_connection(char *token);
int add_connection(char *name1, char *name2);
int parse_ant_movement(char *line);

#endif
