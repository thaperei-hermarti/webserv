include config.mk

DEBUG ?= 0
ifeq ($(DEBUG), 1)
    CFLAGS += -g2 -O0
endif

SRC = $(SRC_FILES:%=$(SRC_DIR)/%)
OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
APP_OBJ = $(filter-out $(OBJ_DIR)/main.o,$(OBJ))
TEST_SRCS = $(shell find $(TEST_DIR) -type f -name '*.cpp' | sort)
TEST_OBJS = $(patsubst $(TEST_DIR)/%.cpp,$(OBJ_DIR)/test/%.o,$(TEST_SRCS))
HDR = $(shell find $(INC_DIR) -type f -name '*.hpp')

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) $(LIBS) -o $(NAME)
	@echo "[OK] $(NAME) compiled successfully"

test: $(TEST_NAME)

test-run: $(TEST_NAME)
	./$(TEST_NAME) --gtest_color=no

test-run-valgrind: $(TEST_NAME)
	@valgrind -q \
		--leak-check=full \
		--show-leak-kinds=all \
		--track-origins=yes \
		--track-fds=yes \
		./$(TEST_NAME) --gtest_color=no

check-deps:
	@sh scripts/check-deps.sh

$(TEST_NAME): check-deps $(TEST_OBJS) $(APP_OBJ)
	$(CC) $(TEST_CFLAGS) $(GTEST_CFLAGS) $(INCLUDES) $(TEST_OBJS) $(APP_OBJ) $(GTEST_LIBS) -o $(TEST_NAME)
	@echo "[OK] GoogleTest suite compiled successfully"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(HDR)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/test/%.o: $(TEST_DIR)/%.cpp $(HDR)
	@mkdir -p $(@D)
	$(CC) $(TEST_CFLAGS) $(GTEST_CFLAGS) $(INCLUDES) -c $< -o $@

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
	@for tool in clang-format clang-tidy valgrind; do \
		if command -v $$tool >/dev/null 2>&1; then \
			echo "[OK] $$tool found"; \
		else \
			echo "[WARNING] $$tool not found"; \
			echo "[INFO] Install it with: make setup"; \
			echo ""; \
		fi; \
	done
	@if command -v clang-format >/dev/null 2>&1 && command -v clang-tidy >/dev/null 2>&1 && command -v valgrind >/dev/null 2>&1; then \
		echo "[OK] All development tools are installed"; \
	else \
		echo "[INFO] Some tools are missing. Run 'make setup' to install them."; \
	fi

install-deps:
	@bash scripts/install-deps.sh

setup: check-tools install-deps check-deps
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

.PHONY: all clean fclean re bonus setup check-tools format format-check lint test test-run test-run-valgrind check-deps install-deps
