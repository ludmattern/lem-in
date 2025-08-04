.PHONY: all clean fclean re test release bonus visualizer libft

CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -MMD -MP
CFLAGS += -O2 -march=native
CFLAGS += -Wformat=2 -Wformat-security -Wcast-align -Wpointer-arith
CFLAGS += -Wwrite-strings -Wmissing-prototypes -Wstrict-prototypes
CFLAGS += -fstack-protector-strong -D_FORTIFY_SOURCE=2

SRCS_DIR = srcs
INCLUDE_DIR = include
OBJS_DIR = objs
LIBFTDIR = libft

SRC_FILES = main parser parse_line input validator hash error output pathfinding
SRCS = $(addprefix $(SRCS_DIR)/,$(addsuffix .c,$(SRC_FILES)))
OBJS = $(patsubst $(SRCS_DIR)/%.c,$(OBJS_DIR)/%.o,$(SRCS))
DEPS = $(OBJS:.o=.d)
INCLUDES = -I$(INCLUDE_DIR) -I$(LIBFTDIR)/inc
LIBS = -L$(LIBFTDIR) -lft
NAME = lem-in

# Colors for output
GREEN = \033[0;32m
RED = \033[0;31m
BLUE = \033[0;34m
RESET = \033[0m

# Rules
all: $(NAME)

$(NAME): libft $(OBJS)
	@echo "$(BLUE)Linking $(NAME)...$(RESET)"
	@$(CC) $(CFLAGS) $(INCLUDES) -o $@ $(OBJS) $(LIBS)
	@echo "$(GREEN)$(NAME) compiled successfully!$(RESET)"

$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.c | $(OBJS_DIR)
	@echo "$(BLUE)Compiling $<...$(RESET)"
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJS_DIR):
	@mkdir -p $(OBJS_DIR)

libft:
	@echo "$(BLUE)Compiling libft...$(RESET)"
	@$(MAKE) -C $(LIBFTDIR) --no-print-directory > /dev/null

release: CFLAGS = -Wall -Wextra -Werror -std=c11 -O3 -DNDEBUG -MMD -MP
release: fclean libft $(NAME)

# Bonus rule - compile both lem-in and visualizer
bonus: $(NAME)
	@echo "$(BLUE)Compiling visualizer...$(RESET)"
	@cd visualizer && make > /dev/null
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

clean:
	@echo "$(RED)Removing object files...$(RESET)"
	@rm -f $(OBJS) $(DEPS)
	@rm -rf $(OBJS_DIR)
	@$(MAKE) -C $(LIBFTDIR) clean --no-print-directory > /dev/null 2>&1
	@cd visualizer && make clean > /dev/null 2>&1

fclean: clean
	@echo "$(RED)Removing $(NAME)...$(RESET)"
	@rm -f $(NAME)
	@$(MAKE) -C $(LIBFTDIR) fclean --no-print-directory > /dev/null 2>&1
	@cd visualizer && make fclean > /dev/null 2>&1

re: fclean all

test: $(NAME)
	@echo "$(BLUE)Running comprehensive test suite...$(RESET)"
	@./test_suite.sh -v

-include $(DEPS)
