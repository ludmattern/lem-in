#include "lem_in.h"

/* ============================================================================
 *                               GRAPH BUILDER FUNCTIONS
 * ============================================================================ */

 //creer un passage entre deux noeuds
int8_t create_edge(t_graph *graph, size_t src, size_t dest)
{
    t_edge *forward_edge;
    t_edge *backward_edge;

    if ((forward_edge = (t_edge*)malloc(sizeof(t_edge))) == NULL)
        return FAILURE;
    forward_edge->capacity = 1;
    forward_edge->dest = dest;
    forward_edge->next = graph->nodes[src].head;
    graph->nodes[src].head = forward_edge;

    if ((backward_edge = (t_edge*)malloc(sizeof(t_edge))) == NULL)
    {
        free(forward_edge);
        return FAILURE;
    }
    backward_edge->capacity = 1;
    backward_edge->dest = src;
    backward_edge->next = graph->nodes[dest].head;
    graph->nodes[dest].head = backward_edge;

    return SUCCESS;
}

// initialiser les valeurs du graph grace a celles recuperee dans le parser
static t_graph *graph_initializer(const lem_in_parser_t *parser, t_graph *graph)
{
    for (size_t i = 0; i < graph->size; i++)
    {
        graph->nodes[i].index = parser->rooms[i].id;
        if (!(graph->nodes[i].name = ft_strdup(parser->rooms[i].name)))
        {
            for (size_t j = 0; j < i; j++)
            {
                free(graph->nodes[j].name);
            }
            return NULL;
        }
        graph->nodes[i].flags = parser->rooms[i].flags;
        if (parser->rooms[i].flags & ROOM_START)
            graph->start_room_id = i;
        else if (parser->rooms[i].flags & ROOM_END)
            graph->end_room_id = i;
        graph->nodes[i].bfs_marked = FALSE;
        graph->nodes[i].enqueued = FALSE;
        graph->nodes[i].enqueued_backward = FALSE;
        graph->nodes[i].head = NULL;
    }
    return graph;
}

// creer le graph donc allouer la memoire pour le graph et les noeuds
t_graph *create_graph(const lem_in_parser_t *parser)
{
    t_graph *graph;
    size_t size = parser->room_count;

    if (size == 0 || (graph = (t_graph*)malloc(sizeof(t_graph))) == NULL)
        return NULL;

    graph->ants = parser->ant_count;
    graph->size = size;
    graph->paths_count = 0;
    graph->old_output_lines = 0;
    graph->start_room_id = INVALID_ROOM_ID;
    graph->end_room_id = INVALID_ROOM_ID;
    if ((graph->nodes = (t_node*)malloc(size * sizeof(t_node))) == NULL)
    {
        free(graph);
        return NULL;
    }
    if (!(graph = graph_initializer(parser, graph)))
    {
        free(graph->nodes);
        free(graph);
        return NULL;
    }
    return graph;
}

// fonction main pour creer le graph
t_graph *graph_builder(const lem_in_parser_t *parser)
{
    t_graph *graph;

    if ((graph = create_graph(parser)) == NULL)
        return NULL;
    for (size_t i = 0; i < parser->link_count; i++)
    {
        if (create_edge(graph, parser->links[i].from, parser->links[i].to) == FAILURE)
        {
            free_graph(graph);
            return NULL;
        }
    }
    if (graph->start_room_id == INVALID_ROOM_ID || graph->end_room_id == INVALID_ROOM_ID || graph->start_room_id == graph->end_room_id)
    {
        free_graph(graph);
        return NULL;
    }
    return graph;
}