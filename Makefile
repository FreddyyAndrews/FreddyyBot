# Compiler and flags
CXX = g++
CXXFLAGS = -O3 -pedantic-errors -Wall -Weffc++ -Wextra -Wconversion -Wsign-conversion -Werror -std=c++23

# Include directories
INCLUDE_DIRS = -Iinclude -isystem /usr/src/googletest/googletest/include

# Library directories
LIB_DIRS = -L/usr/lib

# Libraries for the main program
MAIN_LIBS = -pthread # -ltinfo  -lncursesw

# Libraries for test executables (include gtest_main)
TEST_LIBS = -lgtest_main -lgtest -pthread -ltinfo # -lncursesw

# Directories
SRC_DIR := src
TEST_DIR := $(SRC_DIR)/tests
BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj
BIN_DIR := $(BUILD_DIR)/bin

# Source files and object files
SOURCES := $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SOURCES))

# Test source files and object files
TEST_SOURCES := $(wildcard $(TEST_DIR)/*.cpp)
TEST_OBJECTS := $(patsubst $(TEST_DIR)/%.cpp, $(OBJ_DIR)/tests/%.o, $(TEST_SOURCES))
TEST_EXECUTABLES := $(patsubst $(TEST_DIR)/%.cpp, $(BIN_DIR)/tests/%, $(TEST_SOURCES))

# Common objects (excluding main.o)
COMMON_OBJECTS := $(filter-out $(OBJ_DIR)/main.o, $(OBJECTS))

# Default target
.PHONY: all
all: $(BIN_DIR)/main

# Rule to build the main executable
$(BIN_DIR)/main: $(OBJECTS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) $(OBJECTS) -o $@ $(LIB_DIRS) $(MAIN_LIBS)

# Rule to compile source files into object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) -c $< -o $@

# Rule to compile test source files into object files
$(OBJ_DIR)/tests/%.o: $(TEST_DIR)/%.cpp | $(OBJ_DIR)/tests
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) -c $< -o $@

# Rule to build test executables
$(BIN_DIR)/tests/%: $(OBJ_DIR)/tests/%.o $(COMMON_OBJECTS) | $(BIN_DIR)/tests
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) $^ -o $@ $(LIB_DIRS) $(TEST_LIBS)

# Directory creation
$(BIN_DIR) $(BIN_DIR)/tests $(OBJ_DIR) $(OBJ_DIR)/tests:
	mkdir -p $@

# Phony targets
.PHONY: clean test run

# Rule to clean the build directory
clean:
	rm -rf $(BUILD_DIR)

# Rule to build and run all tests
test: $(TEST_EXECUTABLES)
	@for test_exec in $(TEST_EXECUTABLES); do \
		echo "Running $$test_exec"; \
		$$test_exec || exit 1; \
	done

# Rule to build and run the main program
run: $(BIN_DIR)/main
	$(BIN_DIR)/main

# Rule to build the main executable without running it
build: $(BIN_DIR)/main
