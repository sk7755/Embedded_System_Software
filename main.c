#include "main.h"

static int current_mode = CLOCK;
static int current_sw = 0;
static int ip_queue_id;
int op_queue_id;
static int msg_size;
int main()
{
	init_dev();
	int pid = fork();
	if(pid < 0){
		printf("fail to fork process1\n");
		exit(1);
	}
	else if (pid == 0){
		input_process();
		close_dev();
		return 0;
	}
	else
		pid = fork();

	if (pid < 0)
		printf("fail to fork process2\n");
	else if( pid == 0){
		output_process();
		close_dev();
		return 0;
	}

	mode_init = 1;
	init_msg_queue();
	while(TRUE){
		msg_rcv_update();
		
		switch(current_mode){
			case EXIT :
				close_dev();
				//Input process Output process Main process all exit!
				break;
			case CLOCK :
				mode_clock(current_sw);
				break;
			case COUNTER :
			case TEXT_EDITOR :
			case DRAW_BOARD :
			default :
				;
		}
	}

	close_dev();
	return 0;
}

void init_msg_queue()
{
	key_t input_key = INPUT_KEY;
	key_t output_key = OUTPUT_KEY;
	
	ip_queue_id = msgget(input_key,IPC_CREAT | 0666);
	op_queue_id = msgget(output_key, IPC_CREAT | 0666);
	msg_size =sizeof(MsgType);
	return;
}

void close_msg_queue()
{
	msgctl(ip_queue_id, IPC_RMID, NULL);
	msgctl(op_queue_id, IPC_RMID, NULL);
	return;
}

int msg_rcv_update()
{
	current_sw = 0;
	MsgType msg;
	int nbytes = msgrcv(ip_queue_id, (void*)&msg, msg_size, 0, IPC_NOWAIT);
	if(nbytes > 0){
		switch(msg.mtype){
			case MSG_PUSH_SWITCH:
				current_sw = msg.mvalue;
				break;
			case MSG_INPUT_EVENT:
				switch(msg.mvalue){
					case BACK_KEY:
						current_mode = EXIT;
						break;
					case VOL_UP_KEY:
						mode_init = 1;
						current_mode = (current_mode + 1) % MODE_NUM;
						break;
					case VOL_DOWN_KEY:
						mode_init = 1;
						current_mode = (current_mode + MODE_NUM - 1) % MODE_NUM;
						break;
					default:
						;
				}
				if(PRINT_DEBUG)
					printf("CURRENT MODE : %d\n",current_mode);
				break;
			default:
				;
		}
	}
	return 1;
}


