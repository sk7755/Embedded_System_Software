#include "main.h"

static int current_mode = CLOCK;
static int current_sw = 0;
static int ip_queue_id;
static int op_queue_id;
static int msg_size;
int main()
{
	//init_dev();
	int pid = fork();
	if(pid < 0){
		printf("fail to fork process1\n");
		exit(1);
	}
	else if (pid == 0){
		input_process();
		return 0;
	}
	else
		pid = fork();

	if (pid < 0)
		printf("fail to fork process2\n");
	else if( pid == 0){
		output_process();
		return 0;
	}
	
	while(TRUE){
		msg_rcv_update();
		
		switch(current_mode){
			case EXIT :
			case CLOCK :
			case COUNTER :
			case TEXT_EDITOR :
			case DRAW_BOARD :
			default :
				;
		}
	}
/*
	while(1){
		printf("I'm main process\n");
		int nbytes = msgrcv(ip_queue_id, (void*)&input_msg, msg_size, 1, 0);
		if(nbytes > 0){
			printf("Main process : recieve %s\n",input_msg.mtext);
			if(msgsnd(op_queue_id,(void *)&input_msg, msg_size, IPC_NOWAIT) < 0){
				printf("Main Process : Message Send Fail!\n");
				return 0;
			}
			else
				printf("Main Process : Message Send %s!!\n",input_msg.mtext);

		}
		usleep(1000000);
	}
	*/
	//close_dev();
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
	current_sw = NONE;

	MsgType msg;
	int nbytes = msgrcv(ip_queue_id, (void*)&msg, msg_size, 0, 0);
	if(nbytes < 0){
		printf("Main Process : Message recieve error!\n");
		return 0;
	}
	else if(nbytes > 0){
		switch(msg.mtype){
			case MSG_INPUT_EVENT:
				current_sw = msg.mvalue;
				break;
			case MSG_DIP_SWITCH:
				switch(msg.mvalue){
					case BACK:
						current_mode = EXIT;
						break;
					case VOL_UP:
						current_mode = (current_mode + 1) % MODE_NUM;
						break;
					case VOL_DOWN:
						current_mode = (current_mode - 1) % MODE_NUM;
						break;
					default:
						;
				}
				break;
			default:
				;
		}
	}
	return 1;
}


