#include "lem_in.h"
#include <stddef.h>

int8_t is_solution_found(t_paths *paths, t_graph *graph)
{
    size_t i = 0, sum = 0;
    while (i < graph->paths_count)
    {
        if (paths->output_lines < paths->len[i] - 1)
            paths->n[i] = 0;
        else if (paths->len[i] == 1)
            paths->n[i] = graph->ants;
        else
            paths->n[i] = paths->output_lines - paths->len[i] + 1;
        i++;
    }
    i = 0;
    while (i < graph->paths_count)
        sum += paths->n[i++];
    if (sum >= graph->ants)
        return TRUE;
    return FALSE;
}

t_paths *find_solution(t_graph *graph, t_list *aug_paths)
{
    t_paths *paths;
    size_t i = 0;

    if ((paths = init_output(graph, aug_paths)) == NULL)
        return NULL;

    init_lines(paths, graph);
    while (is_solution_found(paths, graph) == FALSE)
    {
        i = 0;
        while (i < graph->paths_count)
            paths->n[i++] = 0;
        paths->output_lines++;
    }
    if (graph->paths_count && !(paths->available = malloc(sizeof(int8_t) * graph->paths_count)))
    {
        free_paths(paths, graph);
        return NULL;
    }
    return paths;
}

static int8_t reset_availability(t_graph *graph, t_paths *paths, size_t *ants_to_paths)
{
    for (size_t i = 0; i < graph->paths_count; i++)
    {
        paths->available[i] = ants_to_paths[i] > 0 ? TRUE : FALSE;
    }
    return (SUCCESS);
}

static size_t update_index(t_graph *graph, t_paths *paths, size_t *ants_to_paths, size_t j)
{
    if (j == graph->paths_count - 1 && paths->available[j] == FALSE)
    {
        reset_availability(graph, paths, ants_to_paths);
        j = 0;
    }
    else
        j++;
    return j;
}

static void update_n(t_graph *graph, t_paths *paths, size_t *tmp)
{
    for (size_t i = 0; i < graph->paths_count; i++)
    {
        paths->n[i] = paths->n[i] - tmp[i];
    }
    reset_availability(graph, paths, paths->n);
}

static void all_paths_used(t_graph *graph, t_paths *paths, size_t *tmp)
{
    for (size_t i = 0; i < graph->paths_count; i++)
    {
        if (paths->n[i] > 0 && paths->available[i] == TRUE)
            return ;   
    }
    reset_availability(graph, paths, tmp);
}   

void assign_ants_to_paths(t_graph *graph, t_paths *paths, size_t *tmp)
{
    size_t j;

    for (size_t i = 0; i < graph->ants; i++)
    {
        all_paths_used(graph, paths, tmp);
        j = 0;
        while (j < graph->paths_count)
        {
            if (tmp[j] == 0)
                paths->available[j] = FALSE;
            if (paths->available[j] == TRUE && tmp[j] > 0)
            {
                paths->ants_to_paths[i] = j;
                tmp[j]--;
                paths->available[j] = FALSE;
                break ;
            }
            j = update_index(graph, paths, tmp, j);
        }
    }
    update_n(graph, paths, tmp);
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
    display_lines(paths, graph);
    free_paths(paths, graph);
    return SUCCESS;
}