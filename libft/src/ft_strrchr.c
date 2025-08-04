#include "../inc/libft.h"

char	*ft_strrchr(const char *s, int c)
{
	size_t	s_len;

	s_len = ft_strlen(s) + 1;
	while (s_len-- > 0)
		if (s[s_len] == (unsigned char)c)
			return ((char *)(s + s_len));
	return (NULL);
}
