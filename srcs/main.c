#include "lem_in.h"

int main(void)
{
	lem_in_parser_t *parser = parser_create();
	int status = EXIT_SUCCESS;

	if (!parser)
		return EXIT_FAILURE;

	if (!read_input(parser) || !parse_input(parser) || !display_input(parser))
		status = EXIT_FAILURE;
	t_graph *graph = graph_builder(parser);
	if (!graph)
		return EXIT_FAILURE;
	t_list *aug_paths = find_paths(graph);
	if (!aug_paths)
		return EXIT_FAILURE;
	if (solver(graph, aug_paths) == FAILURE)
		return EXIT_FAILURE;
	ft_lstclear(&aug_paths, del_content);
	return (status);
}
