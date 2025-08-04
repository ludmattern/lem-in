#include "lem_in.h"

// djb2 hash algorithm - industry standard
uint32_t hash_string(const char *str)
{
	if (!str)
		return 0;

	uint32_t hash = 5381;
	int c;

	while ((c = *str++))
		hash = ((hash << 5) + hash) + c; // hash * 33 + c

	return hash;
}

bool hash_add_room(lem_in_parser_t *parser, const char *name, uint16_t room_id)
{
	if (!parser || !name || !parser->hash_table)
		return false;

	uint32_t hash = hash_string(name);
	uint32_t index = hash & (HASH_SIZE - 1);
	uint32_t original_index = index;

	// Linear probing with wraparound
	while (parser->hash_table[index].name != NULL)
	{
		// Check for duplicate
		if (ft_strncmp(parser->hash_table[index].name, name, ft_strlen(name)) == 0)
			return false; // Duplicate found

		index = (index + 1) & (HASH_SIZE - 1);

		// Table full check (should never happen with our size limits)
		if (index == original_index)
			return false;
	}

	// Add the entry
	parser->hash_table[index].name = name;
	parser->hash_table[index].room_id = room_id;

	return true;
}

int16_t hash_get_room_id(const lem_in_parser_t *parser, const char *name)
{
	if (!parser || !name || !parser->hash_table)
		return -1;

	uint32_t hash = hash_string(name);
	uint32_t index = hash & (HASH_SIZE - 1);
	uint32_t original_index = index;

	while (parser->hash_table[index].name != NULL)
	{
		if (ft_strncmp(parser->hash_table[index].name, name, ft_strlen(name)) == 0)
			return parser->hash_table[index].room_id;

		index = (index + 1) & (HASH_SIZE - 1);

		if (index == original_index)
			break; // Searched entire table
	}

	return -1; // Not found
}
