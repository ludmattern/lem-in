#include "../inc/libft.h"

size_t	skip_prefix(char *program)
{
	size_t	len;

	len = 0;
	if (!program)
		return (0);
	if (ft_strchr(program, '/'))
	{
		while (program[len] != '/')
			len++;
		len++;
	}
	return (len);
}

int	ft_check_extension(char *program, char *file, char *ext)
{
	if (ft_strnrcmp(file, ext, ft_strlen(ext)))
	{
		ft_eprintf("Error\n%s only accepts %s files\n", program
			+ skip_prefix(program), ext);
		return (0);
	}
	return (1);
}
