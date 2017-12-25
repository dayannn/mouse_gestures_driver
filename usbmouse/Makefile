obj-m += usbmouse.o

KBUILD_EXTRA_SYMBOLS := /home/dayan/kurs/app/myModule/Module.symvers

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
