CC       := clang
STD      := c17
WARN     := -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wstrict-prototypes
OPT_DBG  := -O0 -g3
SAN_DBG  := -fsanitize=address,undefined -fno-omit-frame-pointer
CFLAGS   := -std=$(STD) $(WARN) -Iinclude $(OPT_DBG) $(SAN_DBG)
LIBS     := -lcurses

SRC      := $(filter-out src/test_input.c,$(wildcard src/*.c))
OBJ_DIR  := build/obj
BIN_DIR  := build
OBJ      := $(patsubst src/%.c,$(OBJ_DIR)/%.o,$(SRC))
BIN      := $(BIN_DIR)/console-engine

.PHONY: all run clean tidy

all: $(BIN)

run: $(BIN)
	TERM=xterm-256color $(BIN)

$(OBJ_DIR)/%.o: src/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -DCE_NCURSES -c $< -o $@

$(BIN): $(OBJ)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

clean:
	rm -rf $(BIN_DIR)

tidy:
	rm -rf $(OBJ_DIR)
