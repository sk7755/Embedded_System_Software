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
	DECIMAL, OCATAL, QUATER, BINARY
}RADIX_TYPE;

int output_msg_send(long mtype, int mvalue);
int radix_convert(int value, int radix);
int mode_clock(int sw);
int mode_counter(int sw);
int mode_text_editor();
int mode_draw_board();
#endif
