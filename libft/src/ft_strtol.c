#include "../inc/libft.h"

long	ft_strtol(const char *str, char **endptr, int base)
{
	long result;
	int sign;

	if (!str)
		return (0);
	while (*str && ft_isspace(*str))
		str++;
	sign = 1;
	if (*str == '-' || *str == '+')
	{
		if (*str == '-')
			sign = -1;
		str++;
	}
	result = 0;
	while (*str && ft_isdigit(*str))
	{
		result = result * base + (*str - '0');
		str++;
	}
	if (endptr)
		*endptr = (char *)str;
	return (result * sign);
}
