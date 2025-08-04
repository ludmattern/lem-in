#include "../inc/libft.h"

int	ft_strnrcmp(const char *s1, const char *s2, size_t n)
{
	size_t	len1;
	size_t	len2;

	len1 = ft_strlen(s1);
	len2 = ft_strlen(s2);
	if (len2 > len1 || !n)
		return (1);
	s1 += len1 - 1;
	s2 += len2 - 1;
	while (n && len2 && *s1 == *s2)
	{
		s1--;
		s2--;
		len2--;
		n--;
	}
	if (!n || !len2)
		return (0);
	return (*(unsigned char *)s1 - *(unsigned char *)s2);
}
