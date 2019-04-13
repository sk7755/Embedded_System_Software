#ifndef __MODE_FUNC_H__
#define __MODE_FUNC_H__

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include "main.h"
#include "io_dev.h"

extern int op_queue_id;
int mode_init;

#define RADIX_NUM 4
typedef enum{
	BINARY,DECIMAL,OCATAL, QUATER
}RADIX_TYPE;

typedef enum{
	CHAR_MODE,INT_MODE 
}EDITOR_INPUT_MODE;
#define CHAR_MODE_NUM 3	//한 칸에 모여 있는 char 개수

int output_msg_send(long mtype, int mvalue);
int radix_convert(int value, int radix);
int mode_clock(int sw);
int mode_counter(int sw);
int mode_text_editor(int sw);
int mode_draw_board(int sw);
#endif
