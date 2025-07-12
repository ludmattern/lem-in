/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmattern <lmattern@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/11 15:36:24 by lmattern          #+#    #+#             */
/*   Updated: 2025/07/11 15:36:24 by lmattern         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "lem_in.h"

// ============================================================================
// PARSING HELPER FUNCTIONS - Modular approach
// ============================================================================

// Extract room name from line, returns pointer after name or NULL on error
// Sets name_out to NULL if empty name detected
static char *extract_room_name(char *line, char **name_out)
{
	if (!line || !name_out)
		return NULL;

	char *p = line;
	*name_out = p;

	// Check for empty name (line starts with space/tab)
	if (*p == ' ' || *p == '\t')
	{
		*name_out = NULL; // Signal empty name
		return NULL;
	}

	// Find end of room name
	while (*p && *p != ' ' && *p != '\t')
		p++;

	if (!*p)
		return NULL; // No space after name

	*p = '\0';	  // Null terminate name
	return p + 1; // Return pointer after name
}

// Extract coordinates from line, returns true on success
static bool extract_coordinates(char *p, char **x_out, char **y_out)
{
	if (!p || !x_out || !y_out)
		return false;

	// Skip whitespace before X
	while (*p == ' ' || *p == '\t')
		p++;

	// Extract X coordinate
	*x_out = p;
	while (*p && *p != ' ' && *p != '\t')
		p++;
	if (!*p)
		return false; // No space after X
	*p = '\0';
	p++;

	// Skip whitespace before Y
	while (*p == ' ' || *p == '\t')
		p++;

	// Extract Y coordinate
	*y_out = p;
	while (*p && *p != ' ' && *p != '\t' && *p != '\n' && *p != '\r')
		p++;

	// Null-terminate Y coordinate
	if (*p)
	{
		*p = '\0';
		p++;

		// Check for trailing characters
		while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
			p++;

		return *p == '\0'; // Should be end of line
	}

	return true; // End of string, that's fine
}

// Apply start/end flags to room
static bool apply_room_flags(lem_in_parser_t *parser, room_t *room, int next_flag)
{
	if (next_flag == 1) // ##start
	{
		if (parser->has_start)
		{
			print_error(ERR_MULTIPLE_START, NULL);
			return false;
		}
		room->flags = ROOM_START;
		parser->start_room_id = room->id;
		parser->has_start = true;
	}
	else if (next_flag == 2) // ##end
	{
		if (parser->has_end)
		{
			print_error(ERR_MULTIPLE_END, NULL);
			return false;
		}
		room->flags = ROOM_END;
		parser->end_room_id = room->id;
		parser->has_end = true;
	}
	return true;
}

// ============================================================================
// MAIN PARSING LOGIC - Refactored and modular
// ============================================================================

bool parse_room_line(lem_in_parser_t *parser, char *line, int next_flag)
{
	if (!parser || !line)
		return false;

	// Check room limit
	if (parser->room_count >= MAX_ROOMS)
	{
		print_error(ERR_TOO_MANY_ROOMS, NULL);
		return false;
	}

	// Extract room name
	char *name;
	char *rest = extract_room_name(line, &name);
	if (!rest)
	{
		print_error(!name ? ERR_ROOM_NAME_INVALID : ERR_INVALID_LINE,
					!name ? line : line);
		return false;
	}

	// Validate room name
	error_code_t error = ERR_NONE;
	if (!validate_room_name(name, &error))
	{
		print_error(error, name);
		return false;
	}

	// Check for duplicates
	if (hash_get_room_id(parser, name) >= 0)
	{
		print_error(ERR_ROOM_DUPLICATE, name);
		return false;
	}

	// Extract coordinates
	char *x_str, *y_str;
	if (!extract_coordinates(rest, &x_str, &y_str))
	{
		print_error(ERR_INVALID_LINE, line);
		return false;
	}

	// Validate coordinates
	if (!validate_coordinates(x_str, y_str, &error))
	{
		print_error(error, line);
		return false;
	}

	// Create room
	uint16_t room_id = (uint16_t)parser->room_count;
	room_t *room = &parser->rooms[parser->room_count];

	room->name = name;
	room->x = (int32_t)atoi(x_str);
	room->y = (int32_t)atoi(y_str);
	room->flags = ROOM_NORMAL;
	room->id = room_id;

	// Apply start/end flags
	if (!apply_room_flags(parser, room, next_flag))
		return false;

	// Add to hash table
	if (!hash_add_room(parser, room->name, room_id))
	{
		print_error(ERR_ROOM_DUPLICATE, name);
		return false;
	}

	parser->room_count++;
	return true;
}

bool parse_link_line(lem_in_parser_t *parser, char *line)
{
	if (!parser || !line)
		return false;

	char *dash = strchr(line, '-');
	if (!dash || dash == line || !dash[1])
	{
		print_error(ERR_LINK_INVALID, line);
		return false;
	}

	if (parser->link_count >= MAX_LINKS)
	{
		print_error(ERR_TOO_MANY_LINKS, NULL);
		return false;
	}

	// Split the line at the dash
	*dash = '\0';
	char *room1_name = line;
	char *room2_name = dash + 1;

	// Trim whitespace
	char *p = room1_name + strlen(room1_name) - 1;
	while (p >= room1_name && (*p == ' ' || *p == '\t'))
		*p-- = '\0';

	while (*room2_name == ' ' || *room2_name == '\t')
		room2_name++;
	p = room2_name + strlen(room2_name) - 1;
	while (p >= room2_name && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r'))
		*p-- = '\0';

	// Get room IDs
	int16_t room1_id = hash_get_room_id(parser, room1_name);
	int16_t room2_id = hash_get_room_id(parser, room2_name);

	if (room1_id < 0 || room2_id < 0)
	{
		print_error(ERR_LINK_ROOM_NOT_FOUND, line);
		return false;
	}

	if (room1_id == room2_id)
	{
		print_error(ERR_LINK_SELF, line);
		return false;
	}

	// Add the link
	link_t *link = &parser->links[parser->link_count];
	link->from = (uint16_t)room1_id;
	link->to = (uint16_t)room2_id;

	parser->link_count++;
	return true;
}

bool parser_parse_input(lem_in_parser_t *parser)
{
	if (!parser || !parser->input_buffer)
		return false;

	char *line = parser->input_buffer;
	char *end = parser->input_buffer + parser->input_size;
	int next_flag = 0; // 0 = normal, 1 = ##start, 2 = ##end
	bool found_ant_count = false;

	while (line < end)
	{
		// Find line boundaries
		char *line_end = line;
		while (line_end < end && *line_end != '\n' && *line_end != '\r')
		{
			line_end++;
		}

		// Null terminate current line
		char saved_char = '\0';
		if (line_end < end)
		{
			saved_char = *line_end;
			*line_end = '\0';
		}

		// Skip empty lines
		if (!*line)
		{
			if (!found_ant_count)
			{
				print_error(ERR_EMPTY_INPUT, NULL);
				return false;
			}
			// Empty line can terminate parsing according to subject
			break;
		}

		// Handle comments and commands
		if (line[0] == '#')
		{
			if (line[1] == '#')
			{
				if (strcmp(line, "##start") == 0)
				{
					if (next_flag == 1) // Already have a pending ##start
					{
						print_error(ERR_MULTIPLE_START, NULL);
						return false;
					}
					next_flag = 1;
				}
				else if (strcmp(line, "##end") == 0)
				{
					if (next_flag == 2) // Already have a pending ##end
					{
						print_error(ERR_MULTIPLE_END, NULL);
						return false;
					}
					next_flag = 2;
				}
				else
				{
					next_flag = 0; // Unknown command, ignore
				}
			}
			// Regular comments are ignored
		}
		// First non-comment line must be ant count
		else if (!found_ant_count)
		{
			error_code_t ant_error = ERR_NONE;
			if (!validate_ant_count(line, &parser->ant_count, &ant_error))
			{
				print_error(ant_error, line);
				return false;
			}
			found_ant_count = true;
		}
		// Parse room lines
		else if (is_room_line(line))
		{
			if (!parse_room_line(parser, line, next_flag))
			{
				return false;
			}
			next_flag = 0; // Reset flag after use
		}
		// Parse link lines
		else if (strchr(line, '-'))
		{
			if (!parse_link_line(parser, line))
			{
				return false;
			}
		}
		// Invalid line format
		else
		{
			print_error(ERR_INVALID_LINE, line);
			return false;
		}

		// Move to next line
		if (line_end >= end)
			break;

		line = line_end + 1;
		// Skip additional line ending characters
		if (line < end && saved_char == '\n' && *line == '\r')
			line++;
		else if (line < end && saved_char == '\r' && *line == '\n')
			line++;
	}

	// Final validation
	if (!found_ant_count)
	{
		print_error(ERR_EMPTY_INPUT, NULL);
		return false;
	}
	if (!parser->has_start)
	{
		print_error(ERR_NO_START, NULL);
		return false;
	}
	if (!parser->has_end)
	{
		print_error(ERR_NO_END, NULL);
		return false;
	}
	if (parser->room_count == 0)
	{
		print_error(ERR_NO_ROOMS, NULL);
		return false;
	}

	return true;
}

// ============================================================================
// MAIN ENTRY POINT
// ============================================================================

int main(void)
{
	lem_in_parser_t *parser = parser_create();
	if (!parser)
	{
		return EXIT_FAILURE;
	}

	// Read input
	if (!read_stdin_to_buffer(parser))
	{
		parser_destroy(parser);
		return EXIT_FAILURE;
	}

	// Parse input
	if (!parser_parse_input(parser))
	{
		parser_destroy(parser);
		return EXIT_FAILURE;
	}

	// Output the original input (required by subject)
	if (!output_original_input(parser))
	{
		parser_destroy(parser);
		return EXIT_FAILURE;
	}

#ifdef DEBUG
	// Debug output is only available in debug builds
	debug_print_parser_state(parser);
#endif

	// TODO: Implement pathfinding algorithm here
	// For now, parsing is complete and successful

	parser_destroy(parser);
	return EXIT_SUCCESS;
}
