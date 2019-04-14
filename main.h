#ifndef __MAIN_H__
#define __MAIN_H__

#include "io_dev.h"
#include "mode_func.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/wait.h>

#define INPUT_KEY 0x1234
#define OUTPUT_KEY 0x5678
#define TRUE 1
#define FALSE 0
#define IN_CSPRO 1			//For CSPRO TEST
#define PRINT_DEBUG 1
typedef struct MsgType{		//Message Structure
	long mtype;
	int mvalue;
}MsgType;

typedef enum {	//Message Type
	MSG_NONE, MSG_INPUT_EVENT, MSG_DIP_SWITCH, MSG_PUSH_SWITCH,		//INPUT
	MSG_LED, MSG_FND, MSG_TEXT_LCD,MSG_TEXT_LCD_MDF, MSG_DOT	//OUTPUT
}MSG_TYPE;

#define MODE_NUM 5	//Number of Mode
typedef enum{
	CLOCK, COUNTER, TEXT_EDITOR, DRAW_BOARD,SNAKE_GAME, EXIT
}MODE_TYPE;

extern int mode_init;

//Initialize Message Queue
void init_msg_queue();
//Close Message Queue
void close_msg_queue();
//Message Recieve and Update pushed switch and current mode
int msg_rcv_update();
#endif
