#include "../inc/libft.h"

void	*ft_calloc(size_t nmemb, size_t size)
{
	unsigned char	*ptr;
	size_t			max_allocatable;
	size_t			total_size;

	max_allocatable = (size_t)-1;
	if (!nmemb || !size)
		total_size = 0;
	else
	{
		if (nmemb > max_allocatable / size)
			return (NULL);
		total_size = nmemb * size;
	}
	ptr = malloc(total_size);
	if (!ptr)
		return (NULL);
	ft_bzero(ptr, total_size);
	return (ptr);
}
