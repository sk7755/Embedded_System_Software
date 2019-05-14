#Makefile for a device driver

obj-m   := dev_driver.o

KDIR    :=/work/achroimx_kernel
PWD     :=$(shell pwd)

all: driver app
#all: driver 

driver:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules ARCH=arm

app:
	arm-none-linux-gnueabi-gcc -static -o app app.c

install:
	cp -a dev_driver.ko /nfsroot
	cp -a app /nfsroot
clean:
	rm -rf *.ko
	rm -rf *.mod.*
	rm -rf *.o
	rm -rf app
	rm -rf Module.symvers
	rm -rf modules.order
	rm -rf .tmp*
