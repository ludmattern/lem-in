#include "../inc/ft_printf.h"

static int	process_format(const char **format, va_list *args)
{
	int	result;

	(*format)++;
	if (**format == 'c')
		result = print_char_printf((char)va_arg(*args, int));
	else if (**format == 's')
		result = print_string_printf((char *)va_arg(*args, char *));
	else if (**format == 'p')
		result = print_pointer_printf((void *)va_arg(*args, void *), "", 0);
	else if (**format == 'i' || **format == 'd')
		result = print_int_printf((int)va_arg(*args, int));
	else if (**format == 'u' || **format == 'x' || **format == 'X')
		result = print_num_printf((unsigned int)va_arg(*args, unsigned int), \
		**format);
	else if (**format == '%')
		result = ft_putchar_fd('%', 1);
	else
	{
		(*format)--;
		return (0);
	}
	if (result == -1)
		return (-1);
	return (result);
}

int	ft_printf(const char *format, ...)
{
	va_list	args;
	int		nb_char;
	int		result;

	nb_char = 0;
	va_start(args, format);
	while (*format)
	{
		if (*format == '%')
		{
			result = process_format(&format, &args);
			if (result == -1)
				return (-1);
			nb_char += result;
		}
		else
		{
			if (ft_putchar_fd(*format, 1) == -1)
				return (-1);
			nb_char++;
		}
		format++;
	}
	va_end(args);
	return (nb_char);
}
