#include "lem_in.h"

bool read_input(lem_in_parser_t *parser)
{
	if (!parser)
		return false;

	size_t capacity = 4096;
	size_t size = 0;

	parser->input_buffer = malloc(capacity);
	if (!parser->input_buffer)
		return print_error(ERR_MEMORY, "input buffer");

	ssize_t bytes_read;
	while ((bytes_read = read(STDIN_FILENO, parser->input_buffer + size, capacity - size)) > 0)
	{
		size += bytes_read;

		if (size >= capacity - 1)
		{
			if (capacity >= MAX_INPUT_SIZE)
				return print_error(ERR_INPUT_READ, "input too large");

			capacity *= 2;
			if (capacity > MAX_INPUT_SIZE)
			{
				capacity = MAX_INPUT_SIZE;
			}
			char *new_buffer = ft_realloc(parser->input_buffer, capacity / 2, capacity);
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
