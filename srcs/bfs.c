#include "lem_in.h"
#include "libft.h"
#include <stdint.h>
#include <string.h>

/* ============================================================================
 *                               BFS FUNCTIONS
 * ============================================================================ */

/*---------------------------------------------------------------------------
 *                               paths building utilities
 *--------------------------------------------------------------------------- */
 
 //ajouter un noeud sur un chemin (a la fin de la liste chainee des chemins)
 t_list *add_node_to_paths(size_t *node, t_list **aug_paths)
 {
     t_list *tmp;
     size_t *dup;
 
     dup = malloc(sizeof(size_t));
     if (!dup)
         return NULL;
     *dup = *node;
     if (!(tmp = ft_lstnew(dup)))
     {
         free(dup);
         return NULL;
     }
     ft_lstappend(aug_paths, tmp);
     return tmp;
 }

//Reconstruire les chemins en suivant les noeuds ayant une capacite de 0
t_list *rebuild_paths(t_graph *graph)
{
    t_list *aug_paths;
    t_edge *neighbours;

    aug_paths = NULL;
    for (t_edge *from_start_edge = graph->nodes[graph->start_room_id].head; from_start_edge != NULL; from_start_edge = from_start_edge->next)
    {
        if (from_start_edge->capacity == 0)
        {
            if (add_node_to_paths(&graph->start_room_id, &aug_paths) == NULL)
                return NULL;
             if (add_node_to_paths(&from_start_edge->dest, &aug_paths) == NULL)
                return NULL;
            neighbours = graph->nodes[from_start_edge->dest].head;
            while (neighbours != NULL && neighbours->dest != graph->end_room_id)
            {
                if (neighbours->capacity == 0)
                {
                    if (add_node_to_paths(&neighbours->dest, &aug_paths) == NULL)
                        return (NULL);
                    neighbours = graph->nodes[neighbours->dest].head;
                }
                else
                    neighbours = neighbours->next;
                if (neighbours->dest == graph->end_room_id
                    && neighbours->capacity == 0
                    && add_node_to_paths(&neighbours->dest, &aug_paths) == NULL)
                    return (NULL);
            }
        }
    }
    return aug_paths;
}

 /*---------------------------------------------------------------------------
 *                               helper functions
 *--------------------------------------------------------------------------- */
 

 //verifier si un noeud est sur un chemin
 size_t is_on_path(size_t node, t_list *path, t_graph *graph)
 {
    for (t_list *curr = path; curr != NULL && *(size_t *)curr->content != graph->end_room_id; curr = curr->next)
    {
        if (*(size_t *)curr->content == node && node != graph->start_room_id && node != graph->end_room_id)
            return TRUE;
    }
    return FALSE;
}

// trouver l'index d'un chemin dans la liste des chemins
size_t find_path_index(t_list **path, t_list *aug_paths, t_graph *graph)
{
    size_t path_index = 0;
    
    for (t_list *curr = aug_paths; curr != *path && curr->next != NULL; curr = curr->next)
    {
        if (*(size_t *)curr->content == graph->start_room_id)
            path_index++;
    }
    return path_index;
}

// trouver le prochain chemin dans la liste des chemins
t_list *get_next_path(t_list *path, t_graph *graph)
{
    t_list *next_path = path;

    while (next_path->next != NULL && *(size_t *)next_path->next->content != graph->start_room_id)
        next_path = next_path->next;
    next_path = next_path->next;
    return next_path;
}

 /*---------------------------------------------------------------------------
 *                               capacity utilities
 *--------------------------------------------------------------------------- */

 //fonction helper pour update_capacity() juste en dessous
void capacity_changer(t_graph *graph, t_list *from, t_list *to, int8_t order)
{
    t_edge *neighbours;
    neighbours = graph->nodes[*(size_t *)from->content].head;

    while (neighbours->dest != *(size_t *)to->content)
        neighbours = neighbours->next;

    if (neighbours->dest == *(size_t *)to->content)
    {
        if (order == INCREASE)
            neighbours->capacity++;
        if (order == DECREASE)
            neighbours->capacity--;
    }
}

// changer la capacite des passages d'un chemin
void update_capacity(t_graph *graph, t_bfs *bfs, int8_t order)
{
    for (t_list *curr = bfs->shortest_path; curr->next != NULL; curr = curr->next)
    {
        if (order == INCREASE)
        {
            capacity_changer(graph, curr, curr->next, DECREASE);
            capacity_changer(graph, curr->next, curr, INCREASE);
        }
        else if (order == DECREASE)
        {
            capacity_changer(graph, curr, curr->next, INCREASE);
            capacity_changer(graph, curr->next, curr, DECREASE);
        }
    }
    graph->paths_count += order;
}

 /*---------------------------------------------------------------------------
 *                               bfs reset
 *--------------------------------------------------------------------------- */

