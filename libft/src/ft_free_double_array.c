/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_free_double_array.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fprevot <fprevot@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/07 09:00:26 by lmattern          #+#    #+#             */
/*   Updated: 2024/05/21 11:19:07 by fprevot          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/libft.h"

void	ft_free_double_int_array(int **double_array, size_t size)
{
	size_t	i;

	i = 0;
	while (double_array && i < size)
	{
		free(double_array[i]);
		double_array[i] = NULL;
		i++;
	}
	free(double_array);
	double_array = NULL;
}

void	ft_free_double_array(char **double_array)
{
	size_t	i;

	i = 0;
	while (double_array && double_array[i])
	{
		free(double_array[i]);
		double_array[i] = NULL;
		i++;
	}
	free(double_array);
	double_array = NULL;
}
