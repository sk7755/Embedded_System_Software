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
#define IN_CSPRO 0
#define PRINT_DEBUG 1
typedef struct MsgType{
	long mtype;
	int mvalue;
}MsgType;

typedef enum {
	MSG_NONE, MSG_INPUT_EVENT, MSG_DIP_SWITCH, MSG_PUSH_SWITCH,		//INPUT
	MSG_LED, MSG_FND, MSG_TEXT_LCD,MSG_TEXT_LCD_MDF, MSG_DOT	//OUTPUT
}MSG_TYPE;

#define MODE_NUM 4
typedef enum{
	CLOCK, COUNTER, TEXT_EDITOR, DRAW_BOARD, EXIT
}MODE_TYPE;

extern int mode_init;

void init_msg_queue();
void close_msg_queue();
int msg_rcv_update();
#endif
