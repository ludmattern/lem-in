NAME = visualizer

# Directories
SRCDIR = src
OBJDIR = objs
INCDIR = include

# Source files
SRCS = main.c init.c parser.c renderer.c animation.c

# Object files
OBJS = $(SRCS:%.c=$(OBJDIR)/%.o)

# Compiler and flags - Modern C with strict warnings
CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -pedantic
CFLAGS += -O2 -march=native  # Optimization
INCLUDES = -I$(INCDIR)

# SDL2 flags
SDL2_CFLAGS = $(shell pkg-config --cflags sdl2 sdl2_ttf)
SDL2_LIBS = $(shell pkg-config --libs sdl2 sdl2_ttf)

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
	@$(CC) $(CFLAGS) $(OBJS) $(SDL2_LIBS) -o $(NAME)
	@echo "$(GREEN)$(NAME) compiled successfully!$(RESET)"

$(OBJDIR):
	@mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@echo "$(BLUE)Compiling $<...$(RESET)"
	@$(CC) $(CFLAGS) $(INCLUDES) $(SDL2_CFLAGS) -c $< -o $@

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

.PHONY: all clean fclean re debug release 