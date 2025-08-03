/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fprevot <fprevot@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/03 17:11:51 by lmattern          #+#    #+#             */
/*   Updated: 2024/04/14 14:55:29 by fprevot          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/get_next_line.h"

void	free_buffer(char **buffer)
{
	free(*buffer);
	*buffer = NULL;
}

int	trim_prev_line(char **buffer)
{
	size_t		i;
	size_t		j;
	char		*tmp;

	i = 0;
	while ((*buffer)[i] && (*buffer)[i] != '\n')
		i++;
	if ((*buffer)[i] != '\n')
	{
		free_buffer(buffer);
		return (0);
	}
	tmp = malloc(ft_strlen_gnl(*buffer) - i + 1);
	if (!tmp)
		return (-1);
	j = 0;
	i++;
	while ((*buffer)[i])
		tmp[j++] = (*buffer)[i++];
	tmp[j] = '\0';
	free_buffer(buffer);
	*buffer = tmp;
	return (0);
}

int	extract_line(char **buffer, char **line)
{
	size_t	i;

	i = 0;
	while ((*buffer)[i] && (*buffer)[i] != '\n')
		i++;
	if ((*buffer)[i] == '\n')
		i++;
	*line = ft_substr_gnl(*buffer, 0, i);
	if (!(*line))
		return (-1);
	return (0);
}

int	store_next_line(int fd, char **buffer)
{
	char	*temp;
	char	*new_buffer;
	ssize_t	read_status;

	read_status = 1;
	while (read_status > 0)
	{
		temp = malloc(BUFFER_SIZE + 1);
		if (!temp)
			return (-1);
		read_status = read(fd, temp, BUFFER_SIZE);
		if (read_status == -1 || (!read_status && !(*buffer)))
			return (free(temp), -1);
		temp[read_status] = '\0';
		new_buffer = ft_strjoin_gnl(*buffer, temp);
		free(temp);
		if (!new_buffer)
			return (-1);
		free_buffer(buffer);
		*buffer = new_buffer;
		if (ft_strchr_gnl(*buffer, '\n'))
			break ;
	}
	return (0);
}

char	*get_next_line(int fd)
{
	static char	*buffer = NULL;
	char		*line;

	if (fd < 0 || BUFFER_SIZE <= 0 || read(fd, NULL, 0) == -1)
	{
		free_buffer(&buffer);
		return (NULL);
	}
	if (store_next_line(fd, &buffer) == -1)
	{
		free_buffer(&buffer);
		return (NULL);
	}
	if (extract_line(&buffer, &line) == -1)
	{
		free_buffer(&buffer);
		return (NULL);
	}
	if (trim_prev_line(&buffer) == -1)
	{
		free_buffer(&buffer);
		free(line);
		return (NULL);
	}
	return (line);
}
