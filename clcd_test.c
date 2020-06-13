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



static char tactswDev[] = "/dev/tactsw";
static int  tactswFd = (-1);

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
			case 2:  selected_tact = 2 ; break;
			case 5:  selected_tact = 5 ; break;
			case 8:  selected_tact = 8 ; break;
			case 11:  selected_tact = 11 ; break;

			default: printf("press other key\n"); break;
        }
	return selected_tact;
	}
	
}

void clcd_input(char clcd_text[]){
	int clcd_d;
	clcd_d = open(clcd , O_RDWR);

	if (clcd_d < 0) {
		printf("디바이스 드라이버가 없습니다.\n");
	}
	
	write(clcd_d , clcd_text , strlen(clcd_text)); // 1.디스크립터  2.문자들 3.문자들의 크기
	close(clcd_d);
}


int main()
{
	while(1){
		if (tact_switch()){
			clcd_input("Bet amount :");
		}
	}	
		
	return 0;
}



/* EOF */