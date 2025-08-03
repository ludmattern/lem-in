#include "lem_in.h"

const char *error_to_string(error_code_t code)
{
	static const char *error_messages[] = {
		[ERR_NONE] = "No error",
		[ERR_MEMORY] = "Memory allocation failed",
		[ERR_INPUT_READ] = "Cannot read input",
		[ERR_EMPTY_INPUT] = "Empty input or missing ant count",
		[ERR_INVALID_ANT_COUNT] = "Invalid ant count format",
		[ERR_ANT_COUNT_ZERO] = "Ant count must be > 0",
		[ERR_ANT_COUNT_OVERFLOW] = "Ant count too large",
		[ERR_ROOM_NAME_INVALID] = "Invalid room name",
		[ERR_ROOM_NAME_STARTS_L] = "Room name cannot start with 'L'",
		[ERR_ROOM_NAME_STARTS_HASH] = "Room name cannot start with '#'",
		[ERR_ROOM_NAME_HAS_SPACE] = "Room name cannot contain spaces",
		[ERR_ROOM_NAME_HAS_DASH] = "Room name cannot contain dashes",
		[ERR_ROOM_DUPLICATE] = "Duplicate room name",
		[ERR_ROOM_COORDINATES] = "Invalid coordinates for room",
		[ERR_MULTIPLE_START] = "Multiple ##start rooms defined",
		[ERR_MULTIPLE_END] = "Multiple ##end rooms defined",
		[ERR_LINK_INVALID] = "Invalid link format",
		[ERR_LINK_SELF] = "Room cannot link to itself",
		[ERR_LINK_ROOM_NOT_FOUND] = "Link references unknown room",
		[ERR_NO_START] = "No ##start room defined",
		[ERR_NO_END] = "No ##end room defined",
		[ERR_NO_ROOMS] = "No rooms defined",
		[ERR_INVALID_LINE] = "Invalid line format",
		[ERR_TOO_MANY_ROOMS] = "Too many rooms",
		[ERR_TOO_MANY_LINKS] = "Too many links",
		[ERR_NO_PATH] = "No path found"};

	if (code >= 0 && code < sizeof(error_messages) / sizeof(error_messages[0]))
	{
		return error_messages[code];
	}
	return "Unknown error";
}

bool print_error(error_code_t code, const char *context)
{
	fprintf(stderr, "ERROR: %s", error_to_string(code));
	if (context && *context)
	{
		fprintf(stderr, " '%s'", context);
	}
	fprintf(stderr, "\n");
	return false;
}
