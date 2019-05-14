/* dev driver test application
File : app.c
Auth : sk7755@naver.com*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <syscall.h>

#include "../module/dev_driver.h"
#define MY_DEVICE "/dev/dev_driver"

int main(int argc, char **argv)
{
	int dev;
	int interval, count, start_number;
	int i;
	if(argc!=4) {
		printf("please input the parameter! \n");
		printf("ex)./app [1-100] [1-100] [0001-8000]\n");
		return -1;
	}

	interval = atoi(argv[1]);
	count = atoi(argv[2]);

	if(interval < 1 || interval > 100){
		printf("Invalid Interval Range! [1-100]\n");
		exit(1);
	}
	if(count < 1 || count > 100){
		printf("Invalid Count Range! [1-100]\n");
		exit(1);
	}
	if(strlen(argv[3]) != 4){
		printf("Invalid start number length! [4]\n");
		exit(1);
	}

	int tmp = 0;
	for(i = 0 ; i < 4; i++){
		if(argv[3][i] < '0' || argv[3][i] > '8'){
			printf("Invalid start digit number! [0-8]\n");
			exit(1);
		}
		if(argv[3][i] != '0')
			tmp++;
	}

	if(tmp != 1){
		printf("Invalid start number! [only one nonzero number]\n");
		exit(1);
	}
	start_number = atoi(argv[3]);

	dev = open(MY_DEVICE, O_RDWR);
	if(dev < 0){
		printf("Device open error : %s\n",MY_DEVICE); 
		exit(1);
	}

	unsigned int data = syscall(376,interval, count,start_number);
	int ret_val = ioctl(dev, IOCTL_SET_TIMER, data);

	if(ret_val < 0){
		printf("ioctl_set_timer failed:%d\n",ret_val);
		exit(-1);
	}
	close(dev);

	return 0;
}
