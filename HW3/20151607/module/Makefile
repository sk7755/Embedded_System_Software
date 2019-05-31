#Makefile for a device driver

obj-m   := stopwatch.o

KDIR    :=/work/achroimx_kernel
PWD     :=$(shell pwd)

all: driver
#all: driver 

driver:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules ARCH=arm

install:
	cp -a stopwatch.ko /nfsroot
	cp -a $(APP) /nfsroot
clean:
	rm -rf *.ko
	rm -rf *.mod.*
	rm -rf *.o
	rm -rf Module.symvers
	rm -rf modules.order
	rm -rf .tmp*
	rm -rf .stopwatch*
