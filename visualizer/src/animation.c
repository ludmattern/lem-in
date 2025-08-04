#include "visualizer.h"
#include <libft.h>

void update_ant_animation(void)
{
    for (int i = 0; i < g_map.ant_count; i++) {
        Ant* ant = &g_map.ants[i];
        if (ant->is_moving) {
            ant->progress += 0.05;
            if (ant->progress >= 1.0) {
                ant->current_room = ant->target_room;
                ant->progress = 0.0;
                ant->is_moving = 0;
            }
        }
    }
}

void process_turn_movements(int turn)
{
    if (turn >= turn_line_count) return;
    
    char* line = turn_lines[turn];
    if (!line) return;
    
    // Parse the movements of this turn using ft_split
    char** tokens = ft_split(line, ' ');
    if (!tokens) return;
    
    for (int j = 0; tokens[j] != NULL; j++) {
        char* token = tokens[j];
        char* dash = strchr(token, '-');
        if (dash) {
            *dash = '\0';
            char* ant_id_str = token;
            char* room_name = dash + 1;
            
            int ant_id = ft_atoi(ant_id_str + 1);
            
            // search the index of the target room
            int target_room_index = -1;
            for (int i = 0; i < g_map.room_count; i++) {
                if (strcmp(g_map.rooms[i].name, room_name) == 0) {
                    target_room_index = i;
                    break;
                }
            }
            // move the ant to the target room
            if (target_room_index != -1 && ant_id <= g_map.ant_count) {
                Ant* ant = &g_map.ants[ant_id - 1];
                ant->target_room = target_room_index;
                ant->is_moving = 1;
                ant->progress = 0.0;
            }
        }
    }
    
    // Free the allocated memory
    ft_free_double_array(tokens);
}

void reset_ants_to_start(void)
{
    // search the index of the start room
    int start_room_index = -1;
    for (int i = 0; i < g_map.room_count; i++) {
        if (g_map.rooms[i].is_start) {
            start_room_index = i;
            break;
        }
    }
    
    if (start_room_index != -1) {
        // reset all ants to the start room
        for (int i = 0; i < g_map.ant_count; i++) {
            g_map.ants[i].current_room = start_room_index;
            g_map.ants[i].target_room = start_room_index;
            g_map.ants[i].progress = 0.0;
            g_map.ants[i].is_moving = 0;
        }
    }
}

// check if all ants are stopped
int all_ants_stopped(void)
{
    for (int i = 0; i < g_map.ant_count; i++) {
        if (g_map.ants[i].is_moving)
            return 0;
    }
    return 1;
}
