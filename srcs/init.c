#include "lem_in.h"

t_paths	*init_paths(t_graph *graph, t_list *aug_paths)
{
	t_list		*tmp;
	size_t		i;
	t_list		*curr;
	t_paths		*paths;

	if (!(paths = malloc(sizeof(t_paths))))
		return (NULL);
	if (!(paths->array = malloc(graph->paths_count * sizeof(t_list*))))
		return (free_paths(paths, graph));
	i = 0;
	while (i < graph->paths_count)
		paths->array[i++] = NULL;
	i = 0;
	curr = aug_paths;
	while (curr != NULL)
	{
        size_t *dup = malloc(sizeof(size_t));
        if (!dup)
            return NULL;
        *dup = *(size_t *)curr->content;
        if (!(tmp = ft_lstnew(dup)))
        {
            free(dup);
            return NULL;
        }
		ft_lstappend(&paths->array[i], tmp);
		curr = curr->next;
		if (curr != NULL && *(size_t *)curr->content == graph->start_room_id)
			i++;
	}
	return (paths);
}

t_paths	*init_output(t_graph *graph, t_list *aug_paths)
{
	t_paths	*paths;
	size_t	i;

	if (!(paths = init_paths(graph, aug_paths)))
		return (free_paths(paths, graph));
	if (!(paths->ants_to_paths = malloc(graph->ants * sizeof(size_t))))
		return (free_paths(paths, graph));
	if (graph->paths_count)
	{
		if (!(paths->n = malloc(graph->paths_count * sizeof(size_t))))
			return (free_paths(paths, graph));
		if (!(paths->len = malloc(graph->paths_count * sizeof(size_t))))
			return (free_paths(paths, graph));
	}
	i = 0;
	while (i < graph->paths_count)
	{
		paths->len[i] = ft_lstsize(paths->array[i]) - 1;
		paths->n[i++] = 0;
	}
	i = 0;
	while (i < graph->ants)
		paths->ants_to_paths[i++] = 0;
	paths->output_lines = 0;
	return (paths);
}


//verifie le nombre de lignes de sortie grace a la longueur de la plus courte chaine
void init_lines(t_paths *paths, t_graph *graph)
{
	size_t tmp = 0;

	paths->output_lines = ft_lstsize(paths->array[0]) - 1;
	for (size_t i = 0; i < graph->paths_count; i++)
	{
		tmp = ft_lstsize(paths->array[i]) - 1;
		if (tmp < paths->output_lines)
			paths->output_lines = tmp;
	}
}

