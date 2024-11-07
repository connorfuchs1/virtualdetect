# Makefile for VM Detection CLI Tool

# Compiler
CXX = g++

# Compiler Flags
CXXFLAGS = -Wall -Wextra -std=c++17

# Include Directories
INCLUDES = -I. $(shell pkg-config --cflags libpci)

# Libraries
LIBS = $(shell pkg-config --libs libpci)

# Source Files
SRCS = main.cpp vm_detection.cpp

# Object Files
OBJS = $(SRCS:.cpp=.o)

# Target Executable
TARGET = vm_detection

# Default Target
all: check_tools check_libpci $(TARGET)

# Build the executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

# Compile source files into object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@


# Add acpica-tools installation in the check_tools target
check_tools:
	@which acpidump >/dev/null 2>&1 || (echo "acpidump not found, installing acpica-tools..."; sudo apt-get update && sudo apt-get install -y acpica-tools)

# Modify the all target to depend on check_tools

# Clean up build files
clean:
	rm -f $(TARGET) $(OBJS)

# Phony targets
.PHONY: all clean check_libpci check_tools
