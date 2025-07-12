#include "lem_in.h"

// ============================================================================
// OUTPUT MODULE - Handle output formatting and display
// ============================================================================

bool output_original_input(const lem_in_parser_t *parser)
{
	if (!parser)
		return false;

	// Output ant count
	printf("%d\n", parser->ant_count);

	// Output rooms in order with their original commands
	for (size_t i = 0; i < parser->room_count; i++)
	{
		const room_t *room = &parser->rooms[i];

		// Output ##start or ##end command if needed
		if (room->flags == ROOM_START)
			printf("##start\n");
		else if (room->flags == ROOM_END)
			printf("##end\n");

		// Output room definition
		printf("%s %d %d\n", room->name, room->x, room->y);
	}

	// Output links
	for (size_t i = 0; i < parser->link_count; i++)
	{
		const link_t *link = &parser->links[i];
		const room_t *from_room = &parser->rooms[link->from];
		const room_t *to_room = &parser->rooms[link->to];
		printf("%s-%s\n", from_room->name, to_room->name);
	}

	return true;
}

// Debug function to print parser state (only in debug builds)
#ifdef DEBUG
#ifdef DEBUG
void debug_print_parser_state(const lem_in_parser_t *parser)
{
	if (!parser)
		return;

	fprintf(stderr, "\n=== PARSER STATE DEBUG ===\n");
	fprintf(stderr, "Ant count: %d\n", parser->ant_count);
	fprintf(stderr, "Room count: %zu\n", parser->room_count);
	fprintf(stderr, "Link count: %zu\n", parser->link_count);
	fprintf(stderr, "Start room ID: %u\n", parser->start_room_id);
	fprintf(stderr, "End room ID: %u\n", parser->end_room_id);

	if (parser->room_count > 0)
	{
		fprintf(stderr, "\nFirst few rooms:\n");
		for (size_t i = 0; i < parser->room_count && i < 3; i++)
		{
			const room_t *room = &parser->rooms[i];
			fprintf(stderr, "  [%zu] %s (%d,%d) flags=0x%x\n",
					i, room->name, room->x, room->y, room->flags);
		}
	}

	if (parser->link_count > 0)
	{
		fprintf(stderr, "\nFirst few links:\n");
		for (size_t i = 0; i < parser->link_count && i < 3; i++)
		{
			const link_t *link = &parser->links[i];
			fprintf(stderr, "  [%zu] %u -> %u\n", i, link->from, link->to);
		}
	}

	fprintf(stderr, "========================\n\n");
}
#endif
#endif
