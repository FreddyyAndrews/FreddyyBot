# Define compiler and flags
CXX = g++
CXXFLAGS = -pedantic-errors -Wall -Weffc++ -Wextra -Wconversion -Wsign-conversion -Werror -std=c++23

# Include directories (use -isystem for GTest to suppress warnings from GTest)
INCLUDE_DIRS = -Iinclude -isystem /usr/src/googletest/googletest/include

# GTest library directories and libraries
LIB_DIRS = -L/usr/lib
LIBS = -lgtest -lgtest_main -pthread -lncursesw -ltinfo

# Directories
SRC_DIR = src
TEST_DIR = src/tests
BUILD_DIR = build/


# Source files
SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp) $(SHARED_SRC_FILES)
TEST_FILES = $(wildcard $(TEST_DIR)/*.cpp)

# Output binaries
BIN = $(BUILD_DIR)
TEST_BIN = $(BUILD_DIR)/tests

# Create the build directories if they don't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build the manager executable
manager: $(MANAGER_BIN)

$(MANAGER_BIN): $(MANAGER_SRC_FILES) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) $(MANAGER_SRC_FILES) -o $(MANAGER_BIN)

# Build the manager test executable
manager_test: $(MANAGER_TEST_BIN)

$(MANAGER_TEST_BIN): $(MANAGER_TEST_FILES) $(filter-out src/manager/main.cpp, $(MANAGER_SRC_FILES)) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) $(MANAGER_TEST_FILES) $(filter-out src/manager/main.cpp, $(MANAGER_SRC_FILES)) $(LIB_DIRS) $(LIBS) -o $(MANAGER_TEST_BIN)


# Run the manager
.PHONY: manager_run
manager_run: manager
	./$(MANAGER_BIN)

# Run the manager tests
.PHONY: manager_test_run
manager_test_run: manager_test
	./$(MANAGER_TEST_BIN)


# Clean manager build artifacts
.PHONY: manager_clean
manager_clean:
	rm -rf $(BUILD_DIR)

# General clean
.PHONY: clean
clean:
	rm -rf build
