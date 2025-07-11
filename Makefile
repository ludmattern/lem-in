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

# Colors for pretty output
GREEN = \033[0;32m
RED = \033[0;31m
BLUE = \033[0;34m
YELLOW = \033[0;33m
RESET = \033[0m

# Make options for cleaner output
MAKEFLAGS += --no-print-directory

# Rules
all: $(NAME)

$(NAME): $(OBJDIR) $(OBJS)
	@echo "$(BLUE)🔗 Linking $(NAME)...$(RESET)"
	@$(CC) $(CFLAGS) $(OBJS) -o $(NAME)
	@echo "$(GREEN)✅ $(NAME) compiled successfully!$(RESET)"

$(OBJDIR):
	@mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@echo "$(BLUE)🔨 Compiling $<...$(RESET)"
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Development targets
# Development targets
debug: CFLAGS += $(DEBUG_FLAGS) -DDEBUG_VERBOSE -fsanitize=address,undefined
debug: $(NAME)

release: CFLAGS = -Wall -Wextra -Werror -std=c11 -O3 -DNDEBUG
release: fclean $(NAME)

analyze: CFLAGS += -fanalyzer
analyze: $(NAME)

clean:
	@echo "$(RED)Removing object files...$(RESET)"
	@rm -rf $(OBJDIR)

fclean: clean
	@echo "$(RED)Removing $(NAME)...$(RESET)"
	@rm -f $(NAME)

re: fclean all

# Quick integrated test suite (essential tests only)
test: $(NAME)
	@echo "$(BLUE)===========================================$(RESET)"
	@echo "$(BLUE)🧪 LEM-IN QUICK TEST SUITE$(RESET)"
	@echo "$(BLUE)===========================================$(RESET)"
	@$(MAKE) test-basic
	@$(MAKE) test-files
	@echo "$(GREEN)✅ Quick tests completed!$(RESET)"
	@echo "$(CYAN)💡 Run 'make test-full' for comprehensive testing$(RESET)"

# Essential validation tests
test-basic: $(NAME)
	@echo "\n$(YELLOW)📋 Essential Validation$(RESET)"
	@echo "" | ./$(NAME) >/dev/null 2>&1 || echo "$(GREEN)✓ Empty input rejected$(RESET)"
	@echo "0" | ./$(NAME) >/dev/null 2>&1 || echo "$(GREEN)✓ Zero ants rejected$(RESET)"
	@echo "abc" | ./$(NAME) >/dev/null 2>&1 || echo "$(GREEN)✓ Invalid ant count rejected$(RESET)"
	@printf "2\n##start\nstart 0 0\n##end\nend 1 1\nstart-end\n" | ./$(NAME) >/dev/null 2>&1 && echo "$(GREEN)✓ Simple valid case works$(RESET)" || echo "$(RED)✗ Simple case failed$(RESET)"

# Professional comprehensive test suite
test-full: $(NAME)
	@echo "$(BLUE)🚀 Running Comprehensive Test Suite$(RESET)"
	@if [ -f test_suite.sh ]; then \
		./test_suite.sh; \
	else \
		echo "$(RED)Error: test_suite.sh not found$(RESET)"; \
		echo "$(CYAN)Falling back to basic tests...$(RESET)"; \
		$(MAKE) test-basic test-files; \
	fi

# File-based testing (essential for validation)
test-files: test-valid test-invalid

test-invalid: $(NAME)
	@echo "\n$(YELLOW)❌ Invalid Maps$(RESET)"
	@for file in resources/invalid_maps/*; do \
		if [ -f "$$file" ]; then \
			./$(NAME) < "$$file" >/dev/null 2>&1 || echo "$(GREEN)✓ $$(basename $$file)$(RESET)"; \
		fi; \
	done

test-valid: $(NAME)
	@echo "\n$(YELLOW)✅ Valid Maps$(RESET)"
	@for file in resources/valid_maps/*; do \
		if [ -f "$$file" ]; then \
			./$(NAME) < "$$file" >/dev/null 2>&1 && echo "$(GREEN)✓ $$(basename $$file)$(RESET)" || echo "$(RED)✗ $$(basename $$file)$(RESET)"; \
		fi; \
	done

# Development and debugging targets
test-debug: debug
	@echo "$(YELLOW)🐛 Debug Testing$(RESET)"
	@echo -e "2\n##start\nstart 0 0\n##end\nend 1 1\nstart-end" | ./$(NAME)

test-memory: debug
	@echo "$(YELLOW)🧠 Memory Safety Test$(RESET)"
	@echo -e "3\n##start\nstart 0 0\nmiddle 1 1\n##end\nend 2 2\nstart-middle\nmiddle-end" | ./$(NAME) >/dev/null 2>&1 && echo "$(GREEN)✓ No memory issues$(RESET)" || echo "$(RED)✗ Memory issues detected$(RESET)"

test-performance: $(NAME)
	@echo "$(YELLOW)⚡ Performance Test$(RESET)"
	@if [ -f resources/slow_maps/bigsuplow ]; then \
		echo "Testing large map:"; \
		time ./$(NAME) < resources/slow_maps/bigsuplow >/dev/null 2>&1 && echo "$(GREEN)✓ Large map OK$(RESET)" || echo "$(YELLOW)⚠ Large map failed$(RESET)"; \
	else \
		echo "$(CYAN)No large test files found$(RESET)"; \
	fi

.PHONY: all clean fclean re test test-basic test-full test-invalid test-valid test-files test-debug test-memory test-performance debug release analyze
