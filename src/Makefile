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
SRCS = main.cpp vm_detection.cpp vm_mitigations.cpp  # Added vm_mitigations.cpp

# Object Files
OBJS = $(SRCS:.cpp=.o)

# Target Executable
TARGET = vm_detection

# Default Target
all: check_tools $(TARGET)

# Build the executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

# Compile source files into object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Check for required tools and install if missing
check_tools:
	@echo "Checking dependencies..."
	@which acpidump >/dev/null 2>&1 || (echo "acpidump not found, installing acpica-tools..."; sudo apt-get update && sudo apt-get install -y acpica-tools)
	@which lsusb >/dev/null 2>&1 || (echo "lsusb not found, installing usbutils..."; sudo apt-get install -y usbutils)
	@which lscpu >/dev/null 2>&1 || (echo "lscpu not found, installing util-linux..."; sudo apt-get install -y util-linux)
	@ldconfig -p | grep libpci.so >/dev/null 2>&1 || (echo "libpci not found, installing pciutils and libpci-dev..."; sudo apt-get install -y pciutils libpci-dev)

# Clean up build files
clean:
	rm -f $(TARGET) $(OBJS)

# Phony targets
.PHONY: all clean check_tools
