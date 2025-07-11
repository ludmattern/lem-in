/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmattern <lmattern@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/11 15:36:31 by lmattern          #+#    #+#             */
/*   Updated: 2025/07/11 15:36:31 by lmattern         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "lem_in.h"

// ============================================================================
// PARSER LIFECYCLE - Modern resource management
// ============================================================================

lem_in_parser_t *parser_create(void)
{
	lem_in_parser_t *parser = calloc(1, sizeof(lem_in_parser_t));
	if (!parser)
	{
		print_error(ERR_MEMORY, "parser allocation");
		return NULL;
	}

	// Allocate rooms array
	parser->rooms = calloc(MAX_ROOMS, sizeof(room_t));
	if (!parser->rooms)
	{
		print_error(ERR_MEMORY, "rooms array");
		goto error_cleanup;
	}

	// Allocate links array
	parser->links = calloc(MAX_LINKS, sizeof(link_t));
	if (!parser->links)
	{
		print_error(ERR_MEMORY, "links array");
		goto error_cleanup;
	}

	// Allocate hash table
	parser->hash_table = calloc(HASH_SIZE, sizeof(hash_entry_t));
	if (!parser->hash_table)
	{
		print_error(ERR_MEMORY, "hash table");
		goto error_cleanup;
	}

	// Initialize state
	parser->start_room_id = INVALID_ROOM_ID;
	parser->end_room_id = INVALID_ROOM_ID;
	parser->ant_count = -1;
	parser->has_start = false;
	parser->has_end = false;

	return parser;

error_cleanup:
	parser_destroy(parser);
	return NULL;
}

void parser_destroy(lem_in_parser_t *parser)
{
	if (!parser)
		return;

	free(parser->input_buffer);
	free(parser->rooms);
	free(parser->links);
	free(parser->hash_table);
	free(parser);
}

// ============================================================================
// INPUT HANDLING - Modern and secure
// ============================================================================

bool read_stdin_to_buffer(lem_in_parser_t *parser)
{
	if (!parser)
		return false;

	size_t capacity = 4096; // Start smaller
	size_t size = 0;

	parser->input_buffer = malloc(capacity);
	if (!parser->input_buffer)
	{
		print_error(ERR_MEMORY, "input buffer");
		return false;
	}

	ssize_t bytes_read;
	while ((bytes_read = read(STDIN_FILENO, parser->input_buffer + size, capacity - size)) > 0)
	{
		size += bytes_read;

		// Need more space?
		if (size >= capacity - 1)
		{
			if (capacity >= MAX_INPUT_SIZE)
			{
				print_error(ERR_INPUT_READ, "input too large");
				return false;
			}

			capacity *= 2;
			if (capacity > MAX_INPUT_SIZE)
			{
				capacity = MAX_INPUT_SIZE;
			}

			char *new_buffer = realloc(parser->input_buffer, capacity);
			if (!new_buffer)
			{
				print_error(ERR_MEMORY, "input buffer resize");
				return false;
			}
			parser->input_buffer = new_buffer;
		}
	}

	if (bytes_read < 0)
	{
		print_error(ERR_INPUT_READ, strerror(errno));
		return false;
	}

	parser->input_buffer[size] = '\0';
	parser->input_size = size;

	return size > 0;
}
