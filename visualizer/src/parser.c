#include "visualizer.h"
#include <string.h>
#include <unistd.h>
#include <get_next_line.h>
#include <libft.h>

int add_room(char *name, int x, int y)
{
	if (g_map.room_count > MAX_ROOMS)
		return -1;
	ft_strcpy(g_map.rooms[g_map.room_count].name, name);
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

int get_room(char *token)
{
	char **parts = ft_split(token, ' ');
	if (!parts || !parts[0] || !parts[1] || !parts[2])
	{
		ft_printf("Error: Invalid room format\n");
		if (parts)
			ft_free_double_array(parts);
		return (-1);
	}

	char *name = parts[0];
	int x = ft_atoi(parts[1]);
	int y = ft_atoi(parts[2]);

	if (add_room(name, x, y) == -1)
	{
		ft_printf("Error: Too many rooms\n");
		ft_free_double_array(parts);
		return (-1);
	}

	ft_free_double_array(parts);
	g_map.room_count++;
	return (0);
}

int get_connection(char *token)
{
	char **parts = ft_split(token, '-');
	if (!parts || !parts[0] || !parts[1])
	{
		ft_printf("Error: Invalid connection format\n");
		if (parts)
			ft_free_double_array(parts);
		return (-1);
	}

	if (add_connection(parts[0], parts[1]) == -1)
	{
		ft_printf("Error: Too many connections\n");
		ft_free_double_array(parts);
		return (-1);
	}

	ft_free_double_array(parts);
	return (0);
}

int add_connection(char *name1, char *name2)
{
	if (g_map.connection_count > MAX_CONNECTIONS)
		return (-1);
	for (int i = 0; i < g_map.room_count; i++)
	{
		if (ft_strncmp(g_map.rooms[i].name, name1, ft_strlen(name1)) == 0 &&
			ft_strlen(g_map.rooms[i].name) == ft_strlen(name1))
		{
			g_map.connections[g_map.connection_count].from_room = i;
		}
	}
	for (int i = 0; i < g_map.room_count; i++)
	{
		if (ft_strncmp(g_map.rooms[i].name, name2, ft_strlen(name2)) == 0 &&
			ft_strlen(g_map.rooms[i].name) == ft_strlen(name2))
		{
			g_map.connections[g_map.connection_count].to_room = i;
		}
	}
	g_map.connection_count++;
	return (0);
}

int parse_ant_movement(char *line)
{
	if (turn_line_count < MAX_ACTIONS_PER_TURN - 1)
	{
		turn_lines[turn_line_count] = ft_strdup(line);
		if (!turn_lines[turn_line_count])
			return -1;
		turn_line_count++;
	}
	else
		ft_printf("ERROR: Too many actions per turn (%d)\n", MAX_ACTIONS_PER_TURN);

	char **tokens = ft_split(line, ' ');
	if (!tokens)
		return (-1);

	for (int j = 0; tokens[j] != NULL; j++)
	{
		char *token = tokens[j];
		char *dash = ft_strchr(token, '-');
		if (dash)
		{
			*dash = '\0';
			char *ant_id_str = token;
			char *room_name = dash + 1;

			int ant_id = ft_atoi(ant_id_str + 1);

			int room_index = -1;
			for (int i = 0; i < g_map.room_count; i++)
			{
				if (ft_strncmp(g_map.rooms[i].name, room_name, ft_strlen(room_name)) == 0 &&
					ft_strlen(g_map.rooms[i].name) == ft_strlen(room_name))
				{
					room_index = i;
					break;
				}
			}

			if (room_index != -1)
			{
				if (ant_id > g_map.ant_count)
				{
					g_map.ant_count = ant_id;
				}

				g_map.ants[ant_id - 1].ant_id = ant_id;
				g_map.ants[ant_id - 1].current_room = room_index;
				g_map.ants[ant_id - 1].target_room = room_index;
				g_map.ants[ant_id - 1].progress = 0.0;
				g_map.ants[ant_id - 1].is_moving = 0;
			}
		}
	}

	ft_free_double_array(tokens);
	return 0;
}

int get_map_info(void)
{
	char *line;
	while ((line = get_next_line(STDIN_FILENO)) != NULL)
	{
		if (ft_strncmp(line, "ERROR", 5) == 0)
		{
			ft_eprintf("%s", line);
			free(line);  // Libérer avant de nettoyer
			get_next_line(-1);
			exit(-1);
		}
		size_t len = ft_strlen(line);
		if (len > 0 && line[len - 1] == '\n')
			line[len - 1] = '\0';

		if (!line || line[0] == '\0')
		{
			free(line);
			continue;
		}
		if (is_first_line == 1)
		{
			is_first_line = 0;
			int ants_count = ft_atoi(line);
			if (ants_count > MAX_ANTS)
			{
				ft_printf("Error: Too many ants\n");
				free(line);
				get_next_line(-1);
				return (-1);
			}
			g_map.ant_count = ants_count;
			free(line);
			continue;
		}
		if (line[0] == '#')
		{
			if (ft_strncmp(line, "##start", 7) == 0)
				next_start = 1;
			else if (ft_strncmp(line, "##end", 5) == 0)
				next_end = 1;
			free(line);
			continue;
		}
		if (ft_strchr(line, '-') == NULL)
		{
			if (get_room(line) == -1)
			{
				ft_printf("Error: Failed to get room\n");
				free(line);
				get_next_line(-1);
				return (-1);
			}
		}
		else if (line[0] != 'L') // Ligne contient '-' mais pas mouvement de fourmi
		{
			// Vérifier si c'est vraiment une connexion ou une salle avec coordonnée négative
			char **parts = ft_split(line, ' ');
			bool is_room_with_negative_coord = false;
			
			// Si on a exactement 3 parties (nom x y), c'est probablement une salle
			if (parts && parts[0] && parts[1] && parts[2] && !parts[3])
			{
				// Vérifier si parts[1] ou parts[2] sont des nombres (possiblement négatifs)
				char *endptr;
				ft_strtol(parts[1], &endptr, 10); // Test si parts[1] est un nombre
				if (*endptr == '\0') // parts[1] est un nombre valide
				{
					ft_strtol(parts[2], &endptr, 10); // Test si parts[2] est un nombre
					if (*endptr == '\0') // parts[2] est aussi un nombre valide
					{
						is_room_with_negative_coord = true;
					}
				}
			}
			
			if (parts)
				ft_free_double_array(parts);
			
			if (is_room_with_negative_coord)
			{
				// C'est une salle avec coordonnée négative
				if (get_room(line) == -1)
				{
					ft_printf("Error: Failed to get room\n");
					free(line);
					get_next_line(-1);
					return (-1);
				}
			}
			else
			{
				// C'est une vraie connexion
				if (get_connection(line) == -1)
				{
					ft_printf("Error: Failed to get connection\n");
					free(line);
					get_next_line(-1);
					return (-1);
				}
			}
		}
		if (line[0] == 'L')
		{
			ft_printf("Ant: %s\n", line);
			if (parse_ant_movement(line) == -1)
			{
				free(line);
				get_next_line(-1);
				return (-1);
			}
			free(line);
			continue;
		}
		free(line);
	}
	// Nettoyer get_next_line à la fin
	get_next_line(-1);
	return (0);
}
