#include "io_dev.h"

int main()
{
	init_dev();
	
	printf("1. led, 2. fnd : ");
	int type;
	scanf("%d",&type);

	int value;
	if(type == 1)
	{
		printf("input value : ");
		scanf("%d",&value);
		output_led(value);
	}
	if(type == 2)
	{
		printf("input value : ");
		scanf("%d",&value);
		output_fnd(value);
	}

	close_dev();
	return 0;
}

int init_dev()
{
	//LED DEVICE OPEN BY MMAP
	unsigned long *fpga_addr = 0;
	int fd;
	fd = open("/dev/mem", O_RDWR | O_SYNC);
	if(fd < 0){
		perror("/dev/mem open error");
		exit(1);
	}

	fpga_addr = (unsigned long *)mmap(NULL,4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, FPGA_BASE_ADDRESS);
	if( fpga_addr == MAP_FAILED)
	{
		printf("LED : mmap error!\n");
		close(fd);
		exit(1);
	}
	led_addr = (unsigned char*)((void*)fpga_addr + LED_ADDR);

	//FND DEVICE OPEN BY DRIVER
	dev_fnd = open(FND_DEVICE, O_RDWR);
	if(dev_fnd < 0){
		printf("FND : Device open error : %s\n",FND_DEVICE);
		exit(1);
	}

	//TEXT_LCD DEVICE OPEN BY DRIVER
	dev_text_lcd = open(FPGA_TEXT_LCD_DEVICE, O_WRONLY);
	if(dev_text_lcd < 0){
		printf("TEXT_LCD : Device open error : %s\n",FPGA_TEXT_LCD_DEVICE);
		exit(1);
	}

	//DOT DEVICE OPEN BY DRIVER
	dev_dot = open(FPGA_DOT_DEVICE, O_WRONLY);
	if(dev_dot < 0){
		printf("DOT : Device open error : %s\n",FPGA_DOT_DEVICE);
		exit(1);
	}

	//SWITCH DEVICE OPEN BY DRIVER
	dev_dip_switch = open(FPGA_DIP_SWITCH_DEVICE, O_RDWR);
	
	if (dev_dip_switch < 0){
		printf("DIP_SWITCH : Device open error : %s\n",FPGA_DIP_SWITCH_DEVICE);
		exit(1);
	}	

	//INPUT_EVENT DEVICE OPEN BY DRIVER
	dev_input_event = open(INPUT_EVENT_DEVICE, O_RDONLY);
	
	if(dev_input_event < 0){
		printf("INPUT_EVENT : Device open error : %s\n",INPUT_EVENT_DEVICE);
		exit(1);
	}

	return 1;
}

int close_dev()
{
	munmap(led_addr, 4096);
	close(dev_fnd);
	close(dev_text_lcd);
	close(dev_dot);
	close(dev_dip_switch);
	close(dev_input_event);

	return 1;
}

int output_led(int value)
{
	if(value < 0 || value > 255)
	{
		printf("LED : Invalid range! -%d\n",value);
		return 0;
	}

	*led_addr = value;
	return 1;
}

int output_fnd(int value)
{
	if(value <0 || value >9999)
	{
		printf("FND : Invalid range! -%d\n",value);
		return 0;
	}

	unsigned char data[4] = {0,};
	int d = 1000;
	int i;
	for(i = 0;i<4;i++)
	{
		data[i] = value / d;
		value %= d;
		d /= 10;
	}
	unsigned char retval = write(dev_fnd, &data,4);

	if(retval < 0){
		printf("FND : Write Error! -%d\n",value);
		return 0;
	}
	memset(data,0,sizeof(data));

	return 1;
}

int output_text_lcd(const char str1[], const char str2[])
{
	int str1_size = strlen(str1);
	int str2_size = strlen(str2);
	
	if(str1_size > TEXT_LCD_LINE_BUFF || str2_size > TEXT_LCD_LINE_BUFF)
	{
		printf("TEXT_LCD : 16 alphanumeric characters on a line! - strlen1 %d strlen2 %d\n",str1_size, str2_size);
		return 0;
	}
	unsigned char str[TEXT_LCD_MAX_BUFF];
	
	strncat(str, str1,str1_size);
	memset(str+str1_size, ' ',TEXT_LCD_LINE_BUFF - str1_size);
	strncat(str, str2,str2_size);
	memset(str+str2_size, ' ',TEXT_LCD_LINE_BUFF - str2_size);
	write(dev_text_lcd, str, TEXT_LCD_MAX_BUFF);

	return 1;
}

int output_dot(char character)
{
	int str_size;
	if(character >= '0' && character <= '9'){
		str_size = sizeof(fpga_number[character -'0']);
		write(dev_dot,fpga_number[character-'0'],str_size);
	}
	else if(character == 'A'){
		str_size = sizeof(fpga_number[character -'A']);
		write(dev_dot,fpga_number[character-'A'],str_size);
	}
	else{
		printf("DOT : Invalid Character (0~9 | A) ! -%c\n",character);
		return 0;
	}

	return 1;
}

void user_signal1(int sig)
{
	quit = 1;
}

int input_process()
{
	unsigned char dip_sw_buff = 0;
	struct input_event ev[INPUT_EVENT_BUFF_SIZE];
	int size = sizeof(struct input_event);
	int rd;

	(void)signal(SIGINT, user_signal1);

	while(!quit){
		usleep(400000);
		if((rd = read(dev_input_event,ev,size * INPUT_EVENT_BUFF_SIZE)) < size){
			printf("INPUT PROCESS : read()\n");
			return 0;
		}
		/*추가 하도록 */
		read(dev_dip_switch, &dip_sw_buff, 1);

	}
	return 1;
}
