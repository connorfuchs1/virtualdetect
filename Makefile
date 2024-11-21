# Top-level Makefile for the virtualdetect project

# Default target
all: kernel_modules src venv install_deps 

# Build kernel modules
kernel_modules:
	$(MAKE) -C kernel_modules

# Build the main application
src:
	$(MAKE) -C src

# Create and activate a virtual environment
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

# Run the Flask application
run: install_deps
	@venv/bin/python3 -m flask --app iso_selection.py run

# Clean up build files in subdirectories
clean:
	$(MAKE) -C src clean
	$(MAKE) -C kernel_modules clean
	@rm -rf venv
	@echo "Cleaned up Python virtual environment."

.PHONY: all clean kernel_modules src venv install_deps run
