#include "lem_in.h"

int main(void)
{
	lem_in_parser_t *parser = parser_create();
	t_graph *graph;
	t_list *aug_paths;
	int status = EXIT_SUCCESS;

	if (!parser)
		return EXIT_FAILURE;

	if (!read_input(parser) || !parse_input(parser))
	{
		parser_destroy(parser);
		return EXIT_FAILURE;
	}
	
	graph = graph_builder(parser);
	if (!graph)
	{
		parser_destroy(parser);
		return EXIT_FAILURE;
	}
	
	if (is_valid_path(graph) == FALSE)
	{
		print_error(ERR_NO_PATH, NULL);
		free_graph(graph);
		parser_destroy(parser);
		return EXIT_FAILURE;
	}
	
	if (!display_input(parser))
		status = EXIT_FAILURE;
	
	aug_paths = find_paths(graph);
	if (!aug_paths)
		return EXIT_FAILURE;
	
	if (solver(graph, aug_paths) == FAILURE)
		return EXIT_FAILURE;
	
	ft_lstclear(&aug_paths, del_content);
	free_graph(graph);
	parser_destroy(parser);
	return (status);
}
