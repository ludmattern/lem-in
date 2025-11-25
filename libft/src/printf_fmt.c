#include "../inc/ft_printf.h"

int	print_num_printf(unsigned int num, char fmt)
{
	char	*str;
	int		len;
	int		base;

	base = 10;
	if (fmt != 'u')
	{
		base = 16;
		if (fmt == 'x')
			base = -16;
	}
	str = itoa_printf(num, base);
	if (!str)
		return (-1);
	if (ft_putstr_fd(str, 1) < 0)
	{
		free(str);
		return (-1);
	}
	len = ft_strlen(str);
	free(str);
	return (len);
}

int	print_char_printf(char c)
{
	if (ft_putchar_fd(c, 1) < 0)
		return (-1);
	return (1);
}

int	print_string_printf(char *str)
{
	if (!str)
		str = "(null)";
	if (ft_putstr_fd(str, 1) < 0)
		return (-1);
	return (ft_strlen(str));
}

int	print_pointer_printf(void *ptr, char *str, int len)
{
	char	*full_str;

	if (!ptr)
	{
		if (ft_putstr_fd("(nil)", 1) < 0)
			return (-1);
		return (5);
	}
	str = itoa_printf((unsigned long long)ptr, -16);
	if (!str)
		return (-1);
	len = ft_strlen(str) + 2;
	full_str = malloc(len + 1);
	if (!full_str)
	{
		free(str);
		return (-1);
	}
	ft_strlcpy(full_str, "0x", 3);
	ft_strlcat(full_str + 2, str, len + 1);
	if (ft_putstr_fd(full_str, 1) < 0)
		len = -1;
	free(str);
	free(full_str);
	return (len);
}

int	print_int_printf(int nbr)
{
	char	*str;
	int		len;

	str = ft_itoa(nbr);
	if (!str)
		return (-1);
	if (ft_putstr_fd(str, 1) < 0)
	{
		free(str);
		return (-1);
	}
	len = ft_strlen(str);
	free(str);
	return (len);
}

int	print_size_t_printf(size_t num)
{
	char	*str;
	int		len;

	str = itoa_printf((unsigned long long)num, 10);
	if (!str)
		return (-1);
	if (ft_putstr_fd(str, 1) < 0)
	{
		free(str);
		return (-1);
	}
	len = ft_strlen(str);
	free(str);
	return (len);
}
