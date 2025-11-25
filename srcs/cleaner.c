#include "lem_in.h"

void del_content(void *content)
{
     free(content);
}

static void free_edges(t_edge *head)
{
    t_edge *current;
    t_edge *next;

    current = head;
    while (current != NULL)
    {
        next = current->next;
        free(current);
        current = next;
    }
}

void free_graph(t_graph *graph)
{
    if (graph == NULL)
        return;
    for (size_t i = 0; i < graph->size; i++)
    {
        free(graph->nodes[i].name);
        free_edges(graph->nodes[i].head);
    }
    free(graph->nodes);
    free(graph);
}

void free_bfs(t_bfs *bfs)
{
    free(bfs->prev);
    free(bfs->queue);
    free(bfs);
}

t_paths	*free_paths(t_paths *paths, t_graph *graph)
{
	size_t	i;

	if (paths == NULL)
		return (NULL);
	if (paths->array)
	{
		i = 0;
		while (i < graph->paths_count)
		{
			if (paths->array[i])
				ft_lstclear(&paths->array[i], del_content);
			i++;
		}
		free(paths->array);
	}
	paths->n ? free(paths->n) : 0;
	paths->len ? free(paths->len) : 0;
	paths->available ? free(paths->available) : 0;
	paths->ants_to_paths ? free(paths->ants_to_paths) : 0;
	free(paths);
	return (NULL);
}