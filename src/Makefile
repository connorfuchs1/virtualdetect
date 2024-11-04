# Makefile for VM Detection CLI Tool

# Compiler
CXX = g++

# Compiler Flags
CXXFLAGS = -Wall -Wextra -std=c++11

# Include Directories
INCLUDES = -I.

# Source Files
SRCS = main.cpp vm_detection.cpp

# Object Files
OBJS = $(SRCS:.cpp=.o)

# Target Executable
TARGET = vm_detection

# Default Target
all: $(TARGET)

# Build the executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Compile source files into object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean up build files
clean:
	rm -f $(TARGET) $(OBJS)

# Phony targets
.PHONY: all clean
