#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <string.h>
#include <asm/ioctls.h>

#include<string.h>
#define clcd "/dev/clcd"

#include<unistd.h>
#include<sys/ioctl.h>
#include<sys/stat.h>

#define led "/dev/led"

static char tactswDev[] = "/dev/tactsw";
static int  tactswFd = (-1);


int led_count = 0;



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
    int selected_tact = 66; // true value
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

			default: printf("press other key\n"); break;
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
//	sprintf(led_count_str, "%d", led_count);
//	strcat(clcd_text, led_count_str);
	write(clcd_d , clcd_text , strlen(clcd_text)); // 1.디스크립터  2.문자들 3.문자들의 크기
	close(clcd_d);
}


void led_control(){
	int dev;

	unsigned char data;
	
	char led_array[] = { 0xFF,0xFE,0xFC,0xF8,0xF0,0xE0,0xC0,0x80,0x00 };
	
	//1. 해당 드라이버 경로
	//2. O_RDWR

	dev = open(led,O_RDWR);

	if(dev <0){
		printf("열수 없습니다.\n");
		exit(0);
	}

	led_count++;
	data = led_array[led_count]; // binary controlled
		
	//1. 파일의 디스크립트
	//2. 데이터 전달
	//3. 크기를 넣어준다.

	write(dev , &data , sizeof(unsigned char));
	usleep(100000);
	

	close(dev);
}

int main()
{
	while(1){
		if (tact_switch()){
			clcd_input("Bet amount : ");
			if (tact_switch() == 4){
				led_control();
        		}
		}
	}	
		
	return 0;
}



/* EOF */