#include "mode_func.h"

int mode_clock(int sw)
{
	static int time_change;
	static int hour;
	static int min;
	static int sec;
	static int previous_sec;	//system
	static int time_change_led;

	time_t current_time;
	time(&current_time);
	struct tm *current_tm;
	current_tm = gmtime(&current_time);

	if(mode_init){
		time_change = 0;
		time_change_led = 0x20;
		hour = current_tm->tm_hour;
		min = current_tm->tm_min;
		sec = current_tm->tm_sec;
		output_msg_send(MSG_FND,hour*100 + min);
		output_msg_send(MSG_LED,0x80);
		output_msg_send(MSG_DOT,DOT_CLEAR);
		char init_str[] = "Hello           World           ";
		int i = 0;
		while(init_str[i] != '\0'){
			int tmp = (int)init_str[i];
			tmp = (tmp << 8) + i;
			tmp = (tmp << 8) + TEXT_LCD_EDIT;
			output_msg_send(MSG_TEXT_LCD_MDF,tmp);
			i++;
		}
		output_msg_send(MSG_TEXT_LCD,0);
		previous_sec = current_tm->tm_sec;
		mode_init = 0;
		if(PRINT_DEBUG){
			printf("--------------CLOCK MODE--------------\n");
			printf("SW1 : time change\n");
			printf("SW2 : time reset to current system time\n");
			printf("SW3 : Increase 1 hour in time change state\n");
			printf("SW4 : Increase 1 minute in time change state\n");
			printf("---------------------------------------\n");
		}
	}

	
	if(sw == 0x100){
		time_change = 1 - time_change;
		time_change_led = 0x20;
		if(!time_change)
			output_msg_send(MSG_LED,0x80);
	}
	
	if(sw == 0x080 && time_change){
		hour = current_tm->tm_hour;
		min = current_tm->tm_min;
		sec = current_tm->tm_sec;
	}

	if(sw == 0x040){
		if(time_change)
			hour++;
	}
	
	if(sw == 0x020){
		if(time_change)
			min++;
	}

	
	//difference in system
	int diff_sec = current_tm->tm_sec - previous_sec;
	int diff_min = 0;
	if(diff_sec < 0)
		diff_sec += 60;

	if(!time_change){
		sec += diff_sec;
		diff_min = sec/60;
		min += diff_min;
		sec %= 60;
		hour += min / 60;
		min %= 60;
		hour %= 24;
	}
	
	
	if(time_change && diff_sec > 0){
		time_change_led ^= 0x30;
		output_msg_send(MSG_LED,time_change_led);
	}

	if(diff_min>0 || sw != 0)
	{
		output_msg_send(MSG_FND,hour*100 + min);
	}
	previous_sec = current_tm-> tm_sec;
	return 1;
}

int mode_counter(int sw)
{
	static int count;
	static int radix_type;
	static int radix[RADIX_NUM] = {2,10,8,4};

	if(mode_init){
		count = 0;
		radix_type = DECIMAL;
		output_msg_send(MSG_FND,count);
		output_msg_send(MSG_LED,0x040);
		output_msg_send(MSG_DOT,DOT_CLEAR);
		output_msg_send(MSG_TEXT_LCD_MDF,TEXT_LCD_CLEAR);
		output_msg_send(MSG_TEXT_LCD,0);
		mode_init = 0;
		if(PRINT_DEBUG){
			printf("-------------COUNTER MODE--------------\n");
			printf("SW1 : radix change\n");
			printf("SW2 : +100 in current radix system\n");
			printf("SW3 : +10 in current radix system\n");
			printf("SW4 : +1 in current radix system\n");
			printf("---------------------------------------\n");
		}
	}
	int send_flag = 0;
	if(sw == 0x100){		//Switch 1 Radix change
		radix_type = (radix_type + 1) % RADIX_NUM;
		if(PRINT_DEBUG)
			printf("Current radix : %d\n",radix[radix_type]);
		send_flag = 1;
	}
	
	if(sw == 0x080){		//Switch 2 Increase 100
		count += radix[radix_type] * radix[radix_type];
		send_flag = 1;
	}
	
	if(sw == 0x040){		//Switch 3 Increase 10
		count += radix[radix_type];
		send_flag = 1;
	}
	
	if(sw == 0x020){		//Switch 4 Increase 1
		count++;
		send_flag = 1;
	}
	
	if(send_flag){
		count %= radix[radix_type] * radix[radix_type] * radix[radix_type];
		int value = radix_convert(count, radix[radix_type]);
		output_msg_send(MSG_FND,value);
		output_msg_send(MSG_LED,0x80 >> radix_type);
	}
	
	return 1;
}

