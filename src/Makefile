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
all: $(TARGET)

# Build the executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

# Compile source files into object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean up build files
clean:
	rm -f $(TARGET) $(OBJS)

# Phony targets
.PHONY: all clean
