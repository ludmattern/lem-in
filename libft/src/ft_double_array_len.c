#include "../inc/libft.h"

/*
Counts the number of double array entries.
*/
size_t	ft_double_array_len(char **array)
{
	size_t	i;

	i = 0;
	while (array[i] != NULL)
		i++;
	return (i);
}
