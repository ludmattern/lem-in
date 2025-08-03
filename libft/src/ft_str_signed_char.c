/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_str_signed_char.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fprevot <fprevot@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/07 18:58:27 by fprevot           #+#    #+#             */
/*   Updated: 2024/05/07 18:58:46 by fprevot          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/ft_printf.h"

char	*ft_str_signed_chr(const char *s, int c)
{
	while (*s && (*s != c))
		s++;
	if (*s == c)
		return ((char *)s);
	return (NULL);
}
