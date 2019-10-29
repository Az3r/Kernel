obj-m = character_device.o

KSOURCE = /lib/modules/$(shell uname -r)/build
all:
	make -C $(KSOURCE) M=$(PWD) modules
clean:
	make -C $(KSOURCE) M=$(PWD) clean
