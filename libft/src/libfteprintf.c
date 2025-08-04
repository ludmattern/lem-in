#include "../inc/ft_printf.h"

static int	process_format(const char **format, va_list *args)
{
	int	result;

	(*format)++;
	if (**format == 'c')
		result = print_char_eprintf((char)va_arg(*args, int));
	else if (**format == 's')
		result = print_string_eprintf((char *)va_arg(*args, char *));
	else if (**format == 'p')
		result = print_pointer_eprintf((void *)va_arg(*args, void *), "", 0);
	else if (**format == 'i' || **format == 'd')
		result = print_int_eprintf((int)va_arg(*args, int));
	else if (**format == 'u' || **format == 'x' || **format == 'X')
		result = print_num_eprintf((unsigned int)va_arg(*args, unsigned int), \
		**format);
	else if (**format == '%')
		result = ft_putchar_fd('%', 2);
	else
	{
		(*format)--;
		return (0);
	}
	if (result == -1)
		return (-1);
	return (result);
}

int	ft_eprintf(const char *format, ...)
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
			if (ft_putchar_fd(*format, 2) == -1)
				return (-1);
			nb_char++;
		}
		format++;
	}
	va_end(args);
	return (nb_char);
}
