#include "lem_in.h"

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

	// Execute pathfinding algorithm
	if (!valid_path(parser) || !find_paths_optimized(parser))
	{
		parser_destroy(parser);
		return EXIT_FAILURE;
	}

	parser_destroy(parser);
	return EXIT_SUCCESS;
}
