CXX := g++
CXXFLAGS := -Wall -Wextra -std=c++17 -pthread # Adjust the flags as needed
SRC_DIR := .
BUILD_DIR := ./build

# Find all source files in the current directory
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)

# Generate object file names based on source file names
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRC_FILES))

# Output executable name
EXECUTABLE := $(BUILD_DIR)/dlp_controller

# Rule to build executable
$(EXECUTABLE): $(OBJ_FILES)
	$(CXX) $^ -o $@ -lsfml-graphics -lsfml-window -lsfml-system -pthread

# Rule to build object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Phony target to clean build artifacts
.PHONY: clean
clean:
	rm -r $(BUILD_DIR)

# Ensure the build directory exists
$(shell mkdir -p $(BUILD_DIR))