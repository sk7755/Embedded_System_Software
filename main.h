#ifndef __MAIN_H__
#define __MAIN_H__

#include "io_dev.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>

#define INPUT_KEY 0x1234
#define OUTPUT_KEY 0x5678
#define TRUE 1
#define FALSE 0
typedef struct{
	long mtype;
	int mvalue;
}MsgType;

typedef enum {
	MSG_INPUT_EVENT, MSG_DIP_SWITCH,		//INPUT
	MSG_LED, MSG_FND, MSG_TEXT_LCD, MSG_DOT	//OUTPUT
}MSG_TYPE;

typedef enum{
	NONE, BACK, VOL_UP, VOL_DOWN
}INPUT_TYPE;

#define MODE_NUM 4
typedef enum{
	CLOCK, COUNTER, TEXT_EDITOR, DRAW_BOARD, EXIT
}MODE_TYPE;

void init_msg_queue();
void close_msg_queue();
int msg_rcv_update();
#endif
