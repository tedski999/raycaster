
TARGET  = raycaster
CC      = gcc
CFLAGS  = -Wall -pedantic -O3
LFLAGS  = -lm -ldl -lglfw
SRC_FILES = src/*.c

.PHONY: all
all: out/$(TARGET)
	@echo "Build complete."

.PHONY: clean
clean:
	@echo "Removing build directories..."
	@rm -rf obj out

obj/%.o: src/%.c Makefile
	@echo "Compiling $< -> $@"
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -c $< -o $@

out/$(TARGET): $(SRC_FILES:src/%.c=obj/%.o)
	@echo "Linking $@..."
	@mkdir -p $(@D)
	@$(CC) $^ $(LFLAGS) -o $@

