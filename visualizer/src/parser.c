#include "visualizer.h"
#include <string.h>

void error_checker(void)
{
    char line[1024];
    //Utiliser GNL pour lire la ligne d'erreur
    //Si rien n'est lu, on continue
    if (read(2, line, sizeof(line)) > 0)
        exit(1);
}

int add_room(char* name, int x, int y)
{
    if (g_map.room_count > MAX_ROOMS)
        return -1;
    strcpy(g_map.rooms[g_map.room_count].name, name);
    g_map.rooms[g_map.room_count].x = x;
    g_map.rooms[g_map.room_count].y = y;
    g_map.rooms[g_map.room_count].is_start = 0;
    g_map.rooms[g_map.room_count].is_end = 0;
    if (next_start)
    {
        g_map.rooms[g_map.room_count].is_start = 1;
        next_start = 0;
    }
    if (next_end)
    {
        g_map.rooms[g_map.room_count].is_end = 1;
        next_end = 0;
    }
    return (0);
}

int get_room(char* token)
{
    char* name;
    int x, y;
    char* new_token = strtok(token, " \n");
    if (!new_token) return (-1);
    name = new_token;
    new_token = strtok(NULL, " \n");
    if (!new_token) return (-1);
    x = atoi(new_token);
    new_token = strtok(NULL, " \n");
    if (!new_token) return (-1);
    y = atoi(new_token);
    if (add_room(name, x, y) == -1)
    {
        printf("Error: Too many rooms\n");
        return (-1);
    }
    g_map.room_count++;
    return (0);
}

int get_connection(char* token)
{
    char* name1 = strtok(token, "-");
    char* name2 = strtok(NULL, "-");

    if (add_connection(name1, name2) == -1)
    {
        printf("Error: Too many connections\n");
        return (-1);
    }
    return (0);
}

int add_connection(char* name1, char* name2)
{
    if (g_map.connection_count > MAX_CONNECTIONS)
        return (-1);
    for (int i = 0; i < g_map.room_count; i++)
    {
        if (strcmp(g_map.rooms[i].name, name1) == 0)
        {
            g_map.connections[g_map.connection_count].from_room = i;
        }
    }
    for (int i = 0; i < g_map.room_count; i++)
    {
        if (strcmp(g_map.rooms[i].name, name2) == 0)
        {
            g_map.connections[g_map.connection_count].to_room = i;
        }
    }
    g_map.connection_count++;
    return (0);
}

int parse_ant_movement(char* line)
{
    if (turn_line_count < 1000) {
        turn_lines[turn_line_count] = strdup(line);
        turn_line_count++;
    }
    
    char* token = strtok(line, " ");
    while (token) {
        char* dash = strchr(token, '-');
        if (dash) {
            *dash = '\0';
            char* ant_id_str = token;
            char* room_name = dash + 1;
            
            int ant_id = atoi(ant_id_str + 1);
            
            int room_index = -1;
            for (int i = 0; i < g_map.room_count; i++) {
                if (strcmp(g_map.rooms[i].name, room_name) == 0) {
                    room_index = i;
                    break;
                }
            }
            
            if (room_index != -1) {
                if (ant_id > g_map.ant_count) {
                    g_map.ant_count = ant_id;
                }
                
                g_map.ants[ant_id - 1].ant_id = ant_id;
                g_map.ants[ant_id - 1].current_room = room_index;
                g_map.ants[ant_id - 1].target_room = room_index;
                g_map.ants[ant_id - 1].progress = 0.0;
                g_map.ants[ant_id - 1].is_moving = 0;
            }
        }
        token = strtok(NULL, " ");
    }
    return 0;
}

int get_map_info(void)
{
    char line[1024];
    while (fgets(line, sizeof(line), stdin))
    {
        char* token = strtok(line, "\n");
        if (!token)
            continue;
        if (is_first_line == 1)
        {
            printf("Number of ants: %s\n", token);
            is_first_line = 0;
            int ants_count = atoi(token);
            if (ants_count > MAX_ANTS)
            {
                printf("Error: Too many ants\n");
                return (-1);
            }
            g_map.ant_count = ants_count;
            continue;
        }
        if (token[0] == '#') 
        {
            if (strcmp(token, "##start") == 0)
                next_start = 1;
            else if (strcmp(token, "##end") == 0)
                next_end = 1;
            continue;
        }
        if (strchr(token, '-') != NULL && token[0] != 'L')
        {
            if (get_connection(token) == -1)
            {
                printf("Error: Failed to get connection\n");
                return (-1);
            }
            continue;
        }
        if (token[0] == 'L')
        {
            printf("Ant: %s\n", token);
            parse_ant_movement(token);
            continue;
        }
        if (strchr(token, '-') == NULL) {
            if (get_room(token) == -1)
            {
                printf("Error: Failed to get room\n");
                return (-1);
            }
        }
    }
    return (0);
}
