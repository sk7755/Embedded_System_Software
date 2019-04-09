#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

#define IOM_LED_ADDRESS 0x08000016 // pysical address

int main(int argc, char **argv)
{
	int fd,i;
	unsigned char val[] = { 0x70,0x80,0xD0,0xE0,0x00,0xF0 };

	unsigned long *ledaddr = 0;
	printf("0");

	fd = open("/dev/mem", O_RDWR | O_SYNC);
	if (fd < 0) {
		perror("/dev/mem open error");
		exit(1);
	}
	printf("1");
	ledaddr = (unsigned long *)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, IOM_LED_ADDRESS);
	printf("2");
	if (ledaddr == NULL)
	{
		printf("mmap error!\n");
		close(fd);
		exit(1);
	}
	printf("3");

	for (i = 0; i < 5; i++) {
//		outw(0x01, (unsigned int)ledaddr);
	*ledaddr = 0x01;//(val[i]);
 	sleep(1);
	}
	printf("4");

	munmap(ledaddr, 4096);
	close(fd);
	return 0;
}