// Convert integer to integer whose string represented target radix 
// Caution : converted value is integer type which may not real value.
//			It is just integer value that switched from string to intger.
// EXample : DECIMAL 10 -----> OCTAR 12 (In octal representation)
//			radix_convert(10, 10, 8) return 12
int radix_convert(int value,int radix)
{
	int ret = 0;
	int exp = 1;

	while(value){
		ret = (ret + (value % radix) * exp) % 1000;
		value /= radix;
		exp *=10;
	}

	return ret;
}

static char text_editor_pad[10][3] ={{'0','0','0'},
	{'.','Q','Z'},{'A','B','C'},{'D','E','F'},
	{'G','H','I'},{'J','K','L'},{'M','N','O'},
	{'P','R','S'},{'T','U','V'},{'W','X','Y'}	};

int mode_text_editor(int sw)
{
	static int prev_sw;
	static int pad_cur;
	static int count;
	static int text_len;
	static int input_mode;
	if(mode_init){
		prev_sw = 0;
		pad_cur = 0;
		count = 0;
		text_len = 0;
		input_mode = CHAR_MODE;
		output_msg_send(MSG_FND, 0);
		output_msg_send(MSG_DOT,DOT_A);
		output_msg_send(MSG_LED,0);
		output_msg_send(MSG_TEXT_LCD_MDF,TEXT_LCD_CLEAR);
		output_msg_send(MSG_TEXT_LCD,0);
		mode_init = 0;

		if(PRINT_DEBUG){
			printf("-----------TEXT EDITOR MODE------------\n");
			printf("SW1~9 : Input text value\n");
			printf("SW2+3 : Text LCD clear\n");
			printf("SW5+6 : Input type change\n");
			printf("SW8+9 : Input space bar\n");
			printf("---------------------------------------\n");

		}
	}
	int press_sw = 0;
	int i, j = 1;
	for(i = 0x100; i>0;i >>= 1,j++){
		if(sw & i){
			press_sw *= 10;
			press_sw += j;
		}
	}

	int value = 0;
	if(1 <= press_sw && press_sw < 10){		//1 pressed button
		
		if(input_mode == CHAR_MODE){	//character mode
			if(prev_sw == sw){	//character change
				value = text_editor_pad[press_sw][pad_cur] << 16;
				value += (text_len - 1) << 8;
				value += TEXT_LCD_EDIT;
				output_msg_send(MSG_TEXT_LCD_MDF, value);
				pad_cur = (pad_cur + 1) % CHAR_MODE_NUM;
			}
			else{
				if(text_len == TEXT_LCD_MAX_BUFF){	//text lcd is full
					output_msg_send(MSG_TEXT_LCD_MDF,TEXT_LCD_LSHIFT);
					text_len--;
				}
				pad_cur = 0;
				value = text_editor_pad[press_sw][pad_cur] << 16;
				value += text_len << 8;
				value += TEXT_LCD_EDIT;
				output_msg_send(MSG_TEXT_LCD_MDF,value);
				pad_cur = 1;
				text_len++;
			}
		}
		if(input_mode == INT_MODE){
			if(text_len == TEXT_LCD_MAX_BUFF){	//text lcd is full
				output_msg_send(MSG_TEXT_LCD_MDF,TEXT_LCD_LSHIFT);
				text_len--;
			}
			value = (press_sw + '0') << 16;
			value += text_len << 8;
			value += TEXT_LCD_EDIT;
			output_msg_send(MSG_TEXT_LCD_MDF,value);
			text_len++;
		}
		count++;
	}
	if(press_sw == 23){
		output_msg_send(MSG_TEXT_LCD_MDF,TEXT_LCD_CLEAR);
		text_len = 0;
		pad_cur = 0;
		count++;
	}
	if(press_sw == 56){
		input_mode = 1 - input_mode;
		if(input_mode == CHAR_MODE) output_msg_send(MSG_DOT,DOT_A);
		if(input_mode == INT_MODE) output_msg_send(MSG_DOT,DOT_1);
		pad_cur = 0;
		count++;
	}
	if(press_sw == 89){
		if(text_len == TEXT_LCD_MAX_BUFF){	//text lcd is full
			output_msg_send(MSG_TEXT_LCD_MDF,TEXT_LCD_LSHIFT);
			text_len--;
		}
		text_len++;
		count++;
	}

	if(press_sw >0){
		count %= 10000;
		output_msg_send(MSG_TEXT_LCD,0);
		output_msg_send(MSG_FND,count);
		prev_sw =sw;
	}
	return 1;
	
}

