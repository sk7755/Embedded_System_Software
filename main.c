#include "main.h"

static int current_mode = CLOCK;
static int current_sw = 0;
static int ip_queue_id;
int op_queue_id;
static int msg_size;
int main()
{
	init_dev();
	int input_pid = fork();
	int output_pid;
	if(input_pid < 0){
		printf("fail to fork process1\n");
		exit(1);
	}
	else if (input_pid == 0){
		input_process();
		close_dev();
		if(PRINT_DEBUG)
			printf("Input process Exit!!\n");
		return 0;
	}
	else
		output_pid = fork();

	if (output_pid < 0)
		printf("fail to fork process2\n");
	else if( output_pid == 0){
		output_process();
		close_dev();
		if(PRINT_DEBUG)
			printf("Output process Exit!!\n");
		return 0;
	}

	mode_init = 1;
	int status;
	init_msg_queue();
	while(TRUE){
		msg_rcv_update();
		
		switch(current_mode){
			case EXIT :
				close_dev();
				kill(input_pid, SIGINT);
				kill(output_pid,SIGINT);
				waitpid(input_pid, &status,0);
				waitpid(output_pid, &status,0);
				close_msg_queue();
				if(PRINT_DEBUG)
					printf("Main process Exit!!\n");
				return 0;
				//Input process Output process Main process all exit!
				break;
			case CLOCK :
				mode_clock(current_sw);
				break;
			case COUNTER :
				mode_counter(current_sw);
				break;
			case TEXT_EDITOR :
				mode_text_editor(current_sw);
				break;
			case DRAW_BOARD :
				mode_draw_board(current_sw);
				break;
			case SNAKE_GAME :
				mode_snake_game(current_sw);
				break;
			default :
				;
		}
		usleep(40000);
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
		if(PRINT_DEBUG)
			printf("Main process : message recieved ! - %ld %d\n",msg.mtype, msg.mvalue);
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
					printf("Main process :current mode - %d\n",current_mode);

				break;
			default:
				;
		}
	}
	return 1;
}


