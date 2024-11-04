# Makefile for VM Detection Program

# Compiler
CXX = g++

# Compiler Flags
CXXFLAGS = -Wall -Wextra -std=c++11

# Target Executable
TARGET = vm_detection

# Source Files
SRC = cli.cpp

# Default Target
all: $(TARGET)

# Compile Target
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

# Clean Target - Removes Executable
clean:
	rm -f $(TARGET)

# Run Target - Compiles and Runs the Program
run: all
	./$(TARGET)
