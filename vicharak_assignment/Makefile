
KDIR := /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)


MODULE := vicharak

USER_PROGRAMS := configurator filler reader

CC := gcc-12

all: $(MODULE).ko $(USER_PROGRAMS)

$(MODULE).ko: $(MODULE).c
	$(MAKE) -C $(KDIR) M=$(PWD) modules

configurator: configurator.c
	$(CC) configurator.c -o configurator

filler: filler.c
	$(CC) filler.c -o filler

reader: reader.c
	$(CC) reader.c -o reader

load: $(MODULE).ko
	sudo insmod $(MODULE).ko
	dmesg | tail -n 10
	sudo mknod /dev/$(MODULE) c $$(awk '$$2=="vicharak" {print $$1}' /proc/devices) 0
	sudo chmod 666 /dev/$(MODULE)

load:
	  sudo rmmod $(MODULE)
	  dmesg | tail -n 10
	  sudo rm -f /dev/$(MODULE)

clean:
	  $(MAKE) -C $(KDIR) M=$(PWD) clean
	  rm -f $(MODULE).ko $(MODULE).o $(MODULE).mod.o $(MODULE).mod.c $(MODULE).symvers $(MODULE).order
	  rm -f $(USER_PROGRAMS)
