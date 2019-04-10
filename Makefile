CC = gcc
CFLAGS =
TARGET = 20151607

$(TARGET) : io_dev.o main.o
	$(CC) $(CFLAGS) -o $(TARGET) io_dev.o main.o

io_dev.o : io_dev.c io_dev.h main.h
	$(CC) $(CFLAGS) -c -o io_dev.o io_dev.c

main.o : main.c main.h io_dev.c
	$(CC) $(CFLAGS) -c -o main.o main.c

clean :
	rm *.o 20151607

