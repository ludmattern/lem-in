# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: lmattern <lmattern@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/11/06 16:14:01 by lmattern          #+#    #+#              #
#    Updated: 2024/05/17 13:53:15 by lmattern         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

.PHONY: all clean fclean re

FLAGS= 	-Wall -Werror -Wextra -fPIC

NAME= 	libft.a

SRCS= 	./src/ft_isalpha.c ./src/ft_isdigit.c ./src/ft_isalnum.c \
		./src/ft_isascii.c ./src/ft_isprint.c ./src/ft_strlen.c \
		./src/ft_strdup.c ./src/ft_strchr.c ./src/ft_strrchr.c \
		./src/ft_strncmp.c ./src/ft_strnstr.c ./src/ft_strlcpy.c \
		./src/ft_strlcat.c ./src/ft_substr.c ./src/ft_strjoin.c \
		./src/ft_strtrim.c ./src/ft_split.c ./src/ft_itoa.c \
		./src/ft_strmapi.c ./src/ft_striteri.c ./src/ft_memset.c \
		./src/ft_bzero.c ./src/ft_memcpy.c ./src/ft_memmove.c \
		./src/ft_memchr.c ./src/ft_memcmp.c ./src/ft_calloc.c \
		./src/ft_toupper.c ./src/ft_tolower.c ./src/ft_atoi.c \
		./src/ft_putchar_fd.c ./src/ft_putstr_fd.c ./src/ft_putendl_fd.c \
		./src/ft_putnbr_fd.c ./src/ft_lstnew_bonus.c ./src/ft_strndup.c\
		./src/ft_lstadd_front_bonus.c ./src/ft_lstsize_bonus.c \
		./src/ft_lstlast_bonus.c ./src/ft_lstadd_back_bonus.c \
		./src/ft_lstdelone_bonus.c ./src/ft_lstclear_bonus.c \
		./src/ft_lstiter_bonus.c ./src/ft_lstmap_bonus.c \
		./src/get_next_line.c ./src/get_next_line_utils.c \
		./src/printf_fmt.c ./src/itoa_printf.c ./src/libftprintf.c \
		./src/eprintf_fmt.c ./src/libfteprintf.c ./src/ft_strcat.c \
		./src/ft_strncpy.c ./src/ft_strcpy.c ./src/ft_realloc.c \
		./src/ft_free_double_array.c ./src/ft_isspace.c \
		./src/ft_double_array_len.c ./src/ft_free.c ./src/ft_sprintf_fmt.c \
		./src/ft_sprintf.c ./src/ft_str_signed_char.c ./src/ft_strnrcmp.c \
		./src/ft_check_extension.c ./src/ft_close.c

OBJS= 	$(SRCS:.c=.o)

all:	$(NAME)

$(NAME): $(OBJS)
			@ar rcs $(NAME) $(OBJS)

%.o: 	%.c ./inc/libft.h ./inc/get_next_line.h ./inc/ft_printf.h
			@cc $(FLAGS) -c $< -o $@

clean:
			rm -f $(OBJS)

fclean: clean
			rm -f $(NAME)

re: 	fclean all