// verifier si un noeud a un passage avec une capacite de 2
static int8_t find_neighbour(t_graph *graph, size_t i, int8_t found)
{
    t_edge *neighbours;

    neighbours = graph->nodes[i].head;
    while (neighbours)
    {
        if (neighbours->capacity == 2)
            return TRUE;
        neighbours = neighbours->next;
    }
    return found;
}

// verifier si le debut et la fin sont directement connectes
int8_t direct_start_end(t_graph *graph)
{
    t_edge *curr;

    curr = graph->nodes[graph->start_room_id].head;
    while (curr)
    {
        if (curr->dest == graph->end_room_id)
            return TRUE;
        curr = curr->next;
    }
    return FALSE;
}

// remettre les marques des noeuds
void reset_marks(t_graph *graph, t_bfs *bfs)
{
    int8_t found;
    t_list *curr;

    for (size_t i = 0; i < graph->size; i++)
    {
        found = FALSE;
        curr = bfs->shortest_path;
        while (curr)
        {
            if (i == *(size_t *)curr->content)
                found = TRUE;
            curr = curr->next;
        }
        found = find_neighbour(graph, i, found);
        if (found == FALSE || \
            (((graph->nodes[i].flags & ROOM_START) || (graph->nodes[i].flags & ROOM_END)) && direct_start_end(graph) == FALSE))
            graph->nodes[i].bfs_marked = FALSE;
            
        graph->nodes[i].enqueued = FALSE;
        graph->nodes[i].enqueued_backward = FALSE;
    }
}

// remettre les marques des noeuds en cas de fail (donc on ne touche pas a ceux appartenant a un autre chemin)
void reset_marks_fail(t_graph *graph, t_bfs *bfs)
{
    for (size_t i = 0; i < graph->size; i++)
    {
        for (ssize_t j = 0; bfs->queue[j] != -1; j++)
        {
            if (i == (size_t)bfs->queue[j])
                graph->nodes[i].bfs_marked = FALSE;
        }
        graph->nodes[i].enqueued = FALSE;
        graph->nodes[i].enqueued_backward = FALSE;
    }
}
 
 /*---------------------------------------------------------------------------
 *                               queue utilities
 *--------------------------------------------------------------------------- */

// Ajoute un element a la queue
int8_t enqueue(size_t node, size_t neigh, t_graph *graph, t_bfs *bfs)
{
    if (bfs->queue_size == bfs->queue_capacity)
        return FAILURE;

    bfs->queue_rear = bfs->queue_rear + 1;
    bfs->queue[bfs->queue_rear] = neigh;
    bfs->queue_size = bfs->queue_size + 1;

    bfs->prev[neigh] = node;
    graph->nodes[neigh].bfs_marked = TRUE;
    graph->nodes[neigh].enqueued = TRUE;

    return SUCCESS;
}

// Retourne l'element en tete de la queue et le retire de la queue
size_t dequeue(t_bfs *bfs)
{
    size_t index;

    if (bfs->queue_size == 0)
        return FAILURE;

    index = bfs->queue[bfs->queue_front];
    bfs->queue_front = bfs->queue_front + 1;
    bfs->queue_size = bfs->queue_size - 1;

    return index;
}

/*---------------------------------------------------------------------------
 *                               BFS INITIALIZER
 *--------------------------------------------------------------------------- */

 // initialiser la structure de bfs
t_bfs *bfs_initializer(t_graph *graph)
{
    t_bfs *bfs;

    if (!(bfs = (t_bfs*)malloc(sizeof(t_bfs))))
        return NULL;

    bfs->queue_front = 0;
    bfs->queue_size = 0;
    bfs->queue_rear = 0;
    bfs->node = 0;
    bfs->shortest_path = NULL;
    bfs->queue_capacity = graph->size * 2;

    if (!(bfs->queue = malloc(bfs->queue_capacity * sizeof(ssize_t))))
    {
        free(bfs);
        return NULL;
    }
    if ((bfs->prev = (ssize_t*)malloc(graph->size * sizeof(ssize_t))) == NULL)
    {
        free(bfs->queue);
        free(bfs);
        return NULL;
    }
    for (size_t i = 0; i < graph->size; i++)
    {
        bfs->prev[i] = -1;
        bfs->queue[i] = -1;
        if (graph->nodes[i].flags & ROOM_START)
        {
            bfs->queue[0] = i;
            bfs->queue_size = 1;
            graph->nodes[i].bfs_marked = TRUE;
            graph->nodes[i].enqueued = TRUE;
            graph->nodes[i].enqueued_backward = TRUE;
        }
    }
    return bfs;
}
