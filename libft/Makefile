.PHONY: all clean fclean re

CC = cc
CFLAGS = -Wall -Werror -Wextra -fPIC -MMD -MP

SRCS_DIR = ./src
INCLUDE_DIR = ./inc
OBJS_DIR = ./build

SRC_FILES = ft_isalpha ft_isdigit ft_isalnum ft_isascii ft_isprint ft_strlen \
			ft_strdup ft_strchr ft_strrchr ft_strncmp ft_strnstr ft_strlcpy \
			ft_strlcat ft_substr ft_strjoin ft_strtrim ft_split ft_itoa \
			ft_strmapi ft_striteri ft_memset ft_bzero ft_memcpy ft_memmove \
			ft_memchr ft_memcmp ft_calloc ft_toupper ft_tolower ft_atoi ft_abs \
			ft_putchar_fd ft_putstr_fd ft_putendl_fd ft_putnbr_fd ft_lstnew_bonus ft_strndup \
			ft_lstadd_front_bonus ft_lstsize_bonus ft_lstlast_bonus ft_lstadd_back_bonus \
			ft_lstappend \
			ft_lstdelone_bonus ft_lstclear_bonus ft_lstiter_bonus ft_lstmap_bonus \
			get_next_line get_next_line_utils printf_fmt itoa_printf libftprintf \
			eprintf_fmt libfteprintf ft_strcat ft_strncpy ft_strcpy ft_realloc \
			ft_free_double_array ft_isspace ft_double_array_len ft_free ft_sprintf_fmt \
			ft_sprintf ft_str_signed_char ft_strnrcmp ft_check_extension ft_close ft_strtol

SRCS = $(addprefix $(SRCS_DIR)/,$(addsuffix .c,$(SRC_FILES)))
OBJS = $(patsubst $(SRCS_DIR)/%.c,$(OBJS_DIR)/%.o,$(SRCS))
DEPS = $(OBJS:.o=.d)
NAME = libft.a

all: $(NAME)

$(NAME): $(OBJS)
	@ar rcs $(NAME) $(OBJS)

$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.c | $(OBJS_DIR)
	@$(CC) $(CFLAGS) -I $(INCLUDE_DIR) -c $< -o $@

$(OBJS_DIR):
	@mkdir -p $(OBJS_DIR)

clean:
	@rm -f $(OBJS) $(DEPS)
	@rm -rf $(OBJS_DIR)

fclean: clean
	@rm -f $(NAME)

re: fclean all

-include $(DEPS)