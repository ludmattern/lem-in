#include "lem_in.h"
#include <stddef.h>

t_paths *find_solution(t_graph *graph, t_list *aug_paths)
{
    //a implementer
}

int8_t reset_availability(t_graph *graph, t_paths *paths, size_t *ants2paths)
{
    //a implementer
}

void assign_ants_to_paths(t_graph *graph, t_paths *paths, size_t *tmp)
{
    //a implementer
}

void print_lines(t_paths *paths, t_graph *graph)
{
    //a implementer
}
int8_t solver(t_graph *graph, t_list *aug_paths)
{
    t_paths *paths;
    size_t i = 0, tmp[graph->paths_count];

    if ((paths = find_solution(graph, aug_paths)) == NULL)
        return FAILURE;

    while (i < graph->paths_count)
    {
        tmp[i] = paths->n[i];
        i++;
    }
    reset_availability(graph, paths, paths->n);
    assign_ants_to_paths(graph, paths, tmp);
    print_lines(paths, graph);
    free_paths(paths, graph);
    free_graph(graph);
    return SUCCESS;
}