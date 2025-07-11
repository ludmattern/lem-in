/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   validator.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmattern <lmattern@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/11 15:36:29 by lmattern          #+#    #+#             */
/*   Updated: 2025/07/11 15:36:29 by lmattern         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "lem_in.h"

// ============================================================================
// VALIDATION MODULE - Modern input validation with detailed error reporting
// ============================================================================

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
	long num = strtol(line, &endptr, 10);

	// Check for conversion errors
	if (errno == ERANGE)
	{
		*error = ERR_ANT_COUNT_OVERFLOW;
		return false;
	}

	// Must be valid integer (no trailing characters except whitespace/newline)
	while (*endptr == ' ' || *endptr == '\t' || *endptr == '\n' || *endptr == '\r')
	{
		endptr++;
	}

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
		if (*p == ' ')
		{
			*error = ERR_ROOM_NAME_HAS_SPACE;
			return false;
		}
		if (*p == '-')
		{
			*error = ERR_ROOM_NAME_HAS_DASH;
			return false;
		}
		// Additional checks for control characters
		if (*p < 32 || *p == 127)
		{
			*error = ERR_ROOM_NAME_INVALID;
			return false;
		}
	}

	*error = ERR_NONE;
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

	char *endptr;
	errno = 0;

	// Validate X coordinate
	long x = strtol(x_str, &endptr, 10);
	if (errno == ERANGE || *endptr != '\0' || x < INT32_MIN || x > INT32_MAX)
	{
		*error = ERR_ROOM_COORDINATES;
		return false;
	}

	// Validate Y coordinate
	errno = 0;
	long y = strtol(y_str, &endptr, 10);
	if (errno == ERANGE || *endptr != '\0' || y < INT32_MIN || y > INT32_MAX)
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

	// Skip comments and links
	if (*line == '#' || strchr(line, '-') != NULL)
	{
		return false;
	}

	// Basic format check: name X Y
	const char *p = line;

	// Skip room name
	while (*p && *p != ' ' && *p != '\t')
		p++;
	if (!*p)
		return false;

	// Skip whitespace
	while (*p == ' ' || *p == '\t')
		p++;
	if (!*p)
		return false;

	// Check for number (X coordinate)
	if (*p == '-')
		p++; // Allow negative
	if (!(*p >= '0' && *p <= '9'))
		return false;
	while (*p >= '0' && *p <= '9')
		p++;

	// Must have space before Y coordinate
	if (*p != ' ' && *p != '\t')
		return false;
	while (*p == ' ' || *p == '\t')
		p++;
	if (!*p)
		return false;

	// Check for number (Y coordinate)
	if (*p == '-')
		p++; // Allow negative
	if (!(*p >= '0' && *p <= '9'))
		return false;
	while (*p >= '0' && *p <= '9')
		p++;

	// Should be end of line or whitespace
	while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
		p++;

	return *p == '\0';
}
