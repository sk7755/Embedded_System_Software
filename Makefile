mode = 0

ifeq ($(mode) ,1)
	CC = arm-none-linux-gnueabi-gcc
	CFLAGS = -static
else
	CC = gcc
	CFLAGS = -Wall
	LDFLAGS = 
endif
TARGET = 20151607

$(TARGET) : io_dev.o main.o mode_func.o
	$(CC) $(CFLAGS) -o $(TARGET) io_dev.o main.o mode_func.o $(LDFLAGS)

io_dev.o : io_dev.c io_dev.h main.h
	$(CC) $(CFLAGS) -c -o io_dev.o io_dev.c

mode_func.o : mode_func.c mode_func.h main.h io_dev.h
	$(CC) $(CFLAGS) -c -o mode_func.o mode_func.c

main.o : main.c main.h io_dev.c mode_func.c
	$(CC) $(CFLAGS) -c -o main.o main.c

clean :
	rm *.o 20151607

