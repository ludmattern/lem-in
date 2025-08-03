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

# Source files - modular approach with new organization
SRCS = main.c \
       parser.c \
       parse_line.c \
       input.c \
       validator.c \
       hash.c \
       error.c \
       output.c \
       pathfinding.c

# Object files
OBJS = $(SRCS:%.c=$(OBJDIR)/%.o)

# Compiler and flags - Modern C with strict warnings
CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -pedantic
CFLAGS += -O2 -march=native  # Optimization
INCLUDES = -I$(INCDIR)

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

release: CFLAGS = -Wall -Wextra -Werror -std=c11 -O3 -DNDEBUG
release: fclean $(NAME)

# Bonus rule - compile both lem-in and visualizer
bonus: $(NAME)
	@echo "$(BLUE)Compiling visualizer...$(RESET)"
	@cd visualizer && make
	@echo "$(GREEN)Bonus compilation complete!$(RESET)"

# Visualizer rule - run lem-in with map and pipe to visualizer
visualizer:
	@if [ -z "$(MAP)" ]; then \
		echo "$(RED)Error: Please specify a map file with MAP=path/to/map$(RESET)"; \
		echo "Example: make visualizer MAP=resources/valid_maps/simple_test"; \
		exit 1; \
	fi
	@if [ ! -f "./visualizer/visualizer" ]; then \
		echo "$(RED)Error: Visualizer not compiled. Please run 'make bonus' first.$(RESET)"; \
		exit 1; \
	fi
	@echo "$(BLUE)Running visualizer with map: $(MAP)$(RESET)"
	@./lem-in < $(MAP) | ./visualizer/visualizer

# Clean targets  
clean:
	@echo "$(RED)Removing object files...$(RESET)"
	@rm -rf $(OBJDIR)
	@cd visualizer && make clean

fclean: clean
	@echo "$(RED)Removing $(NAME)...$(RESET)"
	@rm -f $(NAME)
	@cd visualizer && make fclean

re: fclean all

# Testing
test: $(NAME)
	@echo "$(BLUE)Running comprehensive test suite...$(RESET)"
	@./test_suite.sh -v

.PHONY: all clean fclean re test release bonus visualizer
