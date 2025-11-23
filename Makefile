# Makefile for Snake Game

CC = gcc
CXX = g++
CFLAGS = -Wall -Wextra -O2 -Iinclude
CXXFLAGS = -Wall -Wextra -O2 -Iinclude -std=c++14
LDFLAGS = -lncurses
TEST_LDFLAGS = -lgtest -lgtest_main -pthread

# Directories
SRC_DIR = src
INCLUDE_DIR = include
TEST_DIR = tests
BUILD_DIR = build

# Source files
GAME_SRC = $(SRC_DIR)/game.c
MAIN_SRC = $(SRC_DIR)/main.c
TEST_SRC = $(TEST_DIR)/test_snake.cpp

# Object files
GAME_OBJ = $(BUILD_DIR)/game.o
MAIN_OBJ = $(BUILD_DIR)/main.o
TEST_OBJ = $(BUILD_DIR)/test_snake.o

# Executables
GAME_BIN = snake
TEST_BIN = test_snake

.PHONY: all clean test run dirs cmake cmake-build cmake-test

all: dirs $(GAME_BIN)

dirs:
	@mkdir -p $(BUILD_DIR)

# Build game library object
$(GAME_OBJ): $(GAME_SRC) $(INCLUDE_DIR)/snake.h
	$(CC) $(CFLAGS) -c $(GAME_SRC) -o $(GAME_OBJ)

# Build main executable
$(MAIN_OBJ): $(MAIN_SRC) $(INCLUDE_DIR)/snake.h
	$(CC) $(CFLAGS) -c $(MAIN_SRC) -o $(MAIN_OBJ)

# Link game executable
$(GAME_BIN): $(GAME_OBJ) $(MAIN_OBJ)
	$(CC) $(GAME_OBJ) $(MAIN_OBJ) -o $(GAME_BIN) $(LDFLAGS)
	@echo "✓ Snake game compiled successfully!"

# Build and run tests
test: dirs $(GAME_OBJ)
	@echo "Building tests..."
	$(CXX) $(CXXFLAGS) $(TEST_SRC) $(GAME_OBJ) -o $(TEST_BIN) $(TEST_LDFLAGS)
	@echo "✓ Tests compiled successfully!"
	@echo ""
	@echo "Running tests..."
	@./$(TEST_BIN)

# Run game
run: $(GAME_BIN)
	./$(GAME_BIN)

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) $(GAME_BIN) $(TEST_BIN)
	rm -rf cmake-build
	@echo "✓ Cleaned build files"

# Rebuild everything
rebuild: clean all

# CMake build (recommended for tests)
cmake-build:
	@mkdir -p cmake-build
	cd cmake-build && cmake .. && cmake --build .
	@echo "✓ CMake build complete!"
	@echo "Executables: cmake-build/snake, cmake-build/test_snake"

cmake-test: cmake-build
	cd cmake-build && ctest --output-on-failure

# Help
help:
	@echo "Snake Game Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  make          - Build the game"
	@echo "  make test     - Build and run tests"
	@echo "  make run      - Build and run the game"
	@echo "  make clean    - Remove build artifacts"
	@echo "  make rebuild  - Clean and rebuild"
	@echo "  make cmake-build - Build with CMake"
	@echo "  make cmake-test  - Run tests with CMake/CTest"
	@echo ""
