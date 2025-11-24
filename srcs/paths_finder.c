#include "lem_in.h"
#include "libft.h"
#include <stddef.h>
#include <string.h>
#include <sys/_types/_ssize_t.h>
#include <unistd.h>

/*===========================================================================
 *                               PATHS FINDER
 *=========================================================================== */



 /*---------------------------------------------------------------------------
 *                             helper functions
 *--------------------------------------------------------------------------- */

//Verifie si un chemin est meilleur que le precedent
int8_t is_new_solution_better(t_list *aug_paths, t_graph *graph)
{
    t_paths *paths;
    size_t new_output_lines;

    if ((paths = find_solution(graph, aug_paths)) == NULL)
        return FAILURE;
    new_output_lines = paths->output_lines;
    free_paths(paths, graph);
    if (new_output_lines < graph->old_output_lines)
    {
        graph->old_output_lines = new_output_lines;
        return TRUE;
    }
    return FALSE;
}

 //Decide si on enqueue ou si ou skip un noeud
void enqueue_node(t_bfs *new_bfs, t_graph *graph, t_edge *neigh, t_list *path)
{
    if (graph->nodes[neigh->dest].enqueued == FALSE)
    {
        if (is_on_path(new_bfs->node, path, graph) == FALSE && new_bfs->node != graph->end_room_id) 
        {    
            if (is_on_path(neigh->dest, path, graph) == TRUE && is_source_neighbours(neigh->dest, graph) == FALSE)
                skip_node(new_bfs, neigh, graph, path);
            else if (graph->nodes[neigh->dest].bfs_marked == FALSE)
                enqueue(new_bfs->node, neigh->dest, graph, new_bfs);
        }
    }
    else if 
    (is_on_path(new_bfs->node, path, graph) == TRUE && ((neigh->capacity == 2 && neigh->dest != graph->start_room_id) 
    || 
    (neigh->capacity == 1 && graph->nodes[neigh->dest].bfs_marked == FALSE)))
        enqueue(new_bfs->node, neigh->dest, graph, new_bfs);
}   
    
//Reconstruire un chemin trouve de end a start pour le ranger dans shortest_path
t_bfs *reconstruct_path(t_bfs *new_bfs, t_graph *graph)
{
    t_list *tmp;
    ssize_t i = 0;

    i = graph->end_room_id;

    while (i != -1)
    {
        size_t *dup = malloc(sizeof(size_t));
        if (dup == NULL)
            return (NULL);
        *dup = i;
        if ((tmp = ft_lstnew(dup)) == NULL)
        {
            free(dup);
            return (NULL);
        }
        ft_lstadd_front(&new_bfs->shortest_path, tmp);
        i = new_bfs->prev[i];
    }
    if (!new_bfs->shortest_path || *(size_t *)new_bfs->shortest_path->content != graph->start_room_id)
    {
        reset_marks_fail(graph, new_bfs);
        ft_lstclear(&new_bfs->shortest_path, del_content);
        free_bfs(new_bfs);
        return (NULL);
    }
    reset_marks(graph, new_bfs);
    return (new_bfs);
 }


//Savoir si un noeu est voisin de start
int8_t is_source_neighbours(size_t node, t_graph *graph)
{
    for (t_edge *current_edge = graph->nodes[node].head; current_edge != NULL; current_edge = current_edge->next)
    {
        if (current_edge->dest == graph->start_room_id)
            return TRUE;
    }
    return FALSE;
}

//Permet de skip un noeud dans bfs modified pour permettre dutiliser un noeud deja utilise
void skip_node(t_bfs *new_bfs, t_edge *neigh, t_graph *graph, t_list *path)
{
    new_bfs->prev[neigh->dest] = new_bfs->node;
    graph->nodes[neigh->dest].bfs_marked = TRUE;
    graph->nodes[neigh->dest].enqueued = TRUE;

    for (t_edge *neigh2 = graph->nodes[neigh->dest].head; neigh2 != NULL; neigh2 = neigh2->next)
    {
        if (neigh2->capacity == 2 && is_on_path(neigh2->dest, path, graph) == TRUE && neigh2->dest != graph->start_room_id)
        {
            enqueue(neigh->dest, neigh2->dest, graph, new_bfs);
            graph->nodes[neigh2->dest].enqueued_backward = TRUE;
        }
    }
}

/*---------------------------------------------------------------------------
*                             differents calls to bfs
*--------------------------------------------------------------------------- */

t_list *first_bfs(t_graph *graph)
{
    t_list *aug_paths;
    t_bfs *new_bfs;
    t_paths *paths;

    aug_paths = NULL;
    if ((new_bfs = bfs(graph, NULL)) == NULL)
        return (NULL);

    ft_lstappend(&aug_paths, new_bfs->shortest_path);
    update_capacity(graph, new_bfs, INCREASE);
    if ((paths = find_solution(graph, aug_paths)) == NULL)
        return (NULL);
    graph->old_output_lines = paths->output_lines;
    free_bfs(new_bfs);
    free_paths(paths, graph);
    return (aug_paths);
}

t_list *bfs_and_compare(t_graph *graph, t_list *aug_paths, t_list **path)
{
    t_bfs *new_bfs; 
    size_t path_pos;

    path_pos = compute_path_pos(path, aug_paths, graph);
    new_bfs = NULL;

    if ((new_bfs = bfs(graph, *path)) == NULL)
        return (aug_paths);
    update_capacity(graph, new_bfs, INCREASE);
    ft_lstclear(&aug_paths, del_content);
    aug_paths = rebuild_paths(graph);
    if (is_new_solution_better(aug_paths, graph) == FALSE)
    {
        update_capacity(graph, new_bfs, DECREASE);
        ft_lstclear(&aug_paths, del_content);
        aug_paths = rebuild_paths(graph);
        *path = aug_paths;
        while (path_pos-- > 0)
            *path = get_next_path(*path, graph);
    }
    ft_lstclear(&new_bfs->shortest_path, del_content);
    free_bfs(new_bfs);
    return (aug_paths);

}

 /*---------------------------------------------------------------------------
*                             basic function bfs
 *--------------------------------------------------------------------------- */


 //Fonction principale du bfs

t_bfs *bfs(t_graph *graph, t_list *path)
{
    t_bfs *new_bfs = bfs_initializer(graph);
    t_edge *neigh = NULL;

    while (new_bfs->queue_size > 0)
    {
        new_bfs->node = dequeue(new_bfs);
        neigh = graph->nodes[new_bfs->node].head;

        while (neigh != NULL)
        {
            enqueue_node(new_bfs, graph, neigh, path);
            neigh = neigh->next;
        }
    }
    return reconstruct_path(new_bfs, graph);
}

/*---------------------------------------------------------------------------
*                     Main function of the paths finder
 *--------------------------------------------------------------------------- */
 
t_list *find_paths(t_graph *graph)
{
    t_list *path, *aug_paths;
    size_t prev_paths_count;

    if ((aug_paths = first_bfs(graph)) == NULL)
        return NULL;

    path = aug_paths;
    while (path != NULL)
    {
        prev_paths_count = graph->paths_count;
        aug_paths = bfs_and_compare(graph, aug_paths, &path);
        if (prev_paths_count == graph->paths_count)
        {
            if ((path = get_next_path(path, graph)) == NULL)
                return aug_paths;
        }
        else
            path = aug_paths;
    }
    return (aug_paths);
}