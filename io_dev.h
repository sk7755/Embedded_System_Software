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

//DOT DEVICE
#define FPGA_DOT_DEVICE "/dev/fpga_dot"
int dev_dot;

//SWITCH DEVICE
int dev_dip_switch;
#define FPGA_DIP_SWITCH_DEVICE "/dev/fpga_dip_switch"

//INPUT_EVENT DEVICE
#define INPUT_EVENT_BUFF_SIZE 64
#define KEY_RELEASE 0
#define KEY_PRESS 1
#define INPUT_EVENT_DEVICE "/dev/input/event0"
#define END_PROGRAM 158
#define MODE_UP 115
#define MODE_DOWN 114
int dev_input_event;

typedef enum{
	NONE, BACK, VOL_UP, VOL_DOWN, SW
}INPUT_TYPE;

int init_dev();
int output_led(int value);
int output_fnd(int value);
int output_text_lcd(const char str1[],const char str2[]);
int output_dot(char character);
void user_signal1(int sig);
int input_process();
int output_process();
int close_dev();

#endif
