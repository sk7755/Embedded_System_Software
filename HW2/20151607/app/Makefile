#Makefile for a device driver
APP	:= app

all: app

app:
	arm-none-linux-gnueabi-gcc -static -o $(APP) $(APP).c

clean:
	rm -rf *.o
	rm -rf $(APP)

