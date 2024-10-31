# Define compiler and flags
CXX = g++
CXXFLAGS = -pedantic-errors -Wall -Weffc++ -Wextra -Wconversion -Wsign-conversion -Werror -std=c++23

# Include directories (use -isystem for GTest to suppress warnings from GTest)
INCLUDE_DIRS = -Iinclude -isystem /usr/src/googletest/googletest/include

# GTest library directories and libraries
LIB_DIRS = -L/usr/lib
LIBS = -lgtest -lgtest_main -pthread -lncursesw -ltinfo

# Directories
# Manager directories
MANAGER_SRC_DIR = src/manager
MANAGER_TEST_DIR = src/manager/tests
MANAGER_BUILD_DIR = build/manager

# Engine directories
ENGINE_SRC_DIR = src/engine
ENGINE_TEST_DIR = src/engine/tests
ENGINE_BUILD_DIR = build/engine

# Shared source files (if any)
SHARED_SRC_FILES = src/engine/move_generator.cpp

# Source files
# Manager source files
MANAGER_SRC_FILES = $(wildcard $(MANAGER_SRC_DIR)/*.cpp) $(SHARED_SRC_FILES)
MANAGER_TEST_FILES = $(wildcard $(MANAGER_TEST_DIR)/*.cpp)

# Engine source files
ENGINE_SRC_FILES = $(wildcard $(ENGINE_SRC_DIR)/*.cpp)
ENGINE_TEST_FILES = $(wildcard $(ENGINE_TEST_DIR)/*.cpp)

# Output binaries
# Manager binaries
MANAGER_BIN = $(MANAGER_BUILD_DIR)/manager
MANAGER_TEST_BIN = $(MANAGER_BUILD_DIR)/tests

# Engine binaries
ENGINE_BIN = $(ENGINE_BUILD_DIR)/engine
ENGINE_TEST_BIN = $(ENGINE_BUILD_DIR)/tests

# Create the build directories if they don't exist
$(MANAGER_BUILD_DIR):
	mkdir -p $(MANAGER_BUILD_DIR)

$(ENGINE_BUILD_DIR):
	mkdir -p $(ENGINE_BUILD_DIR)

# Build the manager executable
manager: $(MANAGER_BIN)

$(MANAGER_BIN): $(MANAGER_SRC_FILES) | $(MANAGER_BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) $(MANAGER_SRC_FILES) -o $(MANAGER_BIN)

# Build the manager test executable
manager_test: $(MANAGER_TEST_BIN)

$(MANAGER_TEST_BIN): $(MANAGER_TEST_FILES) $(filter-out src/manager/main.cpp, $(MANAGER_SRC_FILES)) | $(MANAGER_BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) $(MANAGER_TEST_FILES) $(filter-out src/manager/main.cpp, $(MANAGER_SRC_FILES)) $(LIB_DIRS) $(LIBS) -o $(MANAGER_TEST_BIN)

# Build the engine executable
engine: $(ENGINE_BIN)

$(ENGINE_BIN): $(ENGINE_SRC_FILES) | $(ENGINE_BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) $(ENGINE_SRC_FILES) -o $(ENGINE_BIN)

# Build the engine test executable
engine_test: $(ENGINE_TEST_BIN)

$(ENGINE_TEST_BIN): $(ENGINE_TEST_FILES) $(ENGINE_SRC_FILES) | $(ENGINE_BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) $(ENGINE_TEST_FILES) $(ENGINE_SRC_FILES) $(LIB_DIRS) $(LIBS) -o $(ENGINE_TEST_BIN)

# Run the manager
.PHONY: manager_run
manager_run: manager
	./$(MANAGER_BIN)

# Run the manager tests
.PHONY: manager_test_run
manager_test_run: manager_test
	./$(MANAGER_TEST_BIN)

# Run the engine
.PHONY: engine_run
engine_run: engine
	./$(ENGINE_BIN)

# Run the engine tests
.PHONY: engine_test_run
engine_test_run: engine_test
	./$(ENGINE_TEST_BIN)

# Run all tests
.PHONY: test_run_all
test_run_all: manager_test_run engine_test_run

# Clean manager build artifacts
.PHONY: manager_clean
manager_clean:
	rm -rf $(MANAGER_BUILD_DIR)

# Clean engine build artifacts
.PHONY: engine_clean
engine_clean:
	rm -rf $(ENGINE_BUILD_DIR)

# General clean
.PHONY: clean
clean:
	rm -rf build
