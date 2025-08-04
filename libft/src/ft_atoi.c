#include "../inc/libft.h"

long	ft_atoi(const char *nptr)
{
	long	result;
	int		sign;

	result = 0;
	sign = 1;
	while ((*nptr >= '\t' && *nptr <= '\r') || *nptr == ' ')
		nptr++;
	if (*nptr == '-' || *nptr == '+')
	{
		if (*nptr == '-')
			sign = -1;
		nptr++;
	}
	while (*nptr >= '0' && *nptr <= '9')
	{
		if (result * sign > INT_MAX)
			return (-1);
		result = result * 10 + (*nptr - '0');
		nptr++;
	}
	return ((int)(result * sign));
}