int mode_draw_board(int sw)
{
	static int x,y;
	static int count;
	static int cursor_blink;
	static int previous_sec;
	time_t current_time;
	time(&current_time);
	struct tm *current_tm;
	current_tm = gmtime(&current_time);
	
	if(mode_init){
		x = 0 ,y = 0;
		count = 0;
		cursor_blink = 1;
		output_msg_send(MSG_FND, 0);
		output_msg_send(MSG_DOT,DOT_CLEAR);
		output_msg_send(MSG_LED,0);
		output_msg_send(MSG_TEXT_LCD_MDF,TEXT_LCD_CLEAR);
		output_msg_send(MSG_TEXT_LCD,0);
		previous_sec = current_tm->tm_sec;

		if(PRINT_DEBUG){
			printf("-----------DRAW BOARD MODE-------------\n");
			printf("SW2 4 6 8 : direction key\n");
			printf("SW1 : Init mode\n");
			printf("SW3 : Cursor blonk\n");
			printf("SW5 : Fill the dot at cursor\n");
			printf("SW7 : Clear the board\n");
			printf("SW9 : Reverse the board\n");
			printf("---------------------------------------\n");
		}
		mode_init = 0;
	}
	
	int diff_sec = current_tm->tm_sec - previous_sec;
	if(cursor_blink && diff_sec != 0)
		output_msg_send(MSG_DOT,(x<<16) + (y<<8) + DOT_BLINK);

	if(sw == 0x100){	//sw 1
		mode_init = 1;
		count++;
	}
	if(sw == 0x080){	//sw 2
		if(x > 0)
			x--;
		count++;
	}
	if(sw == 0x040){	//sw 3
		cursor_blink = 1 - cursor_blink;
		output_msg_send(MSG_DOT,DOT_PRINT);
		count++;
	}
	if(sw == 0x020){	//sw 4
		if(y > 0)
			y--;
		count++;
	}
	if(sw == 0x010){	//sw 5
		output_msg_send(MSG_DOT,(x<<16) + (y<<8) +DOT_FILL);
		output_msg_send(MSG_DOT,DOT_PRINT);
		count++;
	}
	if(sw == 0x008){	//sw 6
		if(y < DOT_WIDTH - 1)
			y++;
		count++;
	}
	if(sw == 0x004){	//sw 7
		output_msg_send(MSG_DOT,DOT_CLEAR);
		count++;
	}
	if(sw == 0x002){	//sw 8
		if(x < DOT_HEIGHT - 1)
			x++;
		count++;
	}
	if(sw == 0x001){
		output_msg_send(MSG_DOT,DOT_REVERSE);
		count++;
	}

	if(sw){
		count %= 10000;
		output_msg_send(MSG_FND,count);
	}
	previous_sec = current_tm-> tm_sec;

	return 1;
}

