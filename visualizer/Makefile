.PHONY: all clean fclean re debug release

CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -MMD -MP
CFLAGS += -O2 -march=native
CFLAGS += -Wformat=2 -Wformat-security -Wcast-align -Wpointer-arith
CFLAGS += -Wwrite-strings -Wmissing-prototypes -Wstrict-prototypes
CFLAGS += -fstack-protector-strong -D_FORTIFY_SOURCE=2

SRCS_DIR = ./src
INCLUDE_DIR = ./include
OBJS_DIR = ./objs

SRC_FILES = main init parser renderer animation
SRCS = $(addprefix $(SRCS_DIR)/,$(addsuffix .c,$(SRC_FILES)))
OBJS = $(patsubst $(SRCS_DIR)/%.c,$(OBJS_DIR)/%.o,$(SRCS))
DEPS = $(OBJS:.o=.d)
INCLUDES = -I$(INCLUDE_DIR)
NAME = visualizer

# SDL1 flags
SDL_CFLAGS = $(shell sdl-config --cflags)
SDL_LIBS = $(shell sdl-config --libs) -lSDL_ttf

# Colors for output
GREEN = \033[0;32m
RED = \033[0;31m
BLUE = \033[0;34m
RESET = \033[0m

# Rules
all: $(NAME)

$(NAME): $(OBJS)
	@echo "$(BLUE)Linking $(NAME)...$(RESET)"
	@$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) $(SDL_LIBS) $(LIBFTDIR)/libft.a -o $(NAME)
	@echo "$(GREEN)$(NAME) compiled successfully!$(RESET)"

$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.c | $(OBJS_DIR)
	@echo "$(BLUE)Compiling $<...$(RESET)"
	@$(CC) $(CFLAGS) $(INCLUDES) $(SDL_CFLAGS) -c $< -o $@

$(OBJS_DIR):
	@mkdir -p $(OBJS_DIR)

# Build variants
debug: CFLAGS += -g3 -DDEBUG -fsanitize=address,undefined
debug: $(NAME)

release: CFLAGS = -Wall -Wextra -Werror -std=c11 -O3 -DNDEBUG -MMD -MP
release: fclean $(NAME)

# Clean targets  
clean:
	@echo "$(RED)Removing object files...$(RESET)"
	@rm -f $(OBJS) $(DEPS)
	@rm -rf $(OBJS_DIR)

fclean: clean
	@echo "$(RED)Removing $(NAME)...$(RESET)"
	@rm -f $(NAME)

re: fclean all

-include $(DEPS) 