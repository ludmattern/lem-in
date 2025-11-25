# ================================ TARGETS =================================== #
.PHONY: all clean fclean re test big-test ultra-test release debug help
.PHONY: test-big-superposition test-big test-flow-one test-flow-ten test-flow-thousand
.PHONY: libft libft-clean libft-fclean
.PHONY: bonus
.DEFAULT_GOAL := all

# =============================== COMPILER ================================== #
CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -MMD -MP
CFLAGS += -O2 -march=native
CFLAGS += -Wformat=2 -Wformat-security -Wcast-align -Wpointer-arith
CFLAGS += -Wwrite-strings -Wmissing-prototypes -Wstrict-prototypes
CFLAGS += -fstack-protector-strong

DEBUG_FLAGS = -g3 -DDEBUG=1 -fsanitize=address,undefined
RELEASE_FLAGS = -O3 -DNDEBUG -D_FORTIFY_SOURCE=2

# ============================= DIRECTORIES ================================ #
BUILD_DIR = build
LIBFT_DIR = libft

# Lem-in paths
LEMIN_SRC_DIR = srcs
LEMIN_INC_DIR = include
LEMIN_OBJ_DIR = $(BUILD_DIR)/lem-in
LEMIN_TARGET = lem-in

# Visualizer paths
VIS_SRC_DIR = visualizer/src
VIS_INC_DIR = visualizer/include
VIS_OBJ_DIR = $(BUILD_DIR)/visualizer
VIS_TARGET = visualizer/visualizer

# ================================ SOURCES =================================== #
LEMIN_SRCS = \
	main.c \
	parser.c \
	parse_line.c \
	input.c \
	validator.c \
	hash.c \
	error.c \
	display.c \
	init.c \
	output.c \
	cleaner.c \
	graph_builder.c \
	bfs.c \
	paths_finder.c \
	solver.c
LEMIN_OBJS = $(addprefix $(LEMIN_OBJ_DIR)/,$(LEMIN_SRCS:.c=.o))
LEMIN_DEPS = $(LEMIN_OBJS:.o=.d)

VIS_SRCS = main.c init.c parser.c renderer.c animation.c
VIS_OBJS = $(addprefix $(VIS_OBJ_DIR)/,$(VIS_SRCS:.c=.o))
VIS_DEPS = $(VIS_OBJS:.o=.d)

# =============================== LIBRARIES ================================= #
LIBFT = $(LIBFT_DIR)/libft.a
LEMIN_INCLUDES = -I$(LEMIN_INC_DIR) -I$(LIBFT_DIR)/inc
LEMIN_LIBS = -L$(LIBFT_DIR) -lft

VIS_INCLUDES = -I$(VIS_INC_DIR) -I$(LIBFT_DIR)/inc
VIS_SDL_CFLAGS = $(shell sdl-config --cflags 2>/dev/null || echo "")
VIS_SDL_LIBS = $(shell sdl-config --libs 2>/dev/null || echo "-lSDL") -lSDL_ttf

