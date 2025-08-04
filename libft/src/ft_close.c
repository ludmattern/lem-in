#include "../inc/libft.h"

int	ft_close(int fd)
{
	if (fd != -1)
	{
		close(fd);
		fd = -1;
	}
	return (0);
}
