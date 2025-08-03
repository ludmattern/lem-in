#ifndef VISUALIZER_H
#define VISUALIZER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

#define MAX_ROOMS 256
#define MAX_NAME_LENGTH 100
#define MAX_CONNECTIONS 512
#define MAX_ANTS 512
#define MAX_ACTIONS_PER_TURN 1000

typedef struct {
    char name[MAX_NAME_LENGTH];
    int x, y;
    int is_start;
    int is_end;
} Room;

typedef struct {
    int from_room;
    int to_room;
} Connection;

typedef struct {
    int ant_id;
    int current_room;
    int target_room;
    float progress;
    int is_moving;
} Ant;

typedef struct {
    Room rooms[MAX_ROOMS];
    Connection connections[MAX_CONNECTIONS];
    Ant ants[MAX_ANTS];
    int room_count;
    int connection_count;
    int ant_count;
} Map;

// Function prototypes
void global_init(void);
int get_map_info(void);
int get_room(char* token);
int display_map(void);
int add_connection(char* name1, char* name2);
int add_room(char* name, int x, int y);
int get_connection(char* token);
int parse_ant_movement(char* line);
int init_sdl(void);
void init_map(void);
void draw_rooms(void);
void draw_connections(void);
void draw_ants(void);
void update_ant_animation(void);
void process_turn_movements(int turn);
void reset_ants_to_start(void);
int all_ants_stopped(void);
void error_checker(void);

// Global variables declarations
extern Map g_map;
extern SDL_Surface* screen;
extern TTF_Font* font;
extern int next_start;
extern int next_end;
extern int is_first_line;
extern int current_turn;
extern int max_turns;
extern char* turn_lines[1000];
extern int turn_line_count;

#endif
