#include "lem_in.h"

// Plus besoin de all_moved, count_paths_occupied, are_all_ants_launched pour l'affichage simple

static void	display_first_move(t_list **ants_positions, t_paths *paths,
	t_graph *graph, size_t i, int *first)
{
	if (paths->available[paths->ants_to_paths[i]] == TRUE)
	{
		if (ants_positions[i] != NULL)
			ants_positions[i] = ants_positions[i]->next;
		if (ants_positions[i] != NULL && ants_positions[i]->next != NULL)
			paths->available[paths->ants_to_paths[i]] = FALSE;
		paths->n[paths->ants_to_paths[i]]--;
		if (ants_positions[i] != NULL)
		{
			if (!*first)
				ft_printf(" ");
			ft_printf("L%zu-%s", i + 1,
				graph->nodes[*(size_t *)ants_positions[i]->content].name);
			*first = 0;
		}
	}
}

static void display_moves(t_list **ants_positions, t_paths *paths, t_graph *graph, size_t i, int *first)
{
	if (ants_positions[i] == paths->array[paths->ants_to_paths[i]]->next && paths->n[paths->ants_to_paths[i]] > 0)
		paths->available[paths->ants_to_paths[i]] = TRUE;
	if (ants_positions[i] != NULL)
		ants_positions[i] = ants_positions[i]->next;
	if (ants_positions[i] != NULL)
	{
		if (!*first)
			ft_printf(" ");
		ft_printf("L%zu-%s", i + 1,
			graph->nodes[*(size_t *)ants_positions[i]->content].name);
		*first = 0;
	}
}

static void display_laps(t_paths *paths, t_graph *graph, t_list **ants_positions)
{
	size_t	i;
	int		first;

	first = 1;
	i = 0;
	while (i < graph->ants)
	{
		if (ants_positions[i] == paths->array[paths->ants_to_paths[i]])
			display_first_move(ants_positions, paths, graph, i, &first);
		else if (ants_positions[i] != NULL && ants_positions[i]->next != NULL)
			display_moves(ants_positions, paths, graph, i, &first);
		else if (ants_positions[i] != NULL && ants_positions[i]->next == NULL)
			ants_positions[i] = ants_positions[i]->next;
		i++;
	}
	ft_printf("\n");
}

void display_lines(t_paths *paths, t_graph *graph)
{
	t_list *ants_positions[graph->ants];
	size_t lap = 0;

	for (size_t i = 0; i < graph->ants; i++)
	{
		ants_positions[i] = paths->array[paths->ants_to_paths[i]];
	}
	while(lap++ < paths->output_lines)
	{
		display_laps(paths, graph, ants_positions);
	}

	#if DEBUG
		ft_printf("# Number of lines: %zu\n", paths->output_lines);
	#endif
}