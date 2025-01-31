# Kernel Module
KDIR := /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

# Module name
MODULE := vicharak

# User-space programs
USER_PROGRAMS := configurator filler reader

# Compiler
CC := gcc-12

# Default target - Compile everything
all: $(MODULE).ko $(USER_PROGRAMS)

# Compile Kernel Module
$(MODULE).ko: $(MODULE).c
	$(MAKE) -C $(KDIR) M=$(PWD) modules

# Compile User-Space Programs
configurator: configurator.c
	$(CC) configurator.c -o configurator

filler: filler.c
	$(CC) filler.c -o filler

reader: reader.c
	$(CC) reader.c -o reader

# Load Kernel Module
load: $(MODULE).ko
	sudo insmod $(MODULE).ko
	dmesg | tail -n 10
	sudo mknod /dev/$(MODULE) c $$(awk '$$2=="vicharak" {print $$1}' /proc/devices) 0
	sudo chmod 666 /dev/$(MODULE)

# Unload Kernel Module
unload:
	  sudo rmmod $(MODULE)
	  dmesg | tail -n 10
	  sudo rm -f /dev/$(MODULE)

# Clean all compiled files
clean:
	  $(MAKE) -C $(KDIR) M=$(PWD) clean
	  rm -f $(MODULE).ko $(MODULE).o $(MODULE).mod.o $(MODULE).mod.c $(MODULE).symvers $(MODULE).order
	  rm -f $(USER_PROGRAMS)
