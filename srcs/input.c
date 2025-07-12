#include "lem_in.h"

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
		return print_error(ERR_MEMORY, "input buffer");

	ssize_t bytes_read;
	while ((bytes_read = read(STDIN_FILENO, parser->input_buffer + size, capacity - size)) > 0)
	{
		size += bytes_read;

		// Need more space?
		if (size >= capacity - 1)
		{
			if (capacity >= MAX_INPUT_SIZE)
				return print_error(ERR_INPUT_READ, "input too large");

			capacity *= 2;
			if (capacity > MAX_INPUT_SIZE)
			{
				capacity = MAX_INPUT_SIZE;
			}

			char *new_buffer = realloc(parser->input_buffer, capacity);
			if (!new_buffer)
				return print_error(ERR_MEMORY, "input buffer resize");
			parser->input_buffer = new_buffer;
		}
	}

	if (bytes_read < 0)
		return print_error(ERR_INPUT_READ, strerror(errno));

	parser->input_buffer[size] = '\0';
	parser->input_size = size;

	return size > 0;
}
