#include "../inc/ft_printf.h"

static size_t	ft_num_len(unsigned long long num, int base)
{
	size_t	length;

	if (base < 0)
		base *= -1;
	length = 0;
	if (!num)
		length = 1;
	while (num)
	{
		num /= base;
		length++;
	}
	return (length);
}

static void	fill_str(char *str, unsigned long long num, size_t len, int base)
{
	size_t		i;
	char		*digits;
	static char	upper[] = "0123456789ABCDEF";
	static char	lower[] = "0123456789abcdef";

	digits = upper;
	if (base < 0)
	{
		digits = lower;
		base *= -1;
	}
	i = len - 1;
	if (!num)
		str[i] = '0';
	while (num)
	{
		str[i--] = digits[num % base];
		num /= base;
	}
}

char	*itoa_printf(unsigned long long num, int base)
{
	char	*str;
	size_t	len;

	len = ft_num_len(num, base);
	str = ft_calloc(len + 1, sizeof(char));
	if (!str)
		return (NULL);
	fill_str(str, num, len, base);
	return (str);
}
