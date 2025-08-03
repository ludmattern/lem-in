/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_calloc.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmattern <lmattern@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/08 10:10:17 by lmattern          #+#    #+#             */
/*   Updated: 2024/01/02 13:36:42 by lmattern         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/libft.h"

void	*ft_calloc(size_t nmemb, size_t size)
{
	unsigned char	*ptr;
	size_t			max_allocatable;
	size_t			total_size;

	max_allocatable = (size_t)-1;
	if (!nmemb || !size)
		total_size = 0;
	else
	{
		if (nmemb > max_allocatable / size)
			return (NULL);
		total_size = nmemb * size;
	}
	ptr = malloc(total_size);
	if (!ptr)
		return (NULL);
	ft_bzero(ptr, total_size);
	return (ptr);
}
