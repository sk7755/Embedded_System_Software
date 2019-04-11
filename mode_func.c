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

int output_msg_send(long mtype, int mvalue)
{
	MsgType msg = {mtype, mvalue};
	msgsnd(op_queue_id, (void*)&msg, sizeof(MsgType), IPC_NOWAIT);

	return 1;
}