int mode_snake_game(int sw)
{
	static MAP_NODE map[DOT_HEIGHT][DOT_WIDTH];
	static POINT head, tail;
	static POINT dir;
	static int start;
	int i,j;
	if(mode_init){
		for(i = 0; i<10;i++){
			for(j =0;j<7;j++){
				map[i][j] = (MAP_NODE){EMPTY,(POINT){0,0}};
			}
		}
		map[0][0] = (MAP_NODE){SNAKE, (POINT){1, 0}};
		map[1][0] = (MAP_NODE){SNAKE, (POINT){0, 0}};
		head = (POINT){1,0};
		tail = (POINT){0,0};
		dir = (POINT){1,0};
		start = 0;
		mode_init = 0;
		srand(time(NULL));
		if(PRINT_DEBUG){
			printf("-----------SNAKE_GAME MODE-------------\n");
			printf("SW2 4 6 8 : Direction switch\n");
			printf("---------------------------------------\n");

		}
		feed_generate(map);
	}

	if(sw == 0x080)	//sw 2
		dir = (POINT){-1,0};
	if(sw == 0x020) //sw 4
		dir = (POINT){0,-1};
	if(sw == 0x008)	//sw 6
		dir = (POINT){1,0};
	if(sw == 0x002) //sw 8
		dir = (POINT){0,1};
	if(sw == 0x100) //sw 1
		start = 1;

	if(!start)
		return 1;

	switch(move_or_eat(map,&head,&tail,dir)){
		case MOVE:
			break;
		case DIE:
			start = 0;
			mode_init = 1;
			if(PRINT_DEBUG)
				printf("GAME OVER!!\n");
			break;
		case EAT:
			if(PRINT_DEBUG)
				printf("EAT!!\n");
			feed_generate(map);
			break;
	}
	if(sw)
		draw_map(map);
	if(PRINT_DEBUG)
		printf("HEAD POS : (%d, %d) TAIL POS : (%d, %d)\n",head.x, head.y, tail.x, tail.y);
	usleep(1000000);
	return 1;
}

//fail -1 eat 0 move 1
int move_or_eat(MAP_NODE map[DOT_HEIGHT][DOT_WIDTH], POINT *head, POINT *tail, POINT dir)
{
	POINT next_head;
	next_head.x = head->x + dir.x;
	next_head.y = head->y + dir.y;
	 
	if(next_head. x < 0 || next_head.x >= DOT_HEIGHT)
		 return DIE;
	
	if(next_head. y < 0 || next_head.y >= DOT_WIDTH)
		 return DIE;

	switch(map[next_head.x][next_head.y].type){
		case FEED :
			map[head->x][head->y].next = next_head;
			map[next_head.x][next_head.y] =(MAP_NODE){SNAKE,{0,0}};
			*head = next_head;
			return EAT;
		case SNAKE :
			return DIE;
		case EMPTY :
			map[head->x][head->y].next = next_head;
			map[next_head.x][next_head.y] = (MAP_NODE){SNAKE,{0,0}};
			*head = next_head;
			POINT tmp = map[tail->x][tail->y].next;
			map[tail->x][tail->y] = (MAP_NODE){EMPTY, {0,0}};
			*tail = tmp;
			return MOVE;
		default :
			return ERROR;
	}
}

int feed_generate(MAP_NODE map[DOT_HEIGHT][DOT_WIDTH])
{
	int i,j;
	
	do{
		i = rand() % DOT_HEIGHT;
		j = rand() % DOT_WIDTH;
	}while(map[i][j].type != EMPTY);

	map[i][j] = (MAP_NODE){FEED, {0,0}};

	return 1;
}

int draw_map(MAP_NODE map[DOT_HEIGHT][DOT_WIDTH])
{
	int i,j;

	output_msg_send(MSG_DOT,DOT_CLEAR);
	for(i =0;i<DOT_HEIGHT;i++){
		for(j=0;j<DOT_WIDTH;j++){
			if(map[i][j].type != EMPTY)
				output_msg_send(MSG_DOT,(i<<16) + (j<<8) +DOT_FILL);
		}
	}
	output_msg_send(MSG_DOT,DOT_PRINT);

	return 1;
}

int output_msg_send(long mtype, int mvalue)
{
	MsgType msg = {mtype, mvalue};
	msgsnd(op_queue_id, (void*)&msg, sizeof(MsgType), IPC_NOWAIT);

	return 1;
}
