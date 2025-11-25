#include "lem_in.h"

lem_in_parser_t *parser_create(void)
{
	lem_in_parser_t *parser = ft_calloc(1, sizeof(lem_in_parser_t));
	if (!parser)
	{
		print_error(ERR_MEMORY, "parser allocation");
		return NULL;
	}

	parser->rooms = ft_calloc(MAX_ROOMS, sizeof(room_t));
	if (!parser->rooms)
	{
		print_error(ERR_MEMORY, "rooms array");
		return parser_destroy(parser);
	}

	parser->links = ft_calloc(MAX_LINKS, sizeof(link_t));
	if (!parser->links)
	{
		print_error(ERR_MEMORY, "links array");
		return parser_destroy(parser);
	}

	parser->hash_table = ft_calloc(HASH_SIZE, sizeof(hash_entry_t));
	if (!parser->hash_table)
	{
		print_error(ERR_MEMORY, "hash table");
		return parser_destroy(parser);
	}

	parser->start_room_id = INVALID_ROOM_ID;
	parser->end_room_id = INVALID_ROOM_ID;
	parser->ant_count = -1;
	parser->has_start = false;
	parser->has_end = false;

	return parser;
}

void *parser_destroy(lem_in_parser_t *parser)
{
	if (!parser)
		return NULL;

	t_list *current = parser->file_content;
	while (current)
	{
		t_list *next = current->next;
		free(current->content);
		free(current);
		current = next;
	}

	free(parser->input_buffer);
	free(parser->rooms);
	free(parser->links);
	free(parser->hash_table);
	free(parser);
	return NULL;
}

static char *extract_line(char **line_ptr, char *end, char *saved_char)
{
	char *line = *line_ptr;
	char *line_end = line;

	while (line_end < end && *line_end != '\n' && *line_end != '\r')
		line_end++;

	*saved_char = '\0';
	if (line_end < end)
	{
		*saved_char = *line_end;
		*line_end = '\0';
	}

	if (line_end >= end)
	{
		*line_ptr = end;
	}
	else
	{
		*line_ptr = line_end + 1;
		if (*line_ptr < end && *saved_char == '\n' && **line_ptr == '\r')
			(*line_ptr)++;
		else if (*line_ptr < end && *saved_char == '\r' && **line_ptr == '\n')
			(*line_ptr)++;
	}

	return line;
}

static bool handle_command(char *line, int *next_flag)
{
	if (line[1] != '#')
		return true;

	if (ft_strncmp(line, "##start", 8) == 0)
	{
		if (*next_flag == 1)
			return print_error(ERR_MULTIPLE_START, NULL);
		*next_flag = 1;
	}
	else if (ft_strncmp(line, "##end", 6) == 0)
	{
		if (*next_flag == 2)
			return print_error(ERR_MULTIPLE_END, NULL);
		*next_flag = 2;
	}
	else
		*next_flag = 0;

	return true;
}

static bool process_line(lem_in_parser_t *parser, char *line, int *next_flag, bool *found_ant_count)
{
	char *line_copy = ft_strdup(line);
	if (!line_copy)
		return print_error(ERR_MEMORY, "line duplication");

	t_list *new_node = ft_lstnew(line_copy);
	if (!new_node)
	{
		free(line_copy);
		return print_error(ERR_MEMORY, "file content list");
	}
	ft_lstadd_back(&parser->file_content, new_node);

	if (line[0] == '#')
		return handle_command(line, next_flag);

	if (!*found_ant_count)
	{
		error_code_t ant_error = ERR_NONE;
		if (!validate_ant_count(line, &parser->ant_count, &ant_error))
			return print_error(ant_error, line);
		*found_ant_count = true;
		return true;
	}

	if (is_room_line(line))
	{
		if (!parse_room_line(parser, line, *next_flag))
			return false;
		*next_flag = 0;
		return true;
	}

	if (ft_strchr(line, '-'))
		return parse_link_line(parser, line);

	return print_error(ERR_INVALID_LINE, line);
}

static bool validate_final_state(lem_in_parser_t *parser, bool found_ant_count)
{
	if (!found_ant_count)
		return print_error(ERR_EMPTY_INPUT, NULL);
	if (!parser->has_start)
		return print_error(ERR_NO_START, NULL);
	if (!parser->has_end)
		return print_error(ERR_NO_END, NULL);
	if (parser->room_count == 0)
		return print_error(ERR_NO_ROOMS, NULL);
	return true;
}

bool parse_input(lem_in_parser_t *parser)
{
	if (!parser || !parser->input_buffer)
		return false;

	char *line = parser->input_buffer;
	char *end = parser->input_buffer + parser->input_size;
	int next_flag = 0; // 0 = normal, 1 = ##start, 2 = ##end
	bool found_ant_count = false;

	while (line < end)
	{
		char saved_char = '\0';

		char *current_line = extract_line(&line, end, &saved_char);

		if (!*current_line)
		{
			if (!found_ant_count)
				return print_error(ERR_EMPTY_INPUT, NULL);
			break;
		}

		if (!process_line(parser, current_line, &next_flag, &found_ant_count))
			return false;
	}

	if (!validate_final_state(parser, found_ant_count))
		return false;

	return true;
}
