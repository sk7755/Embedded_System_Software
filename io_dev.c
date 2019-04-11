#include "io_dev.h"

unsigned char fpga_number[10][10] = {
	{0x3e,0x7f,0x63,0x73,0x73,0x6f,0x67,0x63,0x7f,0x3e}, // 0
	{0x0c,0x1c,0x1c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x1e}, // 1
	{0x7e,0x7f,0x03,0x03,0x3f,0x7e,0x60,0x60,0x7f,0x7f}, // 2
	{0xfe,0x7f,0x03,0x03,0x7f,0x7f,0x03,0x03,0x7f,0x7e}, // 3
	{0x66,0x66,0x66,0x66,0x66,0x66,0x7f,0x7f,0x06,0x06}, // 4
	{0x7f,0x7f,0x60,0x60,0x7e,0x7f,0x03,0x03,0x7f,0x7e}, // 5
	{0x60,0x60,0x60,0x60,0x7e,0x7f,0x63,0x63,0x7f,0x3e}, // 6
	{0x7f,0x7f,0x63,0x63,0x03,0x03,0x03,0x03,0x03,0x03}, // 7
	{0x3e,0x7f,0x63,0x63,0x7f,0x7f,0x63,0x63,0x7f,0x3e}, // 8
	{0x3e,0x7f,0x63,0x63,0x7f,0x3f,0x03,0x03,0x03,0x03} // 9
};

unsigned char fpga_A[26][10] = {
	{0x1c,0x36,0x63,0x63,0x63,0x7f,0x7f,0x63,0x63,0x63}, // A
};

unsigned char fpga_set_full[10] = {
	// memset(array,0x7e,sizeof(array));
	0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f
};
unsigned char fpga_set_blank[10] = {
	// memset(array,0x00,sizeof(array));
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};
unsigned char quit = 0;

unsigned char text_lcd_buff[TEXT_LCD_MAX_BUFF + 1] = {0,};
int text_lcd_i = 0;

