# Makefile for kernel module

obj-m += acpi_mask_module.o dmi_module.o 

# Default target that checks for kernel headers
all: check_headers
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# Clean up build files
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

# Check for kernel headers and install if missing
check_headers:
	@if [ ! -d "/lib/modules/$(shell uname -r)/build" ]; then \
		echo "Kernel headers not found for $(shell uname -r). Installing..."; \
		sudo apt-get update && sudo apt-get install -y linux-headers-$(shell uname -r); \
	else \
		echo "Kernel headers found."; \
	fi
