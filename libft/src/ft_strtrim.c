/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strtrim.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmattern <lmattern@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/09 14:13:07 by lmattern          #+#    #+#             */
/*   Updated: 2024/01/02 13:38:30 by lmattern         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/libft.h"

char	*ft_strtrim(char const *s1, char const *set)
{
	size_t	str_end;
	char	*str;

	if (!s1)
		return (NULL);
	if (!set || !*set)
		return (ft_strdup(s1));
	while (*s1 && ft_strchr(set, *s1))
		s1++;
	str_end = ft_strlen(s1);
	if (str_end == 0)
		return (ft_strdup(""));
	str_end--;
	while (str_end > 0 && ft_strchr(set, s1[str_end]))
		str_end--;
	str = ft_substr(s1, 0, str_end + 1);
	return (str);
}
