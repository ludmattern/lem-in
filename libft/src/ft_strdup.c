/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strdup.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fprevot <fprevot@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/08 11:55:49 by lmattern          #+#    #+#             */
/*   Updated: 2024/05/08 15:42:02 by fprevot          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
