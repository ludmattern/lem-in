#include "../inc/libft.h"

int	ft_putstr_fd(const char *s, int fd)
{
	size_t	len;

	if (!s)
		return (0);
	len = ft_strlen(s);
	if (write(fd, &(*s), len) < 0)
		return (-1);
	return (len);
}
