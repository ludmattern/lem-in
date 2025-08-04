#include "../inc/libft.h"

char	*ft_strdup(const char *s)
{
	char	*str;
	char	*original;
	size_t	length;

	length = 0;
	if (!s)
		return (NULL);
	while (s[length])
		length++;
	str = ft_calloc((length + 1), sizeof(char));
	if (str == NULL)
		return (NULL);
	original = str;
	while (*s)
		*str++ = *s++;
	return (original);
}
