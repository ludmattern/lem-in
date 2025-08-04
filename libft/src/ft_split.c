#include "../inc/libft.h"

static char	**ft_free_array(char **array, size_t nb_wrd)
{
	char	**current;

	current = array;
	while (nb_wrd--)
	{
		free(*current);
		*current = NULL;
		current++;
	}
	free(array);
	return (NULL);
}

static size_t	ft_count_wrds(char const *s, char c)
{
	size_t	nb_wrd;
	int		in_wrd;

	nb_wrd = 0;
	in_wrd = 0;
	while (*s)
	{
		if (*s != c && !in_wrd)
		{
			in_wrd = 1;
			nb_wrd++;
		}
		else if (*s == c && in_wrd)
			in_wrd = 0;
		s++;
	}
	return (nb_wrd);
}

static char	**ft_fill(char **array, char const *s, char c, size_t nb_wrd)
{
	size_t	wrd_start;
	size_t	wrd_len;
	size_t	wrd_index;

	wrd_start = 0;
	wrd_len = 0;
	wrd_index = 0;
	while (nb_wrd > wrd_index)
	{
		while (s[wrd_start] && s[wrd_start] == c)
			wrd_start++;
		while (s[wrd_start + wrd_len] && s[wrd_start + wrd_len] != c)
			wrd_len++;
		array[wrd_index] = ft_substr(s, wrd_start, wrd_len);
		if (array[wrd_index] == NULL)
			return (ft_free_array(array, wrd_index));
		wrd_start += wrd_len + 1;
		wrd_len = 0;
		wrd_index++;
	}
	array[wrd_index] = NULL;
	return (array);
}

char	**ft_split(char const *s, char c)
{
	size_t	nb_wrd;
	char	**array;

	if (!s)
		return (NULL);
	nb_wrd = ft_count_wrds(s, c);
	array = (char **)ft_calloc(nb_wrd + 1, sizeof(char *));
	if (array == NULL)
		return (NULL);
	if (!ft_fill(array, s, c, nb_wrd))
		return (NULL);
	return (array);
}