int init_dev()
{
	//LED DEVICE OPEN BY MMAP
	unsigned long *fpga_addr = 0;
	int fd;

	fd = open("/dev/mem", O_RDWR | O_SYNC);
	if(fd < 0){
		perror("/dev/mem open error");
		return 0;
	}

	fpga_addr = (unsigned long *)mmap(NULL,4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, FPGA_BASE_ADDRESS);
	if( fpga_addr == MAP_FAILED)
	{
		printf("LED : mmap error!\n");
		close(fd);
		return 0;
	}
	led_addr = (unsigned char*)((void*)fpga_addr + LED_ADDR);

	//FND DEVICE OPEN BY DRIVER
	dev_fnd = open(FND_DEVICE, O_RDWR);
	if(dev_fnd < 0){
		printf("FND : Device open error : %s\n",FND_DEVICE);
		return 0;
	}

	//TEXT_LCD DEVICE OPEN BY DRIVER
	dev_text_lcd = open(FPGA_TEXT_LCD_DEVICE, O_WRONLY);
	if(dev_text_lcd < 0){
		printf("TEXT_LCD : Device open error : %s\n",FPGA_TEXT_LCD_DEVICE);
		return 0;
	}
	output_text_lcd(0,TEXT_LCD_CLEAR);

	//DOT DEVICE OPEN BY DRIVER
	dev_dot = open(FPGA_DOT_DEVICE, O_WRONLY);
	if(dev_dot < 0){
		printf("DOT : Device open error : %s\n",FPGA_DOT_DEVICE);
		return 0;
	}

	//DIP SWITCH DEVICE OPEN BY DRIVER
	dev_dip_switch = open(FPGA_DIP_SWITCH_DEVICE, O_RDWR);
	
	if (dev_dip_switch < 0){
		printf("DIP_SWITCH : Device open error : %s\n",FPGA_DIP_SWITCH_DEVICE);
		return 0;
	}	

	//PUSH SWITCH DEVICE OPEN BY DRIVER
	dev_push_switch = open(FPGA_PUSH_SWITCH_DEVICE, O_RDWR);
	if(dev_push_switch < 0){
		printf("PUSH_SWITCH : Device open error : %s\n",FPGA_PUSH_SWITCH_DEVICE);
		return 0;
	}
	//(void)signal(SIGINT,user_signal1);

	//INPUT_EVENT DEVICE OPEN BY DRIVER
	dev_input_event = open(INPUT_EVENT_DEVICE, O_RDWR | O_NONBLOCK);
	
	//int flags = fcntl(dev_input_event,F_GETFD);
	//fcntl(dev_input_event,F_SETFD,flags | O_NONBLOCK);
	if(dev_input_event < 0){
		printf("INPUT_EVENT : Device open error : %s\n",INPUT_EVENT_DEVICE);
		return 0;
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
	close(dev_push_switch);
	close(dev_input_event);

	return 1;
}

int output_led(int value)
{
	if(PRINT_DEBUG)
		printf("output led - %x\n",value);
	//return 1;
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
	if(PRINT_DEBUG)
		printf("output fnd - %d\n",value);
	//return 1;
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

int text_lcd_buff_mdf(char character,int pos, TEXT_LCD_OP op)
{
	if(PRINT_DEBUG)
		printf("output text lcd mdf- %c %d %d\n",character, pos, op);

	int i;
	switch(op){
		case TEXT_LCD_CLEAR :
			for(i=0;i<TEXT_LCD_MAX_BUFF;i++)
				text_lcd_buff[i] = ' ';
			break;
		case TEXT_LCD_LSHIFT :
			for(i=0;i<TEXT_LCD_MAX_BUFF - 1;i++)
				text_lcd_buff[i] = text_lcd_buff[i+1];
			text_lcd_buff[TEXT_LCD_MAX_BUFF-1] = ' ';
			break;
		case TEXT_LCD_EDIT :
			if(pos < 0 || pos >= TEXT_LCD_MAX_BUFF){
				printf("TEXT_LCD : Can't modify that position %d!\n",pos);
				return 0;
			}
			text_lcd_buff[pos] = character;
			break;
		default :
			;
	}
	return 1;
}

int output_text_lcd()
{
	if(PRINT_DEBUG)
		printf("output text lcd - %s\n",text_lcd_buff);
	//return 1;
	write(dev_text_lcd, text_lcd_buff, TEXT_LCD_MAX_BUFF);

	return 1;
}

int output_dot(char character)
{
	if(PRINT_DEBUG)
		printf("output dot -%c\n",character);
	//return 1;
	int str_size;

	if(character == 0){
		str_size = sizeof(fpga_set_blank);
		write(dev_dot,fpga_set_blank,str_size);
	}
	else if(character >= '0' && character <= '9'){
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
	key_t key = INPUT_KEY;
	int queue_id = msgget(key,IPC_CREAT | 0666);

	MsgType msg;
	MsgType msg2;
	unsigned char push_sw_buff[MAX_BUTTON];
	int msg_size = sizeof(MsgType);
	int push_buff_size = sizeof(push_sw_buff);
	int previous_sw_value = 0;
	int i;


	struct input_event ev[INPUT_EVENT_BUFF_SIZE];
	int input_event_size = sizeof(struct input_event);
	int rd;
	int input_event_value;
	
	while(!quit){	
		//PUSH_SWITCH input process
		read(dev_push_switch,&push_sw_buff, push_buff_size);
		
		int push_sw_value = 0;
		for(i =0;i<MAX_BUTTON;i++){
			push_sw_value <<= 1;
			push_sw_value += push_sw_buff[i];
		}
		if(push_sw_value - previous_sw_value > 0){
			msg.mtype = MSG_PUSH_SWITCH;
			msg.mvalue = push_sw_value;
			if(msgsnd(queue_id,(void *)&msg, msg_size, IPC_NOWAIT) < 0){
				printf("Input Process : Message Send Fail!\n");
				return 0;
			}
			else{
				if(PRINT_DEBUG)
					printf("Input Process : Message Send %d!!\n",push_sw_value);
			}

		}
		previous_sw_value = push_sw_value;

		//INPUT_EVENT input process

		rd =read(dev_input_event,ev,input_event_size * INPUT_EVENT_BUFF_SIZE);

		if(rd >= input_event_size){
			input_event_value = ev[0].value;
			if(input_event_value == KEY_PRESS){
				msg.mtype = MSG_INPUT_EVENT;
				msg.mvalue = (int)ev[0].code;
				
				if(msgsnd(queue_id,(void *)&msg, msg_size, IPC_NOWAIT) < 0){
					printf("Input Process : Message Send Fail!\n");
					return 0;
				}
				else{
					if(PRINT_DEBUG)
						printf("Input Process : Message Send %d!!\n",msg.mvalue);
				}
			}
		}


	}
	return 1;
}

int output_process()
{
	key_t key = OUTPUT_KEY;
	int queue_id = msgget(key,IPC_CREAT | 0666);

	MsgType msg;
	char character;
	int pos;
	TEXT_LCD_OP op;
	int msg_size = sizeof(MsgType);

	while(1){
		int nbytes = msgrcv(queue_id, (void*)&msg, msg_size, 0,0);
		if(nbytes < 0){
			printf("Output Process : Message recieve error!\n");
			return 0;
		}
		else if(nbytes > 0){
			if(PRINT_DEBUG)
				printf("Output process Recieved!\n");
			switch(msg.mtype){
				case MSG_LED :
					output_led(msg.mvalue);
					break;
				case MSG_FND :
					output_fnd(msg.mvalue);
					break;
				case MSG_TEXT_LCD :
					output_text_lcd();
					break;
				case MSG_TEXT_LCD_MDF :
					character = (char)((msg.mvalue >> 16) & 0xFF);
					pos = (msg.mvalue >> 8) & 0xFF;
					op = msg.mvalue & 0xFF;
					text_lcd_buff_mdf(character, pos, op);
					break;
				case MSG_DOT :
					output_dot(msg.mvalue);
					break;
				default :
					;
			}
		}
		

	}


}