#include "../inc/libft.h"

void	ft_free_double_int_array(int **double_array, size_t size)
{
	size_t	i;

	i = 0;
	while (double_array && i < size)
	{
		free(double_array[i]);
		double_array[i] = NULL;
		i++;
	}
	free(double_array);
	double_array = NULL;
}

void	ft_free_double_array(char **double_array)
{
	size_t	i;

	i = 0;
	while (double_array && double_array[i])
	{
		free(double_array[i]);
		double_array[i] = NULL;
		i++;
	}
	free(double_array);
	double_array = NULL;
}
