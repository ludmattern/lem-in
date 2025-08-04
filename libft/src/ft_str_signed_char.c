#include "../inc/ft_printf.h"

char	*ft_str_signed_chr(const char *s, int c)
{
	while (*s && (*s != c))
		s++;
	if (*s == c)
		return ((char *)s);
	return (NULL);
}
