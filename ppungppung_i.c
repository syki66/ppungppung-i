#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <string.h>
#include <asm/ioctls.h>
#include <stdbool.h>
#include<string.h>
#include <time.h>
#define clcd "/dev/clcd"

#include<unistd.h>
#include<sys/ioctl.h>
#include<sys/stat.h>

#define led "/dev/led"

#include    <time.h>

#define dot "/dev/dot"




static char tactswDev[] = "/dev/tactsw";
static int  tactswFd = (-1);





int led_count = 0;

int user_input = 0;




unsigned char tactsw_get(int tmo)
{   
    unsigned char b;
    
	if (tmo) { 
        	if (tmo < 0)
            		tmo = ~tmo * 1000;
        	else
            		tmo *= 1000000;

        	while (tmo > 0) {
            		usleep(10000);
            		read(tactswFd, &b, sizeof(b));
	       		if (b) return(b);
            			tmo -= 10000;
        	}
	        return(-1); 
    	}
    	else {

      		read(tactswFd, &b, sizeof(b));
        	return(b);
    	}
}


int tact_switch(){
	unsigned char c;
	int selected_tact = 0; // false value

	if((tactswFd = open( tactswDev, O_RDONLY )) < 0){         // KEY open
		perror("open faile /dev/key");
		exit(-1);
	}

	while(1){
		c = tactsw_get(10);
		switch (c) {
				case 1:  selected_tact = 1 ; break;
				case 2:  selected_tact = 2 ; break;
				case 3:  selected_tact = 3 ; break;
				case 4:  selected_tact = 4 ; break;
				case 5:  selected_tact = 5 ; break;
				default: printf("press other key\n", c); break;
		}
		return selected_tact;
		}
	
}

void clcd_input(char clcd_text[]){
//	char led_count_str[10];
	int clcd_d;
	clcd_d = open(clcd , O_RDWR);

	if (clcd_d < 0) {
		printf("디바이스 드라이버가 없습니다.\n");
	}

	write(clcd_d , clcd_text , strlen(clcd_text)); // 1.디스크립터  2.문자들 3.문자들의 크기
	close(clcd_d);
}


void led_control(){
	int dev;

	unsigned char data;
	
	char led_array[] = { 0xFE,0xFC,0xF8,0xF0,0xE0,0xC0,0x80,0x00 };
	
	//1. 해당 드라이버 경로
	//2. O_RDWR

	dev = open(led,O_RDWR);

	if(dev <0){
		printf("열수 없습니다.\n");
		exit(0);
	}

	led_count %= 8;
	data = led_array[led_count]; // binary controlled
		
	//1. 파일의 디스크립트
	//2. 데이터 전달
	//3. 크기를 넣어준다.

	write(dev , &data , sizeof(unsigned char));
	usleep(500000);
	
	close(dev);
	
	led_count++;
}



void DOT_control(int val, int time_sleep){
	int dot_d;
	unsigned char c[3][8] ={
		{ 0xff,0x81,0xff,0x00,0xff,0x18,0xff,0x01 }, // muk
		{ 0xfd,0x49,0x49,0x49,0xb5,0xb5,0xb5,0xb5 }, // zzi
		{ 0xaa,0xaa,0xaa,0xfb,0xab,0xaa,0xaa,0xfa }  // bba
	};


	dot_d = open(dot , O_RDWR);
	if(dot_d<0)
	{
		printf("Error\n");
	}

	write(dot_d , &c[val], sizeof(c));
	
	sleep(time_sleep);
	close(dot_d); // important!!!!!!
}



int rockScissorsPaper(int com_rsp, int user_rsp) {
	int state = 6; // trash value
	
	if (com_rsp == user_rsp) {
		clcd_input("Draw, Do it again");
		state = 0;
	}
	
	else if ( ( (com_rsp == 1) && (user_rsp == 3) ) ||  ( (com_rsp == 2) && (user_rsp == 1) ) || ( (com_rsp == 3) && (user_rsp == 2) ) ) {
		clcd_input("You Win!!");
		state = 1;
	}
	else if ( ( (com_rsp == 3) && (user_rsp == 1) ) ||  ( (com_rsp == 1) && (user_rsp == 2) ) || ( (com_rsp == 2) && (user_rsp == 3) ) ) {
		clcd_input("You Lose^^");
		state = -1;
	}
	else {
		clcd_input("use key 1 or 2 or 3");
	}
	return state;
}

void intro(){
	DOT_control(0, 1); // under1 impossible
	DOT_control(1, 1);
	DOT_control(2, 1);
}

int main() {
	while(1){

	
		while(1){
			clcd_input("press any key to start game");
			
			intro();
			
			if (tact_switch()){
				break;
			}
		}
		
		clcd_input("Betting Money");
		while(1){
			if (tact_switch() == 4){
				led_control();
				printf("betting money : %d\n",led_count);
			}
			else if (tact_switch() == 5){
				break;
			}
			else {
				clcd_input("use right key, 4:money, 5:confirm");
			}
	      	}
		
		
		
		int rsp_state = 0;	
		while(rsp_state == 0){
			clcd_input("Rock Scissors Paper!!");
			
			srand(time(NULL));
			int random = rand() % 3 + 1;	
			
			if (tact_switch() == 1 || tact_switch() == 2 || tact_switch() == 3) {
				user_input = tact_switch();
				rsp_state = rockScissorsPaper(random, user_input);
				printf("%d\n", rsp_state);
				DOT_control(random - 1, 3);
			}
			else {
				clcd_input("use right key, 1:muk, 2:zzi, 3:ppa");
			}
			
			
		}

			
		
	led_count = 0;
		
	}
		
	return 0;
}



/* EOF */