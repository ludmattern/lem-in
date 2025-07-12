# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: lmattern <lmattern@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/07/11 15:36:10 by lmattern          #+#    #+#              #
#    Updated: 2025/07/11 15:36:10 by lmattern         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = lem-in

# Directories
SRCDIR = srcs
OBJDIR = objs
INCDIR = include

# Source files - modular approach
SRCS = main.c \
       parser.c \
       validator.c \
       hash.c \
       error.c \
       output.c

# Object files
OBJS = $(SRCS:%.c=$(OBJDIR)/%.o)

# Compiler and flags - Modern C with strict warnings
CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -pedantic
CFLAGS += -O2 -march=native  # Optimization
INCLUDES = -I$(INCDIR)

# Debug flags (only when explicitly requested)
DEBUG_FLAGS = -g3 -DDEBUG

# Additional security and analysis flags
CFLAGS += -Wformat=2 -Wformat-security -Wcast-align -Wpointer-arith
CFLAGS += -Wwrite-strings -Wmissing-prototypes -Wstrict-prototypes
CFLAGS += -fstack-protector-strong -D_FORTIFY_SOURCE=2

# Colors for output
GREEN = \033[0;32m
RED = \033[0;31m
BLUE = \033[0;34m
RESET = \033[0m

# Rules
all: $(NAME)

$(NAME): $(OBJDIR) $(OBJS)
	@echo "$(BLUE)Linking $(NAME)...$(RESET)"
	@$(CC) $(CFLAGS) $(OBJS) -o $(NAME)
	@echo "$(GREEN)$(NAME) compiled successfully!$(RESET)"

$(OBJDIR):
	@mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@echo "$(BLUE)Compiling $<...$(RESET)"
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Build variants
debug: CFLAGS += -g3 -DDEBUG -fsanitize=address,undefined
debug: $(NAME)

release: CFLAGS = -Wall -Wextra -Werror -std=c11 -O3 -DNDEBUG
release: fclean $(NAME)

# Clean targets  
clean:
	@echo "$(RED)Removing object files...$(RESET)"
	@rm -rf $(OBJDIR)

fclean: clean
	@echo "$(RED)Removing $(NAME)...$(RESET)"
	@rm -f $(NAME)

re: fclean all

# Testing
test: $(NAME)
	@echo "$(BLUE)Running comprehensive test suite...$(RESET)"
	@./test_suite.sh

.PHONY: all clean fclean re test debug release
