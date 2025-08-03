#include "lem_in.h"

bool display_input(const lem_in_parser_t *parser)
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
