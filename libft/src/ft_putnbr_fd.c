#include "../inc/libft.h"

void	ft_putnbr_fd(int n, int fd)
{
	unsigned int	nb;

	nb = (unsigned int)n;
	if (n < 0)
	{
		ft_putchar_fd('-', fd);
		nb = -nb;
	}
	if (nb >= 10)
		ft_putnbr_fd(nb / 10, fd);
	ft_putchar_fd((nb % 10) + '0', fd);
}
