#include "lem_in.h"

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
			return print_error(ERR_MULTIPLE_START, NULL);
		room->flags = ROOM_START;
		parser->start_room_id = room->id;
		parser->has_start = true;
	}
	else if (next_flag == 2) // ##end
	{
		if (parser->has_end)
			return print_error(ERR_MULTIPLE_END, NULL);
		room->flags = ROOM_END;
		parser->end_room_id = room->id;
		parser->has_end = true;
	}
	return true;
}

// ============================================================================
// MAIN PARSING LOGIC - Specialized room and link parsing
// ============================================================================

bool parse_room_line(lem_in_parser_t *parser, char *line, int next_flag)
{
	if (!parser || !line)
		return false;

	// Check room limit
	if (parser->room_count >= MAX_ROOMS)
		return print_error(ERR_TOO_MANY_ROOMS, NULL);

	// Extract room name
	char *name;
	char *rest = extract_room_name(line, &name);
	if (!rest)
		return print_error(!name ? ERR_ROOM_NAME_INVALID : ERR_INVALID_LINE, !name ? line : line);

	// Validate room name
	error_code_t error = ERR_NONE;
	if (!validate_room_name(name, &error))
		return print_error(error, name);

	// Check for duplicates
	if (hash_get_room_id(parser, name) >= 0)
		return print_error(ERR_ROOM_DUPLICATE, name);

	// Extract coordinates
	char *x_str, *y_str;
	if (!extract_coordinates(rest, &x_str, &y_str))
		return print_error(ERR_INVALID_LINE, line);

	// Validate coordinates
	if (!validate_coordinates(x_str, y_str, &error))
		return print_error(error, line);

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
		return print_error(ERR_ROOM_DUPLICATE, name);

	parser->room_count++;
	return true;
}

bool parse_link_line(lem_in_parser_t *parser, char *line)
{
	if (!parser || !line)
		return false;

	char *dash = strchr(line, '-');
	if (!dash || dash == line || !dash[1])
		return print_error(ERR_LINK_INVALID, line);

	if (parser->link_count >= MAX_LINKS)
		return print_error(ERR_TOO_MANY_LINKS, NULL);

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
		return print_error(ERR_LINK_ROOM_NOT_FOUND, line);

	if (room1_id == room2_id)
		return print_error(ERR_LINK_SELF, line);

	// Add the link
	link_t *link = &parser->links[parser->link_count];
	link->from = (uint16_t)room1_id;
	link->to = (uint16_t)room2_id;

	parser->link_count++;
	return true;
}
