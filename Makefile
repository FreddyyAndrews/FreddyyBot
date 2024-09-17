# Define compiler and flags
CXX = g++
CXXFLAGS = -pedantic-errors -Wall -Weffc++ -Wextra -Wconversion -Wsign-conversion -std=c++23

# Include Conan generated dependencies
include conandeps.mk

# Add Conan flags to your build
CXXFLAGS += $(CONAN_INCLUDE_DIRS)
LDFLAGS += $(CONAN_LIB_DIRS)
LIBS += $(CONAN_LIBS) $(CONAN_SYSTEM_LIBS)

# Directories
SRC_DIR = src/manager
BUILD_DIR = build

# Source files
SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)

# Output binary
MANAGER_BIN = $(BUILD_DIR)/manager

# Create the build directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build the manager executable
$(MANAGER_BIN): $(SRC_FILES) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(SRC_FILES) -o $(MANAGER_BIN) $(LDFLAGS) $(LIBS)

# Run the manager
.PHONY: run
run: $(MANAGER_BIN)
	./$(MANAGER_BIN)

# Clean build artifacts
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