# ================================= COLORS =================================== #
RESET = \033[0m
BOLD = \033[1m
RED = \033[31m
GREEN = \033[32m
YELLOW = \033[33m
BLUE = \033[34m
MAGENTA = \033[35m
CYAN = \033[36m

# =============================== MESSAGES ================================== #
MSG_COMPILE = $(CYAN)[COMPILE]$(RESET)
MSG_LINK = $(GREEN)[LINK]$(RESET)
MSG_CLEAN = $(YELLOW)[CLEAN]$(RESET)
MSG_INFO = $(BLUE)[INFO]$(RESET)
MSG_SUCCESS = $(GREEN)[SUCCESS]$(RESET)
MSG_ERROR = $(RED)[ERROR]$(RESET)
MSG_WARNING = $(YELLOW)[WARNING]$(RESET)

# ============================= MAIN TARGETS =============================== #
all:
	@if [ ! -f "$(LEMIN_TARGET)" ]; then \
		$(MAKE) $(LEMIN_TARGET); \
	elif $(MAKE) -q $(LEMIN_TARGET); then \
		printf "$(MSG_INFO) No files to compile, $(BOLD)lem-in$(RESET) is already up to date.\n"; \
	else \
		$(MAKE) $(LEMIN_TARGET); \
	fi

$(LEMIN_TARGET): $(LIBFT) $(LEMIN_OBJS)
	@printf "$(MSG_LINK) Linking $(BOLD)$@$(RESET)...\n"
	@$(CC) $(CFLAGS) $(LEMIN_OBJS) $(LEMIN_LIBS) -o $@
	@printf "$(MSG_SUCCESS) $(BOLD)$@$(RESET) compiled successfully!\n"

visualizer: $(VIS_TARGET)

$(VIS_TARGET): $(LIBFT) $(VIS_OBJS)
	@printf "$(MSG_LINK) Linking $(BOLD)$@$(RESET)...\n"
	@$(CC) $(CFLAGS) $(VIS_OBJS) $(VIS_SDL_LIBS) $(LIBFT) -o $@
	@printf "$(MSG_SUCCESS) $(BOLD)$@$(RESET) compiled successfully!\n"

bonus: $(LEMIN_TARGET) $(VIS_TARGET)
	@printf "$(MSG_SUCCESS) Bonus targets compiled successfully!\n"

# ============================== BUILD RULES =============================== #
$(LEMIN_OBJ_DIR)/%.o: $(LEMIN_SRC_DIR)/%.c | $(LEMIN_OBJ_DIR)
	@printf "$(MSG_COMPILE) $<\n"
	@$(CC) $(CFLAGS) $(LEMIN_INCLUDES) -c $< -o $@

$(VIS_OBJ_DIR)/%.o: $(VIS_SRC_DIR)/%.c | $(VIS_OBJ_DIR)
	@printf "$(MSG_COMPILE) $<\n"
	@$(CC) $(CFLAGS) $(VIS_INCLUDES) $(VIS_SDL_CFLAGS) -c $< -o $@

# =========================== DIRECTORY CREATION ============================ #
$(BUILD_DIR):
	@mkdir -p $@

$(LEMIN_OBJ_DIR): | $(BUILD_DIR)
	@mkdir -p $@

$(VIS_OBJ_DIR): | $(BUILD_DIR)
	@mkdir -p $@

# ============================== LIBFT RULES =============================== #
$(LIBFT):
	@printf "$(MSG_INFO) Building libft...\n"
	@$(MAKE) -C $(LIBFT_DIR) --no-print-directory

libft-clean:
	@$(MAKE) -C $(LIBFT_DIR) clean --no-print-directory

libft-fclean:
	@$(MAKE) -C $(LIBFT_DIR) fclean --no-print-directory

# ============================ BUILD VARIANTS ============================== #
debug: CFLAGS += $(DEBUG_FLAGS)
debug: $(LEMIN_TARGET)
	@printf "$(MSG_SUCCESS) Debug build completed!\n"

release: CFLAGS += $(RELEASE_FLAGS)
release: fclean $(LEMIN_TARGET)
	@printf "$(MSG_SUCCESS) Release build completed!\n"

# ============================== UTILITIES ================================== #
run: $(LEMIN_TARGET)
	@if [ -z "$(MAP)" ]; then \
		printf "$(MSG_ERROR) Please specify a map file with MAP=path/to/map\n"; \
		printf "Example: make run MAP=resources/valid_maps/simple_test\n"; \
		exit 1; \
	fi
	@printf "$(MSG_INFO) Running $(LEMIN_TARGET) with map: $(MAP)\n"
	@./$(LEMIN_TARGET) < $(MAP)

viz: bonus
	@if [ -z "$(MAP)" ]; then \
		printf "$(MSG_ERROR) Please specify a map file with MAP=path/to/map\n"; \
		printf "Example: make viz MAP=resources/valid_maps/simple_test\n"; \
		exit 1; \
	fi
	@if [ ! -f "./$(VIS_TARGET)" ]; then \
		printf "$(MSG_ERROR) Visualizer not found. Please run 'make bonus' first.\n"; \
		exit 1; \
	fi
	@printf "$(MSG_INFO) Running visualizer with map: $(MAP)\n"
	@./$(LEMIN_TARGET) < $(MAP) 2>&1 | ./$(VIS_TARGET)

test: $(LEMIN_TARGET)
	@printf "$(MSG_INFO) Testing maps in $(BOLD)resources/all_generated$(RESET)...\n"
	@set -e; \
	dir="resources/all_generated"; \
	if [ ! -d "$$dir" ]; then \
		printf "$(MSG_ERROR) Directory '%s' not found\n" "$$dir"; exit 1; \
	fi; \
	rc=0; \
	for map in "$$dir"/*; do \
		[ -f "$$map" ] || continue; \
		req=$$(grep -m1 -E '^#Here is the number of lines required: *[0-9]+' "$$map" | sed -E 's/.*: *([0-9]+).*/\1/'); \
		if [ -z "$$req" ]; then \
			printf "$(MSG_ERROR) %-40s -> missing required-lines header\n" "$$(basename "$$map")"; \
			rc=1; \
			continue; \
		fi; \
		got=$$(./$(LEMIN_TARGET) < "$$map" 2>/dev/null | awk -F': ' '/^# Number of lines: /{v=$$2} END{if (v) print v}'); \
		if [ -z "$$got" ]; then \
			printf "$(MSG_ERROR) %-40s -> no result found (got N/A, perfect=%s)\n" "$$(basename "$$map")" "$$req"; \
			rc=1; \
			continue; \
		fi; \
		if [ "$$got" -le "$$req" ]; then \
			printf "$(MSG_SUCCESS) %-40s -> SUCCESS (perfect=%s, got=%s)\n" "$$(basename "$$map")" "$$req" "$$got"; \
		else \
			printf "$(MSG_WARNING) %-40s -> WARNING   (perfect=%s, got=%s)\n" "$$(basename "$$map")" "$$req" "$$got"; \
			rc=1; \
		fi; \
	done; \
	exit $$rc

big-test: $(LEMIN_TARGET)
	@printf "$(MSG_INFO) Running big test suite (generates 10x each map style)...\n"
	@if [ ! -f "scripts/big_test.sh" ]; then \
		printf "$(MSG_ERROR) Script big_test.sh not found\n"; exit 1; \
	fi
	@bash scripts/big_test.sh

ultra-test: $(LEMIN_TARGET)
	@printf "$(MSG_INFO) Running ULTRA test suite (100 big-superposition maps)...\n"
	@if [ ! -f "scripts/ultra_test.sh" ]; then \
		printf "$(MSG_ERROR) Script ultra_test.sh not found\n"; exit 1; \
	fi
	@bash scripts/ultra_test.sh

# =============================== CLEANING ================================== #
clean: libft-clean
	@printf "$(MSG_CLEAN) Removing object files...\n"
	@rm -rf $(BUILD_DIR)

fclean: clean libft-fclean
	@printf "$(MSG_CLEAN) Removing executables...\n"
	@rm -f $(LEMIN_TARGET) $(VIS_TARGET)

re: fclean all

# ================================= HELP ==================================== #
help:
	@printf "$(BOLD)Available targets:$(RESET)\n"
	@printf "  $(GREEN)all$(RESET)        - Build lem-in (default)\n"
	@printf "  $(GREEN)visualizer$(RESET) - Build visualizer only\n"
	@printf "  $(GREEN)bonus$(RESET)      - Build both lem-in and visualizer\n"
	@printf "  $(GREEN)debug$(RESET)      - Build with debug flags\n"
	@printf "  $(GREEN)release$(RESET)    - Build optimized release version\n"
	@printf "  $(GREEN)test$(RESET)       - Run test suite\n"
	@printf "  $(GREEN)big-test$(RESET)   - Generate and test 10x each map style\n"
	@printf "  $(GREEN)ultra-test$(RESET) - Generate and test 100 big-superposition maps\n"
	@printf "  $(GREEN)run$(RESET)        - Run lem-in with MAP=<file>\n"
	@printf "  $(GREEN)viz$(RESET)        - Run visualizer with MAP=<file>\n"
	@printf "  $(GREEN)clean$(RESET)      - Remove object files\n"
	@printf "  $(GREEN)fclean$(RESET)     - Remove all generated files\n"
	@printf "  $(GREEN)re$(RESET)         - Rebuild everything\n"
	@printf "  $(GREEN)help$(RESET)       - Show this help\n"
	@printf "\n$(BOLD)Examples:$(RESET)\n"
	@printf "  make run MAP=resources/valid_maps/simple_test\n"
	@printf "  make viz MAP=resources/valid_maps/complex_test\n"

# ========================== DEPENDENCY INCLUSION =========================== #
-include $(LEMIN_DEPS)
-include $(VIS_DEPS)
