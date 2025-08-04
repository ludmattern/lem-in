#include "../inc/get_next_line.h"

char	*ft_substr_gnl(char const *s, unsigned int start, size_t len)
{
	size_t	s_len;
	char	*str;

	if (!s)
		return (NULL);
	s_len = ft_strlen_gnl(s);
	if (start >= s_len)
		return (NULL);
	if (len > s_len - start)
		len = s_len - start;
	str = malloc(len + 1);
	if (!str)
		return (NULL);
	ft_strlcpy_gnl(str, s + start, len + 1);
	return (str);
}

char	*ft_strjoin_gnl(char const *s1, char const *s2)
{
	size_t	s1_len;
	size_t	s2_len;
	char	*str;

	s1_len = 0;
	if (s1)
		s1_len = ft_strlen_gnl(s1);
	s2_len = 0;
	if (s2)
		s2_len = ft_strlen_gnl(s2);
	str = malloc(s1_len + s2_len + 1);
	if (!str)
		return (NULL);
	if (s1)
		ft_strlcpy_gnl(str, s1, s1_len + 1);
	else
		str[0] = '\0';
	if (s2)
		ft_strlcpy_gnl(str + s1_len, s2, s2_len + 1);
	return (str);
}

size_t	ft_strlen_gnl(const char *s)
{
	size_t	i;

	i = 0;
	while (s[i])
		i++;
	return (i);
}

size_t	ft_strlcpy_gnl(char *dst, const char *src, size_t size)
{
	size_t	src_len;
	size_t	i;

	src_len = ft_strlen_gnl(src);
	if (size)
	{
		i = 0;
		while (src[i] && i < size - 1)
		{
			dst[i] = src[i];
			i++;
		}
		if (size > 0)
			dst[i] = '\0';
	}
	return (src_len);
}

char	*ft_strchr_gnl(const char *s, int c)
{
	while (*s && (*s != (unsigned char)c))
		s++;
	if (*s == (unsigned char)c)
		return ((char *)s);
	return (NULL);
}
