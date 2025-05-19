## This is a custom makefile built with the help of an LLM because I don't know how makefiles work and have been using the same one for 10 years.
## Fingers crossed.

# Project Names
PROJECT = ramices
TEST_RUNNER = ramices_test

# Compiler
CC = g++

# Compiler and Linker Options
CXXFLAGS = -std=c++20 -pthread -O3 -Wall -fcolor-diagnostics
LDFLAGS = -lpthread

# Dependency Flags
DEPFLAGS = -MMD -MP

# Source and Build Directories
SRC_DIR = src
BUILD_DIR = .build
BUILD_MAIN_DIR = $(BUILD_DIR)/main
BUILD_TEST_DIR = $(BUILD_DIR)/test

# Catch2 Amalgamated Implementation File (Adjust path if yours is different)
# This file should contain the Catch2 implementation, typically catch_amalgamated.cpp
CATCH_AMALGAMATED_FILE = $(SRC_DIR)/test/catch_amalgamated.cpp

### DON'T EDIT BELOW HERE

# Find all source files
ALL_SOURCE_FILES := $(shell find $(SRC_DIR) -name "*.cpp")

# Filter source files for the main application (exclude the entire test directory)
MAIN_SOURCE_FILES := $(filter-out $(SRC_DIR)/test/%, $(ALL_SOURCE_FILES))

# Filter source files for the test runner (include all except application main)
TEST_SOURCE_FILES := $(filter-out $(SRC_DIR)/main.cpp, $(ALL_SOURCE_FILES))

# Generate object files for the main application
MAIN_OBJECTS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_MAIN_DIR)/%.o, $(MAIN_SOURCE_FILES))

# Generate object files for the test runner
# Correctly map source files from SRC_DIR to object files in BUILD_TEST_DIR
TEST_OBJECTS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_TEST_DIR)/%.o, $(TEST_SOURCE_FILES))

# Dependency Files for Main
MAIN_DEPS := $(MAIN_OBJECTS:.o=.d)

# Dependency Files for Test
TEST_DEPS := $(TEST_OBJECTS:.o=.d)


# Default Target: Build the main project (standard build)
.PHONY: all
all: $(PROJECT)

# Build Main Project
$(PROJECT): $(MAIN_OBJECTS)
	@echo "Linking $(PROJECT)..."
	$(CC) -o $@ $^ $(LDFLAGS)

# Compile Main Source Files
$(BUILD_MAIN_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "Compiling $< for $(PROJECT)..."
	$(CC) $(CXXFLAGS) $(DEPFLAGS) -c -o $@ $<

# Build Test Runner executable
.PHONY: test
test: $(TEST_OBJECTS)
	@echo "Linking $(TEST_RUNNER)..."
	$(CC) -o $(TEST_RUNNER) $^ $(LDFLAGS)

# Compile Test Source Files
# Correct rule to compile any .cpp file from SRC_DIR into an object file in BUILD_TEST_DIR
$(BUILD_TEST_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "Compiling $< for $(TEST_RUNNER)..."
	$(CC) $(CXXFLAGS) $(DEPFLAGS) -c -o $@ $<

# Run Tests
.PHONY: full
full: clean-all test
	@echo "Running tests..."
	./$(TEST_RUNNER)  && $(MAKE) testclean && $(MAKE) $(PROJECT)

# Include Dependencies
-include $(MAIN_DEPS) $(TEST_DEPS)

# Run Main Project
.PHONY: run
run: $(PROJECT)
	./$(PROJECT)

# Clean Targets
.PHONY: clean
clean:
	@echo "Cleaning up main build..."
	rm -rf $(PROJECT) $(BUILD_MAIN_DIR)

.PHONY: testclean
testclean:
	@echo "Cleaning up test build..."
	rm -rf $(TEST_RUNNER) $(BUILD_TEST_DIR)

.PHONY: depclean
depclean:
	@echo "Removing dependency files..."
	rm -f $(MAIN_DEPS) $(TEST_DEPS)

.PHONY: clean-all
clean-all: clean testclean depclean
