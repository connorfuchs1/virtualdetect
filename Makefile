# Top-level Makefile for the virtualdetect project

# Default target
all: install_sys_deps kernel_modules src venv install_deps

# Install system dependencies
install_sys_deps:
	@echo "Checking for required system packages..."
	@if ! dpkg -s rsync >/dev/null 2>&1; then \
		echo "rsync not found. Installing rsync..."; \
		sudo apt-get update && sudo apt-get install -y rsync; \
	else \
		echo "rsync is already installed."; \
	fi
	@if ! dpkg -s squashfs-tools >/dev/null 2>&1; then \
		echo "squashfs-tools not found. Installing squashfs-tools..."; \
		sudo apt-get install -y squashfs-tools; \
	else \
		echo "squashfs-tools is already installed."; \
	fi
	@if ! dpkg -s genisoimage >/dev/null 2>&1; then \
		echo "genisoimage not found. Installing genisoimage..."; \
		sudo apt-get install -y genisoimage; \
	else \
		echo "genisoimage is already installed."; \
	fi
	@if ! dpkg -s make >/dev/null 2>&1; then \
		echo "make not found. Installing make..."; \
		sudo apt-get install -y make; \
	else \
		echo "make is already installed."; \
	fi
	@if ! dpkg -s g++ >/dev/null 2>&1; then \
		echo "g++ not found. Installing g++..."; \
		sudo apt-get install -y g++; \
	else \
		echo "g++ is already installed."; \
	fi
	@if ! dpkg -s python3-venv >/dev/null 2>&1; then \
		echo "python3-venv not found. Installing python3-venv..."; \
		sudo apt-get install -y python3-venv; \
	else \
		echo "python3-venv is already installed."; \
	fi
	@echo "All required system packages are installed."

# Build kernel modules
kernel_modules:
	$(MAKE) -C kernel_modules

# Build C++ sources
src:
	$(MAKE) -C src

# Set up Python virtual environment
venv:
	@if [ ! -d "venv" ]; then \
		python3 -m venv venv; \
		echo "Virtual environment created."; \
	else \
		echo "Virtual environment already exists."; \
	fi

# Install Python dependencies
install_deps: venv
	@venv/bin/pip install --upgrade pip
	@venv/bin/pip install -r requirements.txt
	@echo "Python dependencies installed."

# Run the Flask app
run_flask: install_deps
	@cd upload && ../venv/bin/python3 -m flask --app iso_selection.py run

# Clean up build files
clean:
	$(MAKE) -C src clean
	$(MAKE) -C kernel_modules clean
	@rm -rf venv
	@echo "Cleaned up Python virtual environment."

.PHONY: all clean kernel_modules src venv install_deps run_flask install_sys_deps
