#include "lem_in.h"

bool validate_ant_count(const char *line, int32_t *count, error_code_t *error)
{
	if (!line || !*line || !count || !error)
	{
		if (error)
			*error = ERR_EMPTY_INPUT;
		return false;
	}

	// Skip leading whitespace
	while (*line == ' ' || *line == '\t')
		line++;

	if (!*line)
	{
		*error = ERR_EMPTY_INPUT;
		return false;
	}

	char *endptr;
	errno = 0; // Reset errno for error detection
	long num = ft_strtol(line, &endptr, 10);

	// Check for conversion errors
	if (errno == ERANGE)
	{
		*error = ERR_ANT_COUNT_OVERFLOW;
		return false;
	}

	// Must be valid integer (no trailing characters except whitespace/newline)
	while (*endptr == ' ' || *endptr == '\t' || *endptr == '\n' || *endptr == '\r')
		endptr++;

	if (*endptr != '\0')
	{
		*error = ERR_INVALID_ANT_COUNT;
		return false;
	}

	if (num <= 0)
	{
		*error = ERR_ANT_COUNT_ZERO;
		return false;
	}

	if (num > INT32_MAX)
	{
		*error = ERR_ANT_COUNT_OVERFLOW;
		return false;
	}

	*count = (int32_t)num;
	*error = ERR_NONE;
	return true;
}

bool validate_room_name(const char *name, error_code_t *error)
{
	if (!name || !*name || !error)
	{
		if (error)
			*error = ERR_ROOM_NAME_INVALID;
		return false;
	}

	// CRITICAL RULE: Cannot start with 'L' (reserved for ants in output)
	if (name[0] == 'L')
	{
		*error = ERR_ROOM_NAME_STARTS_L;
		return false;
	}

	// CRITICAL RULE: Cannot start with '#' (reserved for comments)
	if (name[0] == '#')
	{
		*error = ERR_ROOM_NAME_STARTS_HASH;
		return false;
	}

	// Check for forbidden characters
	for (const char *p = name; *p; p++)
	{
		if (*p == ' ' || *p == '\t')
		{
			*error = ERR_ROOM_NAME_HAS_SPACE;
			return false;
		}
		if (*p == '-')
		{
			*error = ERR_ROOM_NAME_HAS_DASH;
			return false;
		}
		// Additional checks for control characters and line endings
		if (*p < 32 || *p == 127 || *p == '\n' || *p == '\r')
		{
			*error = ERR_ROOM_NAME_INVALID;
			return false;
		}
	}

	// Check for reasonable name length
	size_t len = ft_strlen(name);
	if (len > 255) // Reasonable limit
	{
		*error = ERR_ROOM_NAME_INVALID;
		return false;
	}

	*error = ERR_NONE;
	return true;
}

// Helper function for clean integer validation
static bool is_valid_integer(const char *str)
{
	if (!str || !*str)
		return false;

	const char *p = str;

	// Allow optional sign
	if (*p == '-' || *p == '+')
		p++;

	// Must have at least one digit after sign
	if (*p == '\0')
		return false;

	// Only digits allowed
	while (*p)
	{
		if (*p < '0' || *p > '9')
			return false;
		p++;
	}

	return true;
}

bool validate_coordinates(const char *x_str, const char *y_str, error_code_t *error)
{
	if (!x_str || !y_str || !error)
	{
		if (error)
			*error = ERR_ROOM_COORDINATES;
		return false;
	}

	// Check for empty strings
	if (*x_str == '\0' || *y_str == '\0')
	{
		*error = ERR_ROOM_COORDINATES;
		return false;
	}

	// Simple validation: must be integers only (with optional sign)
	if (!is_valid_integer(x_str) || !is_valid_integer(y_str))
	{
		*error = ERR_ROOM_COORDINATES;
		return false;
	}

	// Parse and range check
	char *endptr;
	errno = 0;

	long x = ft_strtol(x_str, &endptr, 10);
	if (errno == ERANGE || x < INT32_MIN || x > INT32_MAX)
	{
		*error = ERR_ROOM_COORDINATES;
		return false;
	}

	errno = 0;
	long y = ft_strtol(y_str, &endptr, 10);
	if (errno == ERANGE || y < INT32_MIN || y > INT32_MAX)
	{
		*error = ERR_ROOM_COORDINATES;
		return false;
	}

	*error = ERR_NONE;
	return true;
}

bool is_room_line(const char *line)
{
	if (!line || !*line)
		return false;

	// Skip comments
	if (*line == '#')
		return false;

	// Simple token counting approach
	const char *p = line;
	int token_count = 0;
	bool has_dash_in_first_token = false;

	while (*p)
	{
		// Skip leading whitespace
		while (*p == ' ' || *p == '\t')
			p++;
		if (!*p)
			break;

		// Found start of token
		token_count++;
		const char *token_start = p;

		// Read until end of token
		while (*p && *p != ' ' && *p != '\t' && *p != '\n' && *p != '\r')
			p++;

		// Check if FIRST token contains a dash (indicates link)
		if (token_count == 1)
		{
			for (const char *check = token_start; check < p; check++)
			{
				if (*check == '-')
				{
					has_dash_in_first_token = true;
					break;
				}
			}
		}
	}

	// Simple decision:
	// - 1 token with dash = LINK
	// - 3 tokens = ROOM (let parse_room_line handle validation)
	// - anything else = invalid (let main parser handle)

	if (token_count == 1 && has_dash_in_first_token)
		return false; // It's a link

	if (token_count == 3)
		return true; // Treat as room, validation will catch errors

	return false; // Invalid format
}
