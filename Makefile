include config.mk

DEBUG ?= 0
ifeq ($(DEBUG), 1)
    CFLAGS += -g2 -O0
endif

SRC = $(SRC_FILES:%=$(SRC_DIR)/%)
OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
HDR = $(wildcard $(INC_DIR)/*.hpp)

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) $(LIBS) -o $(NAME)
	@echo "[OK] $(NAME) compiled successfully"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(HDR)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

valgrind-run:
	@valgrind -q\
		--leak-check=full \
		--show-leak-kinds=all \
		--track-origins=yes \
		--track-fds=yes \
		--trace-children=yes \
		--trace-children-skip='*/bin/*,*/sbin/*,/usr/bin/*' \
		./$(NAME)

clean:
	@rm -rf $(OBJ_DIR)
	@echo "[OK] Object files removed"

fclean: clean
	@rm -f $(NAME)
	@echo "[OK] $(NAME) removed"

re: fclean all

check-tools:
	@echo "[INFO] Checking for development tools..."
	@for tool in clang-format clang-tidy; do \
		if command -v $$tool >/dev/null 2>&1; then \
			echo "[OK] $$tool found"; \
		else \
			echo "[WARNING] $$tool not found"; \
			echo "[INFO] Install with: sudo apt-get install $$tool"; \
			echo ""; \
		fi; \
	done
	@if command -v clang-format >/dev/null 2>&1 && command -v clang-tidy >/dev/null 2>&1; then \
		echo "[OK] All development tools are installed"; \
	else \
		echo "[INFO] Some tools are missing. Install them for full functionality."; \
	fi

setup: check-tools
	@bash scripts/setup-hooks.sh

format:
	@clang-format -i $(SRC) $(HDR)
	@echo "[OK] All files formatted"

format-check:
	@clang-format --dry-run -Werror $(SRC) $(HDR)
	@echo "[OK] All files are properly formatted"

lint:
	@clang-tidy $(SRC) -- $(CFLAGS) $(INCLUDES)
	@echo "[OK] clang-tidy found no issues"

.PHONY: all clean fclean re bonus setup check-tools format format-check lint
