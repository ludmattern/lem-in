#include "../inc/ft_printf.h"

int	print_num_sprintf(unsigned int num, char fmt, char *buffer)
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
	print_string_sprintf(str, buffer);
	len = ft_strlen(str);
	free(str);
	return (len);
}

int	print_char_sprintf(char c, char *buffer)
{
	int	i;

	i = 0;
	while (buffer[i])
		i++;
	buffer[i++] = c;
	buffer[i] = 0;
	return (1);
}

int	print_string_sprintf(char *str, char *buffer)
{
	int	i;

	if (!str)
		str = "(null)";
	i = 0;
	while (str[i])
		print_char_sprintf(str[i++], buffer);
	return (ft_strlen(str));
}

int	print_pointer_sprintf(void *ptr, char *str, int len, char *buffer)
{
	char	*full_str;

	if (!ptr)
		return (print_string_sprintf("(nil)", buffer));
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
	print_string_sprintf(full_str, buffer);
	free(str);
	free(full_str);
	return (len);
}

int	print_int_sprintf(int nbr, char *buffer)
{
	char	*str;
	int		len;

	str = ft_itoa(nbr);
	if (!str)
		return (-1);
	print_string_sprintf(str, buffer);
	len = ft_strlen(str);
	free(str);
	return (len);
}
