#ifndef __IO_DEV_H__
#define __IO_DEV_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/select.h>
#include <termios.h>
#include <dirent.h>
#include <linux/input.h>
#include "main.h"


//LED DEVICE
#define FPGA_BASE_ADDRESS 0x08000000
#define LED_ADDR 0x16
unsigned char *led_addr;

//FND DEVICE
#define FND_DEVICE "/dev/fpga_fnd"
int dev_text_lcd;
int dev_fnd;

//TEXT_LCD DEVICE
#define TEXT_LCD_LINE_BUFF 16
#define TEXT_LCD_MAX_BUFF 32
#define FPGA_TEXT_LCD_DEVICE "/dev/fpga_text_lcd"
typedef enum{
	TEXT_LCD_CLEAR, TEXT_LCD_LSHIFT, TEXT_LCD_EDIT
}TEXT_LCD_OP;
/*TEXT_LCD_MDF value format
value - 32bit
0 x 0 0 | 0 0 | 0 0 | 0 0 |
		| ch  | pos | op  |
*/
//DOT DEVICE
#define FPGA_DOT_DEVICE "/dev/fpga_dot"
#define DOT_WIDTH 7
#define DOT_HEIGHT 10
#define DOT_MAX_BUFF 10
typedef enum{
	DOT_CLEAR,DOT_FILL, DOT_REVERSE, DOT_1, DOT_A, DOT_PRINT, DOT_BLINK
}DOT_OP;
int dev_dot;

//DIP SWITCH DEVICE
int dev_dip_switch;
#define FPGA_DIP_SWITCH_DEVICE "/dev/fpga_dip_switch"

//PUSH SWITCH DEVICE
int dev_push_switch;
#define FPGA_PUSH_SWITCH_DEVICE "/dev/fpga_push_switch"
#define MAX_BUTTON 9

//INPUT_EVENT DEVICE
#define INPUT_EVENT_BUFF_SIZE 64
#define KEY_RELEASE 0
#define KEY_PRESS 1
#define INPUT_EVENT_DEVICE "/dev/input/event0"

typedef enum{
	HOME_KEY = 102 ,BACK_KEY = 158, PROG_KEY = 116,
	VOL_UP_KEY = 115, VOL_DOWN_KEY = 114
}INPUT_EVENT_KEY;


int dev_input_event;
int init_dev();
int output_led(int value);
int output_fnd(int value);
int text_lcd_buff_mdf(char character, int pos, TEXT_LCD_OP op);
int output_text_lcd();
int output_dot(int value);
void user_signal1(int sig);
int input_process();
int output_process();
int close_dev();


#endif
