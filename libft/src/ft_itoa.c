/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_itoa.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmattern <lmattern@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/09 19:40:19 by lmattern          #+#    #+#             */
/*   Updated: 2024/01/02 13:36:52 by lmattern         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/libft.h"

static size_t	ft_int_len(long nb, int is_negative)
{
	size_t	length;

	length = 0;
	if (is_negative || !nb)
		length = 1;
	while (nb)
	{
		nb /= 10;
		length++;
	}
	return (length);
}

static void	ft_process_sign(int *n, int *is_negative, long *nb)
{
	*nb = *n;
	if (*n < 0)
	{
		*is_negative = 1;
		*nb = -(*nb);
	}
	else
		*is_negative = 0;
}

static void	fill_str(char *str, long nb, size_t len, int is_negative)
{
	size_t	i;

	i = len - 1;
	if (!nb)
		str[i] = '0';
	while (nb)
	{
		str[i--] = (nb % 10) + '0';
		nb /= 10;
	}
	if (is_negative)
		str[0] = '-';
}

char	*ft_itoa(int n)
{
	char	*str;
	long	nb;
	int		is_negative;
	size_t	len;

	ft_process_sign(&n, &is_negative, &nb);
	len = ft_int_len(nb, is_negative);
	str = ft_calloc(len + 1, sizeof(char));
	if (!str)
		return (NULL);
	fill_str(str, nb, len, is_negative);
	return (str);
}
