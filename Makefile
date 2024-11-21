# Top-level Makefile for the virtualdetect project

# Default target
all: kernel_modules src venv install_deps 

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

.PHONY: all clean kernel_modules src venv install_deps run_flask
