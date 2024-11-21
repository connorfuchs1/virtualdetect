# Top-level Makefile for the virtualdetect project

# Default target
all: kernel_modules src

# Build kernel modules
kernel_modules:
	$(MAKE) -C kernel_modules

# Build the main application
src:
	$(MAKE) -C src

# Clean up build files in subdirectories
clean:
	$(MAKE) -C src clean
	$(MAKE) -C kernel_modules clean

.PHONY: all clean kernel_modules src
