obj-m += usbmouse.o

KBUILD_EXTRA_SYMBOLS := /home/dayan/driver/mouse_gestures_driver/kmodule/Module.symvers

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
