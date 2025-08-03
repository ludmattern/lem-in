/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_sprintf.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fprevot <fprevot@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/14 10:12:36 by lmattern          #+#    #+#             */
/*   Updated: 2024/05/08 15:48:06 by fprevot          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/ft_printf.h"

static int	process_format(const char **format, va_list *args, char *buffer)
{
	int	result;

	(*format)++;
	if (**format == 'c')
		result = print_char_sprintf((char)va_arg(*args, int), buffer);
	else if (**format == 's')
		result = print_string_sprintf((char *)va_arg(*args, char *), buffer);
	else if (**format == 'p')
	{
		result = print_pointer_sprintf((void *)va_arg(*args, void *), \
		"", 0, buffer);
	}
	else if (**format == 'i' || **format == 'd')
		result = print_int_sprintf((int)va_arg(*args, int), buffer);
	else if (**format == 'u' || **format == 'x' || **format == 'X')
		result = print_num_sprintf((unsigned int)va_arg(*args, unsigned int), \
		**format, buffer);
	else if (**format == '%')
		result = print_char_sprintf('%', buffer);
	else
		return ((*format)--, 0);
	if (result == -1)
		return (-1);
	return (result);
}

int	ft_sprintf(char *buffer, const char *format, ...)
{
	va_list	args;
	int		nb_char;
	int		result;

	nb_char = 0;
	if (!buffer)
		return (0);
	va_start(args, format);
	while (*format)
	{
		if (*format == '%')
		{
			result = process_format(&format, &args, buffer);
			if (result == -1)
				return (-1);
			nb_char += result;
		}
		else
		{
			print_char_sprintf(*format, buffer);
			nb_char++;
		}
		format++;
	}
	return (va_end(args), nb_char);
}
