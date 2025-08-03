#include "visualizer.h"

Map g_map;

SDL_Surface* screen = NULL;
TTF_Font* font = NULL;

int next_start = 0;
int next_end = 0;
int is_first_line = 1;

int current_turn = 0;
int max_turns = 0;
char* turn_lines[MAX_ACTIONS_PER_TURN];
int turn_line_count = 0;

int main(void)
{
    // error_checker();
    init_map();
    display_map();
    return (0);
}
