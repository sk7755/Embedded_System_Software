#include "mode_func.h"

int mode_clock(int sw)
{
	static int time_change;
	static int hour;
	static int min;
	static int sec;
	static int previous_sec;	//system
	static int time_change_led;

	time_t current_time;
	time(&current_time);
	struct tm *current_tm;
	current_tm = gmtime(&current_time);

	if(mode_init){
		time_change = 0;
		time_change_led = 0x20;
		hour = current_tm->tm_hour;
		min = current_tm->tm_min;
		sec = current_tm->tm_sec;
		output_msg_send(MSG_FND,hour*100 + min);
		output_msg_send(MSG_LED,0x80);
		output_msg_send(MSG_DOT,0);
		char init_str[] = "Hello           World           ";
		int i = 0;
		while(init_str[i] != '\0'){
			int tmp = (int)init_str[i];
			tmp = (tmp << 8) + i;
			tmp = (tmp << 8) + TEXT_LCD_EDIT;
			output_msg_send(MSG_TEXT_LCD_MDF,tmp);
			i++;
		}
		output_msg_send(MSG_TEXT_LCD,0);
		previous_sec = current_tm->tm_sec;
		mode_init = 0;
		if(PRINT_DEBUG){
			printf("--------------CLOCK MODE--------------\n");
			printf("SW1 : time change\n");
			printf("SW2 : time reset to current system time\n");
			printf("SW3 : Increase 1 hour in time change state\n");
			printf("SW4 : Increase 1 minute in time change state\n");
			printf("---------------------------------------\n");
		}
	}

	
	if(sw == 0x100){
		time_change = 1 - time_change;
		time_change_led = 0x20;
		if(!time_change)
			output_msg_send(MSG_LED,0x80);
	}
	
	if(sw == 0x080 && time_change){
		hour = current_tm->tm_hour;
		min = current_tm->tm_min;
		sec = current_tm->tm_sec;
	}

	if(sw == 0x040){
		if(time_change)
			hour++;
	}
	
	if(sw == 0x020){
		if(time_change)
			min++;
	}

	
	//difference in system
	int diff_sec = current_tm->tm_sec - previous_sec;
	int diff_min = 0;
	if(diff_sec < 0)
		diff_sec += 60;
	
	sec += diff_sec;
	diff_min = sec/60;
	min += diff_min;
	sec %= 60;
	hour += min / 60;
	min %= 60;
	hour %= 24;
	
	
	if(time_change && diff_sec > 0){
		time_change_led ^= 0x30;
		output_msg_send(MSG_LED,time_change_led);
	}

	if(diff_min>0 || sw != 0)
	{
		output_msg_send(MSG_FND,hour*100 + min);
	}
	previous_sec = current_tm-> tm_sec;
	return 1;
}

int mode_counter(int sw)
{
	static int count;
	static int radix_type;
	static int radix[RADIX_NUM] = {10,8,4,2};

	if(mode_init){
		count = 0;
		radix_type = DECIMAL;
		output_msg_send(MSG_FND,count);
		output_msg_send(MSG_LED,0x080);
		output_msg_send(MSG_DOT,0);
		output_msg_send(MSG_TEXT_LCD_MDF,TEXT_LCD_CLEAR);
		output_msg_send(MSG_TEXT_LCD,0);
		mode_init = 0;
		if(PRINT_DEBUG){
			printf("-------------COUNTER MODE--------------\n");
			printf("SW1 : radix change\n");
			printf("SW2 : +100 in current radix system\n");
			printf("SW3 : +10 in current radix system\n");
			printf("SW4 : +1 in current radix system\n");
			printf("---------------------------------------\n");
		}
	}
	int send_flag = 0;
	if(sw == 0x100){		//Switch 1 Radix change
		radix_type = (radix_type + 1) % RADIX_NUM;
		if(PRINT_DEBUG)
			printf("Current radix : %d\n",radix[radix_type]);
		send_flag = 1;
	}
	
	if(sw == 0x080){		//Switch 2 Increase 100
		count += radix[radix_type] * radix[radix_type];
		send_flag = 1;
	}
	
	if(sw == 0x040){		//Switch 3 Increase 10
		count += radix[radix_type];
		send_flag = 1;
	}
	
	if(sw == 0x020){		//Switch 4 Increase 1
		count++;
		send_flag = 1;
	}
	
	if(send_flag){
		count %= radix[radix_type] * radix[radix_type] * radix[radix_type];
		int value = radix_convert(count, radix[radix_type]);
		output_msg_send(MSG_FND,value);
	}
	
	return 1;
}

// Convert integer to integer whose string represented target radix 
// Caution : converted value is integer type which may not real value.
//			It is just integer value that switched from string to intger.
// EXample : DECIMAL 10 -----> OCTAR 12 (In octal representation)
//			radix_convert(10, 10, 8) return 12
int radix_convert(int value,int radix)
{
	int ret = 0;
	int exp = 1;

	while(value){
		ret = (ret + (value % radix) * exp)%1000;
		value /= radix;
		exp *=10;
	}

	return ret;
}

int output_msg_send(long mtype, int mvalue)
{
	MsgType msg = {mtype, mvalue};
	msgsnd(op_queue_id, (void*)&msg, sizeof(MsgType), IPC_NOWAIT);

	return 1;
}
