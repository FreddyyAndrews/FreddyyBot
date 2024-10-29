# Define compiler and flags
CXX = g++
CXXFLAGS = -pedantic-errors -Wall -Weffc++ -Wextra -Wconversion -Wsign-conversion -Werror -std=c++23
# CXXFLAGS = -std=c++23

# Include directories (use -isystem for GTest to suppress warnings from GTest)
INCLUDE_DIRS = -Iinclude -isystem /usr/src/googletest/googletest/include

# GTest library directories and libraries
LIB_DIRS = -L/usr/lib
LIBS = -lgtest -lgtest_main -pthread -lncursesw -ltinfo

# Directories
SRC_DIR = src/manager
TEST_DIR = src/manager/tests
BUILD_DIR = build/manager

# Source files
SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)
TEST_FILES = $(wildcard $(TEST_DIR)/*.cpp)

# Output binaries
MANAGER_BIN = $(BUILD_DIR)/manager
TEST_BIN = $(BUILD_DIR)/tests

# Create the build directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build the manager executable
manager: $(MANAGER_BIN)

$(MANAGER_BIN): $(SRC_FILES) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) $(SRC_FILES) -o $(MANAGER_BIN)

# Build the test executable
manager_test: $(TEST_BIN)

$(TEST_BIN): $(TEST_FILES) $(filter-out src/manager/main.cpp, $(SRC_FILES)) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) $(TEST_FILES) $(filter-out src/manager/main.cpp, $(SRC_FILES)) $(LIB_DIRS) $(LIBS) -o $(TEST_BIN)

# Run the manager
.PHONY: manager_run
manager_run: $(MANAGER_BIN)
	./$(MANAGER_BIN)

# Run the tests
.PHONY: manager_test
manager_test_run: $(TEST_BIN)
	./$(TEST_BIN)

# Clean manager build artifacts
.PHONY: manager_clean
manager_clean:
	rm -rf $(BUILD_DIR)

# General clean
.PHONY: clean
clean:
	rm -rf build
