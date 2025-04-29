## This is a custom makefile built with the help of an LLM because I don't know how makefiles work and have been using the same one for 10 years. 
## Fingers crossed.

# Project Names
PROJECT = ramices

# Compiler
CC = g++

# Compiler and Linker Options
CXXFLAGS = -std=c++20 -pthread -O3 -Wall
LDFLAGS = -lpthread

# Dependency Flags
DEPFLAGS = -MMD -MP

# Source and Build Directories
SRC_DIR = src
BUILD_DIR = build

### DON'T EDIT BELOW HERE


# Find all source files
SOURCE_FILES := $(shell find $(SRC_DIR) -name "*.cpp")

# Generate object files
OBJECTS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/main/%.o, $(SOURCE_FILES))

# Dependency Files
DEPS := $(OBJECTS:.o=.d)


# Default Target: Build both projects
.PHONY: all
all: $(PROJECT)

# Build Main Project
$(PROJECT): $(OBJECTS)
	@echo "Linking $(PROJECT)..."
	$(CC) -o $@ $^ $(LDFLAGS)

# Compile Main Source Files
$(BUILD_DIR)/main/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "Compiling $< for $(PROJECT)..."
	$(CC) $(CXXFLAGS) $(DEPFLAGS) -c -o $@ $<

# Include Dependencies
-include $(DEPS)

# Run Main Project
.PHONY: run
run: $(PROJECT)
	./$(PROJECT)

# Clean Targets
.PHONY: clean
clean:
	@echo "Cleaning up..."
	rm -rf $(PROJECT) $(BUILD_DIR)

.PHONY: depclean
depclean:
	@echo "Removing dependency files..."
	rm -f $(DEPS) 

.PHONY: clean-all
clean-all: clean depclean
