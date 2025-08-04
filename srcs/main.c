#include "lem_in.h"

int main(void)
{
	lem_in_parser_t *parser = parser_create();
	int status = EXIT_SUCCESS;

	if (!parser)
		return EXIT_FAILURE;

	if (!read_input(parser) || !parse_input(parser) || !valid_path(parser) || !find_paths(parser) || !display_input(parser))
		status = EXIT_FAILURE;

	parser_destroy(parser);
	return status;
}
