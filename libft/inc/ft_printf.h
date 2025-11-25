#ifndef FT_PRINTF_H
# define FT_PRINTF_H

# include <stdarg.h>
# include <stdlib.h>
# include <unistd.h>
# include "./libft.h"

/*
ft_printf
*/
int		ft_printf(const char *format, ...);
char	*itoa_printf(unsigned long long num, int base);
int		print_num_printf(unsigned int num, char fmt);
int		print_char_printf(char c);
int		print_string_printf(char *str);
int		print_pointer_printf(void *ptr, char *str, int len);
int		print_int_printf(int nbr);
int		print_size_t_printf(size_t num);

/*
ft_eprintf
*/
int		ft_eprintf(const char *format, ...);
int		print_num_eprintf(unsigned int num, char fmt);
int		print_char_eprintf(char c);
int		print_string_eprintf(char *str);
int		print_pointer_eprintf(void *ptr, char *str, int len);
int		print_int_eprintf(int nbr);

/*
ft_sprintf
*/
int		print_num_sprintf(unsigned int num, char fmt, char *buffer);
int		print_char_sprintf(char c, char *buffer);
int		print_string_sprintf(char *str, char *buffer);
int		print_pointer_sprintf(void *ptr, char *str, int len, char *buffer);
int		print_int_sprintf(int nbr, char *buffer);
int		ft_sprintf(char *buffer, const char *format, ...);

#endif
